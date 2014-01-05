#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <topaz/atom.h>
using namespace std;

// Global, eh ....
int test_count = 0;

// Convert atom type to string
char const *atom_type_to_string(topaz::atom::type_t type)
{
  switch (type)
  {
    case topaz::atom::EMPTY:
      return "Empty";
      break;
    case topaz::atom::UINT:
      return "Unsigned Integer";
      break;
    case topaz::atom::INT:
      return "Signed Integer";
      break;
    case topaz::atom::BYTES:
      return "Binary Data";
      break;
    default:
      return "Unknown";
      break;
  }
}

// Convert encoding size to string
char const *atom_enc_to_string(topaz::atom::enc_t enc)
{
  switch (enc)
  {
    case topaz::atom::NONE:
      return "N/A";
      break;
    case topaz::atom::TINY:
      return "Tiny";
      break;
    case topaz::atom::SHORT:
      return "Short";
      break;
    case topaz::atom::MEDIUM:
      return "Medium";
      break;
    case topaz::atom::LONG:
      return "LONG";
      break;
    default:
      return "Unknown";
      break;
  }
}

void dump(topaz::byte_vector test_bytes)
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
void check(topaz::atom &test, topaz::atom::type_t type, topaz::atom::enc_t enc, size_t size)
{
  topaz::atom::type_t type_found;
  topaz::atom::enc_t enc_found;
  topaz::byte_vector test_bytes = test.encode_vector();
  topaz::atom copy;
  
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

void test_unsigned(topaz::atom::type_t type, topaz::atom::enc_t enc, size_t size, uint64_t val)
{
  // Debug
  printf("\nUnsigned Integer: %llu (0x%llx)\n",
	 (long long unsigned)val, (long long unsigned)val);
  
  // Encode
  topaz::atom first(val);
  
  // Check
  check(first, type, enc, size);
}

void test_signed(topaz::atom::type_t type, topaz::atom::enc_t enc, size_t size, int64_t val)
{
  // Debug
  printf("\nSigned Integer: %lld (0x%llx)\n",
	 (long long)val, (long long unsigned)val);
  
  // Encode
  topaz::atom first(val);
  
  // Check
  check(first, type, enc, size);
}

void test_binary(topaz::atom::enc_t enc, size_t size)
{
  topaz::byte_vector raw;
  
  // Initialize
  raw.reserve(size);
  for (size_t i = 0; i < size; i++)
  {
    raw.push_back(0xff & i);
  }
  
  // Debug
  printf("\nBinary Data: %u bytes\n", (unsigned int)size);
  
  // Encode
  topaz::atom first(raw);
  
  // Check
  check(first, topaz::atom::BYTES, enc, size);
}


void test_uid(uint64_t val)
{
  // Debug
  printf("\nUnique ID: 0x%lx\n", val);
  
  // Encode
  topaz::atom first(val, true);
  
  // Check
  check(first, topaz::atom::BYTES, topaz::atom::SHORT, 8);

  // Serialize to binary
  topaz::byte_vector encoded = first.encode_vector();
  
  // Deserialize
  topaz::atom second;
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

  //////////////////////////////////////////////////////////////////////////////
  // Unsigned Integers
  //
  
  // Trivial
  test_unsigned(topaz::atom::UINT, topaz::atom::TINY, 1, 0);
  
  // Max Tiny Atom
  test_unsigned(topaz::atom::UINT, topaz::atom::TINY, 1, 0x3f);
  
  // Min Short
  test_unsigned(topaz::atom::UINT, topaz::atom::SHORT, 1, 0x40);
  
  // Variable byte encoding
  for (uint64_t val = 0x100, i = 1; i < 8; i++, val <<= 8)
  {
    test_unsigned(topaz::atom::UINT, topaz::atom::SHORT, i    , val - 1);
    test_unsigned(topaz::atom::UINT, topaz::atom::SHORT, i + 1, val);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Signed Integers
  //
  
  // Trivial
  test_signed(topaz::atom::INT, topaz::atom::TINY, 1, 0);
  
  // Max Tiny Atom
  test_signed(topaz::atom::INT, topaz::atom::TINY, 1, 0x1f);
  test_signed(topaz::atom::INT, topaz::atom::TINY, 1, -0x20);
  
  // Min Short
  test_signed(topaz::atom::INT, topaz::atom::SHORT, 1, 0x20);
  test_signed(topaz::atom::INT, topaz::atom::SHORT, 1, -0x21);
  
  // Variable byte encoding
  for (int64_t val = 0x80, i = 1; i < 8; i++, val <<= 8)
  {
    // Positive encoding
    test_signed(topaz::atom::INT, topaz::atom::SHORT, i    , val - 1);
    test_signed(topaz::atom::INT, topaz::atom::SHORT, i + 1, val);

    // Negative encoding
    test_signed(topaz::atom::INT, topaz::atom::SHORT, i    , (0 - val));
    test_signed(topaz::atom::INT, topaz::atom::SHORT, i + 1, (0 - val) - 1);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Binary Data
  //
  
  // Min Short
  test_binary(topaz::atom::SHORT, 0);
  
  // Max Short
  test_binary(topaz::atom::SHORT, 0xf);
  
  // Min Medium
  test_binary(topaz::atom::MEDIUM, 0x10);
  
  // Max Medium
  test_binary(topaz::atom::MEDIUM, 0x7ff);
  
  // Min Long
  test_binary(topaz::atom::LONG, 0x800);
  
  // Max Long
  test_binary(topaz::atom::LONG, 0xffffff);
  
  //////////////////////////////////////////////////////////////////////////////
  // Misc Types
  //
  
  // Unique ID (Crazy integer that looks like a binary thing)
  test_uid(0x0f);
  
  // Empty Atom
  printf("\n");
  topaz::atom empty;
  check(empty, topaz::atom::EMPTY, topaz::atom::NONE, 1);
  
  printf("\n******** %d Tests Passed ********\n\n", test_count);
  
  return 0;
}
