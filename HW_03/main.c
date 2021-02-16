#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "header.h"

void test_file_writing()
{
  char* arr[] = {"", "str", "hello world", "!!!!", "Hello\tthere\n!"};
  char fn1[] = "fof.txt";
  char fn2[] = "fwf.txt";
  char buf1[1000] = {0}, buf2[1000] = {0};

  for (int i = 0 ; i < sizeof(arr) / sizeof(char *); i++)
  {
    FILE *f1 = fopen(fn1, "wb");
    assert(f1 != NULL);
    fwrite(arr[i], 1, strlen(arr[i]), f1);
    fclose(f1);

    assert(write_to_file(fn2, arr[i]) == 1);

    FILE *fr1 = fopen(fn1, "rb");
    assert(fr1 != NULL);
    fread(buf1, 1, strlen(arr[i]), fr1);
    fclose(fr1);

    FILE *fr2 = fopen(fn2, "rb");
    assert(fr2 != NULL);
    fread(buf2, 1, strlen(arr[i]), fr2);
    fclose(fr2);

    //printf("%d) %s, %s\n", i, buf1, buf2);
    assert(strcmp(buf1, buf2) == 0);
  }
}

int main() 
{
  test_file_writing();

  return 0;
}
