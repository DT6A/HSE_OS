#include <stdio.h>
#include <assert.h>

#include "header.h"

int checkPrimeNumber(int n)
{
    int j, flag = 1;
    for (j = 2; j <= n / 2; ++j) 
    {
        if (n % j == 0) 
        {
            flag = 0;
            break;
        }
    }
    return flag;
}

void test_sqrt()
{
  assert(aprox_int_sqrt(0) == 0);
  assert(aprox_int_sqrt(1) == 1);
  assert(aprox_int_sqrt(2) == 2);
  assert(aprox_int_sqrt(4) == 2);
  assert(aprox_int_sqrt(5) == 3);
  assert(aprox_int_sqrt(8) == 3);
  assert(aprox_int_sqrt(9) == 3);
  assert(aprox_int_sqrt(10) == 4);
  assert(aprox_int_sqrt(16) == 4);
  assert(aprox_int_sqrt(17) == 5);
  assert(aprox_int_sqrt(25) == 5);
  assert(aprox_int_sqrt(26) == 6);
  assert(aprox_int_sqrt(30) == 6);
  assert(aprox_int_sqrt(239) == 16);

  assert(aprox_int_sqrt(3682169683) == 60681);
  assert(aprox_int_sqrt(1300989607) == 36070);
  assert(aprox_int_sqrt(3111547457) == 55782);
  assert(aprox_int_sqrt(2259503551) == 47535);

  assert(aprox_int_sqrt(-1) == 65536);
}

void test_is_prime()
{
  for (long i = 2; i < 100000; ++i)
      assert(checkPrimeNumber(i) == (int)is_prime(i));

  assert(!is_prime(0));
  assert(!is_prime(1));
  assert(is_prime(3682169683));
  assert(is_prime(1300989607));
  assert(is_prime(3111547457));
  assert(is_prime(2259503551));
}

void test_min_max()
{
  uint32_t arr[] = {};
  assert(product_of_min_and_max_primes(arr, sizeof(arr) / sizeof(uint32_t)) == 0);
  uint32_t arr1[] = {1};
  assert(product_of_min_and_max_primes(arr1, sizeof(arr1) / sizeof(uint32_t)) == 0);
  uint32_t arr2[] = {1, 4, 6, 8, 10, 12, 16, 14, 100, 25, 88};
  assert(product_of_min_and_max_primes(arr2, sizeof(arr2) / sizeof(uint32_t)) == 0);
  uint32_t arr3[] = {1, 4, 6, 8, 10, 12, 16, 14, 100, 25, 88, 2};
  assert(product_of_min_and_max_primes(arr3, sizeof(arr3) / sizeof(uint32_t)) == 4);
  uint32_t arr4[] = {1, 4, 6, 8, 10, 12, 16, 14, 100, 25, 88, 2, 3};
  assert(product_of_min_and_max_primes(arr4, sizeof(arr4) / sizeof(uint32_t)) == 6);
  uint32_t arr5[] = {5, 1, 4, 6, 8, 10, 101, 12, 16, 14, 100, 4, 25, 88};
  assert(product_of_min_and_max_primes(arr5, sizeof(arr5) / sizeof(uint32_t)) == 505);
  uint32_t arr6[] = {1, 4, 6, 8, 10, 5, 101, 12, 16, 14, 100, 4, 25, 88};
  assert(product_of_min_and_max_primes(arr6, sizeof(arr6) / sizeof(uint32_t)) == 505);
  uint32_t arr7[] = {0, 1, 7, 8, 9, 101, 239, 30, 123, 101, 97, 47, 89, 100};
  assert(product_of_min_and_max_primes(arr7, sizeof(arr7) / sizeof(uint32_t)) == 239 * 7);
  
  uint32_t arr8[] = {3682169683, 1300989607, 3111547457, 2259503551};
  assert(product_of_min_and_max_primes(arr8, sizeof(arr8) / sizeof(uint32_t)) == 4790464488793484581);
  uint32_t arr9[] = {2368502749, 1434667823};
  assert(product_of_min_and_max_primes(arr9, sizeof(arr9) / sizeof(uint32_t)) == 3398014682677345427);
  uint32_t arr10[] = {3682169682, 1300989606, 3111547458, 2259503552};
  assert(product_of_min_and_max_primes(arr10, sizeof(arr10) / sizeof(uint32_t)) == 0);
  uint32_t arr11[] = {2368502740, 1434667822};
  assert(product_of_min_and_max_primes(arr11, sizeof(arr11) / sizeof(uint32_t)) == 0);
}

int main() 
{
  test_sqrt();
  test_is_prime();
  test_min_max();

  printf("Tests passed!\n");
  return 0;
}
