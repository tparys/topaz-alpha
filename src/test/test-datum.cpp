#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <topaz/atom.h>
#include <topaz/datum.h>
#include <topaz/uid.h>
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
  
  // Atom storage (UID)
  test = topaz::datum();
  test.value() = uid;
  check(test, topaz::datum::ATOM, uid.size());
  
  // Named Value (UID)
  test.name() = uid;
  check(test, topaz::datum::NAMED, 2 + (2 * uid.size()));
  
  // List storage
  test = topaz::datum();
  test[0].value() = uid;
  check(test, topaz::datum::LIST, 2 + uid.size());
  
  // Method call
  test = topaz::datum();
  test.object_uid() = topaz::OBJ_SESSION_MGR;
  test.method_uid() = topaz::MTH_PROPERTIES;
  check(test, topaz::datum::METHOD, 27);
  
  printf("\n******** %d Tests Passed ********\n\n", test_count);
  
  return 0;
}
