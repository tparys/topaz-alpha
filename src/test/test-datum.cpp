/**
 * File:   $URL $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz Test - Datum Packing / Unpacking
 *
 * Copyright (c) 2014, T Parys
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
#include <stdio.h>
#include <stdint.h>
#include <topaz/atom.h>
#include <topaz/datum.h>
#include <topaz/exceptions.h>
#include <topaz/uid.h>
using namespace std;
using namespace topaz;

// Global, eh ....
int test_count = 0;

// Convert datum type to string
char const *datum_type_to_string(datum::type_t type)
{
  switch (type)
  {
    case datum::ATOM:
      return "Atom";
      break;
    case datum::NAMED:
      return "Named Data";
      break;
    case datum::LIST:
      return "List";
      break;
    case datum::METHOD:
      return "Method Call";
      break;
    case datum::END_SESSION:
      return "End of Session Indicator";
      break;
    default:
      return "Unknown";
      break;
  }
}

void dump(byte_vector test_bytes)
{
  size_t i;
  printf("Encoded Data: %u bytes", (unsigned int)test_bytes.size());
  for (i = 0; i < test_bytes.size(); i++)
  {
    if ((i % 16) == 0)
    {
      printf("\n");
    }
    printf(" %02X", test_bytes[i]);
  }
  printf("\n");
}

// Verify data encoding
void check(datum &test, datum::type_t type, size_t size)
{
  datum::type_t type_found;
  byte_vector test_bytes = test.encode_vector();
  datum copy;

  printf("\n");

  // Dump
  printf("Datum: ");
  test.print();
  
  // What's in the datum?
  type_found = test.get_type();
  printf("\nDatum Type: %s\n", datum_type_to_string(type_found));
  
  if (type != type_found)
  {
    printf("*** Failed (expected %s) ***\n", datum_type_to_string(type));
    exit(1);
  }
  
  // Debug
  dump(test_bytes);
  
  // Check total size
  if (size != test_bytes.size())
  {
    printf("*** Failed (expected %u bytes) ***\n", (unsigned int)size);
    exit(1);
  }
  
  // Reconstruct datum
  printf("Testing reconstructed copy ...\n");
  copy.decode_vector(test_bytes);
  if (test != copy)
  {
    printf("*** Failed (decoded object differs) ***\n");
    exit(1);
  }
  
  // Bump the counter
  test_count++;
}

int main()
{
  
  try
  {
    datum test;
    
    // Atom storage (UID)
    test = datum();
    test.value() = atom::new_int(10);
    check(test, datum::ATOM, 1);
    
    // Named Value (UID)
    test = datum();
    test.name() = atom::new_int(20);
    test.named_value() = atom::new_int(20);
    check(test, datum::NAMED, 4);
    
    // List storage
    test = datum();
    test[0].value() = atom::new_int(10);
    check(test, datum::LIST, 3);
    
    // Method call
    test = datum();
    test.object_uid() = SESSION_MGR;
    test.method_uid() = PROPERTIES;
    check(test, datum::METHOD, 21);
    
    printf("\n******** %d Tests Passed ********\n\n", test_count);
  }
  catch (topaz_exception &e)
  {
    printf("Exception raised: %s\n", e.what());
  }
  
  return 0;
}
