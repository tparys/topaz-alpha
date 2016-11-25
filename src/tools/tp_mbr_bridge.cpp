/**
 * Topaz Tools - MBR Shadow Bridge
 *
 * Utility allow system to directly access MBR shadow via a Network Block
 * Device (NBD). Note that the nbd module must be loaded prior to operation.
 *
 * Copyright (c) 2016, T Parys
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/nbd.h>
#include <linux/fs.h>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/pin_entry.h>
#include <topaz/portable_endian.h>
#include <topaz/uid.h>
using namespace std;
using namespace topaz;

/* defines */

#define RAMDISK_MB 128

#define MAX_FD(x, y) ((x) > (y) ? (x) : (y))

/* structures */

typedef struct
{
  string nbd_dev;
  string drive;
  string cur_pin;

  int sig_pipe[2];    // for signal handler
  int kern_pipe[2];   // for NBD kernel thread
  int nbd;            // handle for NBD device

  pthread_t kthread;  // kernel thread
  bool kthread_ok;    // thread started?

} prog_state_t;

/* lone global for signal handler */
int kill_fd = -1;

/* prototypes */
int main2(prog_state_t *state);
void usage();
int start_kern_process(int nbd, int sock, pthread_t *thread);
void *kern_thread(void *ptr);
void set_sig_handler(void (*handler)(int));
void sig_handler(int sig);

int main(int argc, char **argv)
{
  bool cur_pin_valid = false;
  int i, rc;
  char c;

  // initialize state to known values
  prog_state_t state;
  for (i = 0; i < 2; i++)
  {
    state.kern_pipe[i] = -1;
    state.sig_pipe[i] = -1;
  }
  state.nbd = -1;
  memset(&state.kthread, 0, sizeof(state.kthread));
  state.kthread_ok = false;
  
  // defaults
  state.nbd_dev = "/dev/nbd0";
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "p:P:n:v")) != -1)
  {
    switch (c)
    {
      case 'p':
        state.cur_pin = optarg;
        cur_pin_valid = true;
        break;
 
      case 'P':
        state.cur_pin = pin_from_file(optarg);
        cur_pin_valid = true;
        break;
 
      case 'n':
        state.nbd_dev = optarg;
        break;

      case 'v':
        topaz_debug++;
        break;
        
      default:
        if ((optopt == 'p') || (optopt == 'P') || (optopt == 'n'))
        {
          cerr << "Option -" << optopt << " requires an argument." << endl;
        }
        else
        {
          cerr << "Invalid command line option " << c << endl;
        }
        break;
    }
  }
  
  // Check remaining arguments
  if ((argc - optind) < 1)
  {
    cerr << "Invalid number of arguments" << endl;
    usage();
    return -1;
  }
  state.drive = argv[optind];
  
  // Get a PIN if not provided
  if (!cur_pin_valid)
  {
    state.cur_pin = pin_from_console("current");
  }
  
  // pass control so we can catch a return()
  rc = main2(&state);
  
  // cleanup
  for (i = 0; i < 2; i++)
  {
    close(state.kern_pipe[i]);
    close(state.sig_pipe[i]);
  }
  close(state.nbd);
  if (state.kthread_ok)
  {
    pthread_join(state.kthread, NULL);
  }
  
  /* done */
  return rc;
}

