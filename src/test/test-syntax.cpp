#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <topaz/syntax.h>
using namespace std;

// Global, eh ....
int test_count = 0;

// Encoding class
typedef enum
{
  ENC_TINY,   // Value in header byte
  ENC_SHORT,  // Up to 15 bytes
  ENC_MEDIUM, // Up to 2047 bytes
  ENC_LONG    // Up to 16777215 bytes
} enc_type_t;

// Encoding contents
typedef enum
{
  ENC_UNSIGNED = 0,
  ENC_SIGNED   = 1,
  ENC_BINARY   = 2,
  ENC_BIN_CONT = 3
} enc_cont_t;

// Convert encoding type to string
char const *enc_type_to_string(enc_type_t val)
{
  switch (val)
  {
    case ENC_TINY:
      return "Tiny";
      break;
    case ENC_SHORT:
      return "Short";
      break;
    case ENC_MEDIUM:
      return "Medium";
      break;
    case ENC_LONG:
      return "Long";
      break;
    default:
      return "Unknown";
      break;
  }
}

// Convert encoding contents to string
char const *enc_cont_to_string(enc_cont_t val)
{
  switch (val)
  {
    case ENC_UNSIGNED:
      return "Unsigned Integer";
      break;
    case ENC_SIGNED:
      return "Signed Integer";
      break;
    case ENC_BINARY:
      return "Binary Data";
      break;
    case ENC_BIN_CONT:
      return "Binary (continued)";
      break;
    default:
      return "Unknown";
      break;
  }
}

// Verify data encoding
void enc_check(enc_type_t type, enc_cont_t cont, size_t size, topaz::syntax &data)
{
  enc_type_t type_found;
  enc_cont_t cont_found;
  size_t i, head_bytes;
  
  // Determine type of data encoding
  if ((data[0] & topaz::TOK_LONG_ATOM) == topaz::TOK_LONG_ATOM)
  {
    type_found = ENC_LONG;
    cont_found = (enc_cont_t)(0x03 & data[0]);
    head_bytes = 4;
  }
  else if ((data[0] & topaz::TOK_MEDIUM_ATOM) == topaz::TOK_MEDIUM_ATOM)
  {
    type_found = ENC_MEDIUM;
    cont_found = (enc_cont_t)(0x03 & (data[0] >> 3));
    head_bytes = 2;
  }
  else if ((data[0] & topaz::TOK_SHORT_ATOM) == topaz::TOK_SHORT_ATOM)
  {
    type_found = ENC_SHORT;
    cont_found = (enc_cont_t)(0x03 & (data[0] >> 4));
    head_bytes = 1;
  }
  else
  {
    type_found = ENC_TINY;
    cont_found = (enc_cont_t)(0x01 & (data[0] >> 6));
    head_bytes = 0;
  }
  
  // First check the encoding type ...
  printf("Encoding Type: %s\n", enc_type_to_string(type_found));
  if (type != type_found)
  {
    printf("*** Failed (expected %s) ***\n", enc_type_to_string(type));
    exit(1);
  }
  
  // Then what the encoding is of ...
  printf("Encoding Contents: %s\n", enc_cont_to_string(cont_found));
  if (cont != cont_found)
  {
    printf("*** Failed (expected %s) ***\n", enc_cont_to_string(cont));
    exit(1);
  }
  
  // Lastly, check the size
  printf("Data: %lu bytes (%lu byte header)\n", data.size(), head_bytes);
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

void test_signed(enc_type_t type, enc_cont_t cont, size_t size, int64_t val)
{
  topaz::syntax data;
  
  // Debug
  printf("\nSigned Integer: %lld (0x%llx)\n",
	 (long long)val, (long long unsigned)val);
  
  // Encode
  data.encode_signed(val);
  
  // Check
  enc_check(type, cont, size, data);
}

void test_unsigned(enc_type_t type, enc_cont_t cont, size_t size, uint64_t val)
{
  topaz::syntax data;
  
  // Debug
  printf("\nUnsigned Integer: %llu (0x%llx)\n",
	 (long long unsigned)val, (long long unsigned)val);
  
  // Encode
  data.encode_unsigned(val);
  
  // Check
  enc_check(type, cont, size, data);
}

void test_binary(enc_type_t type, size_t size)
{
  topaz::syntax data;
  unsigned char *bin = new unsigned char[size];
  
  // Initialize
  for (size_t i = 0; i < size; i++)
  {
    bin[i] = 0xff & i;
  }
  
  // Debug
  printf("\nBinary Data: %u bytes\n", (unsigned int)size);
  
  // Encode
  data.encode_binary(bin, size);
  
  // Check
  enc_check(type, ENC_BINARY, size, data);
  
  // Cleanup
  delete [] bin;
}

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

int main()
{
  //////////////////////////////////////////////////////////////////////////////
  // Unsigned Integers
  //
  
  // Trivial
  test_unsigned(ENC_TINY, ENC_UNSIGNED, 1, 0);
  
  // Max Atom
  test_unsigned(ENC_TINY, ENC_UNSIGNED, 1, 0x3f);
  
  // Min Short
  test_unsigned(ENC_SHORT, ENC_UNSIGNED, 1, 0x40);
  
  // Variable byte encoding
  for (uint64_t val = 0x100, i = 1; i < 8; i++, val <<= 8)
  {
    test_unsigned(ENC_SHORT, ENC_UNSIGNED, i    , val - 1);
    test_unsigned(ENC_SHORT, ENC_UNSIGNED, i + 1, val);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Signed Integers
  //
  
  // Trivial
  test_signed(ENC_TINY, ENC_SIGNED, 1, 0);
  
  // Max Atom
  test_signed(ENC_TINY, ENC_SIGNED, 1, 0x1f);
  test_signed(ENC_TINY, ENC_SIGNED, 1, -0x20);
  
  // Min Short
  test_signed(ENC_SHORT, ENC_SIGNED, 1, 0x20);
  test_signed(ENC_SHORT, ENC_SIGNED, 1, -0x21);
  
  // Variable byte encoding
  for (int64_t val = 0x80, i = 1; i < 8; i++, val <<= 8)
  {
    // Positive encoding
    test_signed(ENC_SHORT, ENC_SIGNED, i    , val - 1);
    test_signed(ENC_SHORT, ENC_SIGNED, i + 1, val);

    // Negative encoding
    test_signed(ENC_SHORT, ENC_SIGNED, i    , (0 - val));
    test_signed(ENC_SHORT, ENC_SIGNED, i + 1, (0 - val) - 1);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Binary Data
  //
  
  // Min Short
  test_binary(ENC_SHORT, 0);
  
  // Max Short
  test_binary(ENC_SHORT, 0xf);
  
  // Min Medium
  test_binary(ENC_MEDIUM, 0x10);
  
  // Max Medium
  test_binary(ENC_MEDIUM, 0x7ff);
  
  // Min Long
  test_binary(ENC_LONG, 0x800);
  
  // Max Long
  test_binary(ENC_LONG, 0xffffff);
  
  //////////////////////////////////////////////////////////////////////////////
  // Methods and UIDs
  //
  
  // 8 byte UID
  test_uid();
  
  // 8 byte UID
  test_method();
  
  printf("\n******** %d Tests Passed ********\n\n", test_count);
  
  return 0;
}
