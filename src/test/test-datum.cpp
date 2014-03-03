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
    check(test, datum::METHOD, 27);
    
    printf("\n******** %d Tests Passed ********\n\n", test_count);
  }
  catch (topaz_exception &e)
  {
    printf("Exception raised: %s\n", e.what());
  }
  
  return 0;
}