int main2(prog_state_t *state)
{
  uint64_t mbr_shadow_size = 128 * 1024 * 1024; // Min size for now
  uint64_t user_uid = ADMIN_BASE + 1;
  struct nbd_request request;
  struct nbd_reply reply;
  int max_fd, rc = 0;
  fd_set fds;
  vector<byte> buffer;
  
  ////
  // First set up the NBD device ...
  //

  // set up some internal pipes
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, state->kern_pipe) ||
      socketpair(AF_UNIX, SOCK_STREAM, 0, state->sig_pipe))
  {
    perror("Cannot open pipe");
    return 1;
  }
  kill_fd = state->sig_pipe[1];
  
  // install signal handler
  set_sig_handler(sig_handler); // in case of ctl-c

  // open the NBD device
  if ((state->nbd = open(state->nbd_dev.c_str(), O_RDWR)) < 0)
  {
    perror("Cannot open NBD device");
    return 1;
  }
  
  // set up NBD
  if ((ioctl(state->nbd, NBD_SET_SIZE, mbr_shadow_size) == -1) ||
      (ioctl(state->nbd, NBD_SET_BLKSIZE, 4096) == -1) ||
      (ioctl(state->nbd, NBD_CLEAR_SOCK) == -1))
  {
    perror("NBD setup failed");
    return 1;
  }
    
  // set up kernel thread
  if (start_kern_process(state->nbd, state->kern_pipe[1], &(state->kthread)))
  {
    perror("Cannot start kernel thread");
    return 1;
  }
  state->kthread_ok = 1;
  
  ////
  // NBD <-> TCG Opal Operations
  //
  
  // operation
  try
  {
    // Open the device
    drive target(state->drive.c_str());
    target.login(LOCKING_SP, user_uid, state->cur_pin);
    
    // only need to set this once
    reply.magic = htobe32(NBD_REPLY_MAGIC);
    
    // main program loop
    printf("And we're up!\n");
    while (1)
    {
      // set up select call
      FD_ZERO(&fds);
      FD_SET(state->sig_pipe[0], &fds);
      FD_SET(state->kern_pipe[0], &fds);
      max_fd = MAX_FD(state->sig_pipe[0], state->kern_pipe[0]);
      
      // call select()
      rc = select(max_fd + 1, &fds, NULL, NULL, NULL);
      if ((rc == -1) && (errno == EINTR))
      {
        // signal handler probably fired, loop and check
        continue;
      }
      else if (rc < 1)
      {
        printf("Unexpected select error (rc=%d)\n", rc);
        return 1;
      }
    
      // signal handler telling us it's time?
      if (FD_ISSET(state->sig_pipe[0], &fds))
      {
        printf("Caught signal and shutting down ...\n");
        return 0;
      }
    
      // got something from kernel?
      if (FD_ISSET(state->kern_pipe[0], &fds))
      {
        // read I/O request from kernel
        rc = recv(state->kern_pipe[0], &request, sizeof(request), MSG_WAITALL);
        if (rc < (int)sizeof(request))
        {
          perror("Short read from kernel");
          return 1;
        }
 
        TOPAZ_DEBUG(1) printf("Got a command!\n");
 
        // byteflips
        request.magic = be32toh(request.magic);
        request.type = be32toh(request.type);
        request.len = be32toh(request.len);
        request.from = be64toh(request.from);
      
        // sanity check
        if (request.magic != NBD_REQUEST_MAGIC)
        {
          fprintf(stderr, "Invalid NBD magic number");
          return 1;
        }
 
        // I/O handle
        memcpy(reply.handle, request.handle, sizeof(request.handle));
        reply.error = 0;
 
        // figure out what the kernel wants ...
        switch (request.type)
        {
          case NBD_CMD_READ:
            TOPAZ_DEBUG(1) printf("Request for read of size %d\n", request.len);
     
            // Grab the data from the MBR
            buffer.resize(request.len);
            target.table_get_bin(MBR_UID, request.from, &(buffer[0]), request.len);
     
            // Respond
            send(state->kern_pipe[0], &reply, sizeof(struct nbd_reply), 0);
            send(state->kern_pipe[0], &(buffer[0]), request.len, 0);
            break;

          case NBD_CMD_WRITE:
            TOPAZ_DEBUG(1) printf("Request for write of size %d\n", request.len);

            // Grab data from socket
            buffer.resize(request.len);
            recv(state->kern_pipe[0], &(buffer[0]), request.len, MSG_WAITALL);
            target.table_set_bin(MBR_UID, request.from, &(buffer[0]), request.len);

            // Respond
            send(state->kern_pipe[0], (char*)&reply, sizeof(struct nbd_reply), 0);
            break;

#ifdef NBD_CMD_FLUSH
          case NBD_CMD_FLUSH:
            TOPAZ_DEBUG(1) printf("Flush request\n");

            // Nothing to do, respond
            send(state->kern_pipe[0], (char*)&reply, sizeof(struct nbd_reply), 0);
            break;
#endif

          default:
            TOPAZ_DEBUG(1) printf("Invalid / unknown request type %d\n", request.type);
     
            // Respond w/ error
            reply.error = htobe32(EINVAL);
            send(state->kern_pipe[0], (char*)&reply, sizeof(struct nbd_reply), 0);
            break;
        }
      }
    }
  }
  
  // Something bad happened ...
  catch (topaz_exception &e)
  {
    cerr << "Exception raised: " << e.what() << endl;
  }
  
  return 1;
}

void usage()
{
  cerr << endl
       << "Usage:" << endl
       << "  tp_mbr_bridge [opts] <drive>" << endl
    
       << endl
       << "Options:" << endl
       << "  -p <pin>  - Provide current SID PIN" << endl
       << "  -P <file> - Read current PIN from file" << endl
       << "  -n <dev>  - Choose NBD device (default /dev/nbd0)" << endl
       << "  -v        - Increase debug verbosity" << endl;
}

int start_kern_process(int nbd, int sock, pthread_t *thread)
{
  /* set up passed arguments */
  int *args = (int*)calloc(2, sizeof(int));
  if (args == NULL)
  {
    perror("Error allocating memory");
    return 1;
  }
  args[0] = nbd;
  args[1] = sock;
  
  /* fire off kernel thread */
  if (pthread_create(thread, NULL, kern_thread, (void*)args))
  {
    /* cleanup */
    free(args);
    return 1;
  }
  
  /* finish initialization */
  ioctl(nbd, NBD_CLEAR_QUE);
  ioctl(nbd, NBD_CLEAR_SOCK);
  
  return 0;
}

void *kern_thread(void *ptr)
{
  int *args = (int*)ptr;
  int nbd = args[0];
  int sock = args[1];
  int rc;
  free(ptr);
  
  /* set up socket */
  if (ioctl(nbd, NBD_SET_SOCK, sock) == -1)
  {
    fprintf(stderr, "Cannot set NBD socket");
    return NULL;
  }
  
  /* do things in kernel space */
  printf("NBD process starting ...\n");
  rc = ioctl(nbd, NBD_DO_IT);
  printf("NBD process terminated with status %d\n", rc);
  
  return NULL;
}

// Configure interrupt handler
void set_sig_handler(void (*handler)(int))
{
  /* handler info */
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = handler;
  
  /* install signal handlers */
  sigaction(SIGINT, &action, NULL);  /* control-c at console */
  sigaction(SIGTERM, &action, NULL); /* kill / pkill */
  sigaction(SIGPIPE, &action, NULL); /* network disconnect */
}

// Main signal handler to self-pipe a shutdown signal
void sig_handler(int sig)
{
  /* Something to send. Seems appropriate */
  char msg = 'X';
  
  /* off it goes */
  if (send(kill_fd, &msg, sizeof(msg), 0) != sizeof(msg))
  {
    /* can't signal nicely, so here's the hammer */
    exit(0);
  }
}
