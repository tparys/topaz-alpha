#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <topaz/atom.h>
using namespace std;

// Global, eh ....
int test_count = 0;

// Convert encoding contents to string
char const *atom_type_to_string(topaz::atom::type_t type)
{
  switch (type)
  {
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

// Verify data encoding
void check(topaz::atom &data, topaz::atom::type_t type, size_t head_bytes, size_t size)
{
  topaz::atom::type_t type_found;
  size_t i, size_found;
  
  // What's in the atom?
  type_found = data.get_type();
  printf("Data Type: %s\n", atom_type_to_string(type_found));
  if (type != type_found)
  {
    printf("*** Failed (expected %s) ***\n", atom_type_to_string(type));
    exit(1);
  }
  
  // Check header size
  size_found = data.get_header_size();
  printf("Header Bytes: %lu\n", size_found);
  if (head_bytes != size_found)
  {
    printf("*** Failed (expected %lu) ***\n", head_bytes);
    exit(1);
  }
  
  // Check total size
  printf("Encoding: %lu bytes\n", data.size());
  for (i = 0; (i < data.size()) && (i < 16); i++)
  {
    printf(" %02X", data[i]);
  }
  if (i < data.size())
  {
    printf(" ...");
  }
  printf("\n");
  if (head_bytes + size != data.size())
  {
    printf("*** Failed (expected %lu bytes) ***\n", head_bytes + size);
    exit(1);
  }
  
  // Bump the counter
  test_count++;
}

void test_unsigned(topaz::atom::type_t type, size_t head_bytes, size_t size, uint64_t val)
{
  // Debug
  printf("\nUnsigned Integer: %llu (0x%llx)\n",
	 (long long unsigned)val, (long long unsigned)val);
  
  // Encode
  topaz::atom first(val);
  
  // Check
  check(first, type, head_bytes, size);
  
  // Serialize to binary
  topaz::byte_vector encoded = first;
  
  // Deserialize
  topaz::atom second;
  second.deserialize(encoded);
  printf("Decoded value: %lu\n", second.get_uint());
  
  // Verify
  if (second.get_uint() != val)
  {
    printf("Decode mismatch!\n");
    exit(1);
  }
}

void test_signed(topaz::atom::type_t type, size_t head_bytes, size_t size, int64_t val)
{
  // Debug
  printf("\nSigned Integer: %lld (0x%llx)\n",
	 (long long)val, (long long unsigned)val);
  
  // Encode
  topaz::atom first(val);
  
  // Check
  check(first, type, head_bytes, size);
  
  // Serialize to binary
  topaz::byte_vector encoded = first;
  
  // Deserialize
  topaz::atom second;
  second.deserialize(encoded);
  printf("Decoded value: %ld\n", second.get_int());
  
  // Verify
  if (second.get_int() != val)
  {
    printf("Decode mismatch!\n");
    exit(1);
  }
}

void test_binary(size_t head_bytes, size_t size)
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
  check(first, topaz::atom::BYTES, head_bytes, size);
  
  // Serialize to binary
  topaz::byte_vector encoded = first;
  
  // Deserialize
  topaz::atom second;
  second.deserialize(encoded);
  
  // Check data
  topaz::byte_vector copy = second.get_bytes();
  if (copy.size() != raw.size())
  {
    printf("Extracted data wrong size\n");
    exit(1);
  }
  for (size_t i = 0; i < size; i++)
  {
    if (raw[i] != copy[i])
    {
      printf("Data mismatch, byte %lu\n", i);
      exit(1);
    }
  }
}

/*
void test_uid()
{
  topaz::syntax data;
  uint64_t uid = 0x01020304050607ULL;
  
  // Debug
  printf("\nUID: 0x%lx\n", uid);
  
  // Encode
  data.encode_uid(uid);
  
  // Check
  enc_check(ENC_SHORT, ENC_BINARY, 8, data);
}

void test_method()
{
  topaz::syntax data;
  topaz::syntax args; // left empty
  size_t i;
  
  // Debug
  printf("\nMethod Call:\n");
  
  // Encode
  args.push_back(0xee);
  data.encode_method_call(0xff, 0xff01, args);
  
  // Dump
  printf("Data: %lu bytes\n", data.size());
  for (i = 0; i < data.size(); i++)
  {
    printf(" %02X", data[i]);
  }
  if (i < data.size())
  {
    printf(" ...");
  }
  printf("\n");
}
*/

int main()
{
  //////////////////////////////////////////////////////////////////////////////
  // Unsigned Integers
  //
  
  // Trivial
  test_unsigned(topaz::atom::UINT, 0, 1, 0);
  
  // Max Tiny Atom
  test_unsigned(topaz::atom::UINT, 0, 1, 0x3f);
  
  // Min Short
  test_unsigned(topaz::atom::UINT, 1, 1, 0x40);
  
  // Variable byte encoding
  for (uint64_t val = 0x100, i = 1; i < 8; i++, val <<= 8)
  {
    test_unsigned(topaz::atom::UINT, 1, i    , val - 1);
    test_unsigned(topaz::atom::UINT, 1, i + 1, val);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Signed Integers
  //
  
  // Trivial
  test_signed(topaz::atom::INT, 0, 1, 0);
  
  // Max Tiny Atom
  test_signed(topaz::atom::INT, 0, 1, 0x1f);
  test_signed(topaz::atom::INT, 0, 1, -0x20);
  
  // Min Short
  test_signed(topaz::atom::INT, 1, 1, 0x20);
  test_signed(topaz::atom::INT, 1, 1, -0x21);
  
  // Variable byte encoding
  for (int64_t val = 0x80, i = 1; i < 8; i++, val <<= 8)
  {
    // Positive encoding
    test_signed(topaz::atom::INT, 1, i    , val - 1);
    test_signed(topaz::atom::INT, 1, i + 1, val);

    // Negative encoding
    test_signed(topaz::atom::INT, 1, i    , (0 - val));
    test_signed(topaz::atom::INT, 1, i + 1, (0 - val) - 1);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Binary Data
  //
  
  // Min Short
  test_binary(1, 0);
  
  // Max Short
  test_binary(1, 0xf);
  
  // Min Medium
  test_binary(2, 0x10);
  
  // Max Medium
  test_binary(2, 0x7ff);
  
  // Min Long
  test_binary(4, 0x800);
  
  // Max Long
  test_binary(4, 0xffffff);
  
  //////////////////////////////////////////////////////////////////////////////
  // Methods and UIDs
  //
  
  // 8 byte UID
  //test_uid();
  
  // 8 byte UID
  //test_method();
  
  printf("\n******** %d Tests Passed ********\n\n", test_count);
  
  return 0;
}
