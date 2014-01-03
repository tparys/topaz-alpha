#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <topaz/atom.h>
#include <topaz/datum.h>
using namespace std;

// Global, eh ....
int test_count = 0;

// Convert datum type to string
char const *datum_type_to_string(topaz::datum::type_t type)
{
  switch (type)
  {
    case topaz::datum::ATOM:
      return "Atom";
      break;
    case topaz::datum::NAMED:
      return "Named Data";
      break;
    case topaz::datum::LIST:
      return "List";
      break;
    case topaz::datum::METHOD:
      return "Method Call";
      break;
    case topaz::datum::END_SESSION:
      return "End of Session Indicator";
      break;
    default:
      return "Unknown";
      break;
  }
}

void dump(topaz::byte_vector test_bytes)
{
  size_t i;
  printf("Encoded Data: %lu bytes", test_bytes.size());
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
void check(topaz::datum &test, topaz::datum::type_t type, size_t size)
{
  topaz::datum::type_t type_found;
  topaz::byte_vector test_bytes = test.encode_vector();
  topaz::datum copy;

  printf("\n");

  // What's in the datum?
  type_found = test.get_type();
  printf("Data Type: %s\n", datum_type_to_string(type_found));
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
    printf("*** Failed (expected %lu bytes) ***\n", size);
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
  topaz::atom uid(0x0f, true);
  topaz::datum test;
  topaz::datum_vector list;
  
  // Atom storage (UID)
  test = topaz::datum(uid);
  check(test, topaz::datum::ATOM, uid.size());
  
  // Named Value (UID)
  test = topaz::datum(uid, uid);
  check(test, topaz::datum::NAMED, 2 + (2 * uid.size()));
  
  // List storage
  list.push_back(uid);
  test = topaz::datum(list);
  check(test, topaz::datum::LIST, 2 + uid.size());
  
  // Method call
  list.clear();
  test = topaz::datum(0xff, 0xff01, list);
  check(test, topaz::datum::METHOD, 27);
  
  printf("\n******** %d Tests Passed ********\n\n", test_count);
  
  return 0;
}
