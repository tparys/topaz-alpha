/**
 * File:   $URL $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz Test - Atom Packing / Unpacking
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
#include <topaz/exceptions.h>
using namespace std;
using namespace topaz;

// Global, eh ....
int test_count = 0;

// Convert atom type to string
char const *atom_type_to_string(atom::type_t type)
{
  switch (type)
  {
    case atom::EMPTY:
      return "Empty";
      break;
    case atom::UINT:
      return "Unsigned Integer";
      break;
    case atom::INT:
      return "Signed Integer";
      break;
    case atom::BYTES:
      return "Binary Data";
      break;
    default:
      return "Unknown";
      break;
  }
}

// Convert encoding size to string
char const *atom_enc_to_string(atom::enc_t enc)
{
  switch (enc)
  {
    case atom::NONE:
      return "N/A";
      break;
    case atom::TINY:
      return "Tiny";
      break;
    case atom::SHORT:
      return "Short";
      break;
    case atom::MEDIUM:
      return "Medium";
      break;
    case atom::LONG:
      return "LONG";
      break;
    default:
      return "Unknown";
      break;
  }
}

void dump(byte_vector test_bytes)
{
  size_t i;
  printf("Encoded Data: %lu bytes\n", test_bytes.size());
  for (i = 0; (i < test_bytes.size()) && (i < 16); i++)
  {
    printf(" %02X", test_bytes[i]);
  }
  if (i < test_bytes.size())
  {
    printf(" ...");
  }
  printf("\n");
}

// Verify data encoding
void check(atom test, atom::type_t type, atom::enc_t enc, size_t size)
{
  atom::type_t type_found;
  atom::enc_t enc_found;
  byte_vector test_bytes = test.encode_vector();
  atom copy;
  
  // Dump
  printf("Atom: ");
  test.print();
  
  // What's in the atom?
  type_found = test.get_type();
  printf("\nData Type: %s\n", atom_type_to_string(type_found));
  if (type != type_found)
  {
    printf("*** Failed (expected %s) ***\n", atom_type_to_string(type));
    exit(1);
  }
  
  // How is it encoded?
  enc_found = test.get_enc();
  printf("Encoding: %s\n", atom_enc_to_string(enc_found));
  if (enc != enc_found)
  {
    printf("*** Failed (expected %s) ***\n", atom_enc_to_string(enc));
    exit(1);
  }
  
  // Debug
  dump(test_bytes);
  
  // Check total size
  if (test.get_header_size() + size != test_bytes.size())
  {
    printf("*** Failed (expected %lu bytes) ***\n", test.get_header_size() + size);
    exit(1);
  }
  
  // Reconstruct atom
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

void test_unsigned(atom::type_t type, atom::enc_t enc, size_t size, uint64_t val)
{
  // Debug
  printf("\nUnsigned Integer: %lu (0x%lx)\n", val, val);
  
  // Check
  check(atom::new_uint(val), type, enc, size);
}

void test_signed(atom::type_t type, atom::enc_t enc, size_t size, int64_t val)
{
  // Debug
  printf("\nSigned Integer: %ld (0x%lx)\n", val, val);
  
  // Check
  check(atom::new_int(val), type, enc, size);
}

void test_binary(atom::enc_t enc, size_t size)
{
  byte_vector raw;
  
  // Initialize
  raw.reserve(size);
  for (size_t i = 0; i < size; i++)
  {
    raw.push_back(0xff & i);
  }
  
  // Debug
  printf("\nBinary Data: %u bytes\n", (unsigned int)size);
  
  // Check
  check(atom::new_bin(raw), atom::BYTES, enc, size);
}


void test_uid(uint64_t val)
{
  // Debug
  printf("\nUnique ID: 0x%lx\n", val);
  
  // Encode
  atom first = atom::new_uid(val);
  
  // Check
  check(first, atom::BYTES, atom::SHORT, 8);

  // Serialize to binary
  byte_vector encoded = first.encode_vector();
  
  // Deserialize
  atom second;
  second.decode_vector(encoded);
  
  // Verify
  if (second.get_uid() != val)
  {
    printf("Decode mismatch!\n");
    exit(1);
  }
}

int main()
{
  
  try
  {
    
    //////////////////////////////////////////////////////////////////////////////
    // Unsigned Integers
    //
    
    // Trivial
    test_unsigned(atom::UINT, atom::TINY, 1, 0);
    
    // Max Tiny Atom
    test_unsigned(atom::UINT, atom::TINY, 1, 0x3f);
    
    // Min Short
    test_unsigned(atom::UINT, atom::SHORT, 1, 0x40);
    
    // Variable byte encoding
    for (uint64_t val = 0x100, i = 1; i < 8; i++, val <<= 8)
    {
      test_unsigned(atom::UINT, atom::SHORT, i    , val - 1);
      test_unsigned(atom::UINT, atom::SHORT, i + 1, val);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // Signed Integers
    //
  
    // Trivial
    test_signed(atom::INT, atom::TINY, 1, 0);
  
    // Max Tiny Atom
    test_signed(atom::INT, atom::TINY, 1, 0x1f);
    test_signed(atom::INT, atom::TINY, 1, -0x20);
  
    // Min Short
    test_signed(atom::INT, atom::SHORT, 1, 0x20);
    test_signed(atom::INT, atom::SHORT, 1, -0x21);
  
    // Variable byte encoding
    for (int64_t val = 0x80, i = 1; i < 8; i++, val <<= 8)
    {
      // Positive encoding
      test_signed(atom::INT, atom::SHORT, i    , val - 1);
      test_signed(atom::INT, atom::SHORT, i + 1, val);

      // Negative encoding
      test_signed(atom::INT, atom::SHORT, i    , (0 - val));
      test_signed(atom::INT, atom::SHORT, i + 1, (0 - val) - 1);
    }
  
    //////////////////////////////////////////////////////////////////////////////
    // Binary Data
    //
  
    // Min Short
    test_binary(atom::SHORT, 0);
  
    // Max Short
    test_binary(atom::SHORT, 0xf);
  
    // Min Medium
    test_binary(atom::MEDIUM, 0x10);
  
    // Max Medium
    test_binary(atom::MEDIUM, 0x7ff);
  
    // Min Long
    test_binary(atom::LONG, 0x800);
  
    // Max Long
    test_binary(atom::LONG, 0xffffff);
  
    //////////////////////////////////////////////////////////////////////////////
    // Misc Types
    //
  
    // Unique ID (Crazy integer that looks like a binary thing)
    test_uid(0x0f);
  
    // Empty Atom
    printf("\n");
    atom empty;
    check(empty, atom::EMPTY, atom::NONE, 1);
  
    printf("\n******** %d Tests Passed ********\n\n", test_count);
    
  }
  catch (topaz_exception &e)
  {
    printf("Exception raised: %s\n", e.what());
  }
  
  return 0;
}
