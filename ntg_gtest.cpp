#include "ntg_gtest.h"
#include "minimax_gtest.h"
#include "game_gtest.h"
#include "gtest/gtest.h"
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <math.h>
void FindPrimes(const int limit, std::vector<int>* primes)
{
  assert(limit > 0);
  primes->clear();
  int candidate = 1;
  while (candidate < limit)
  {
    bool prime;
    do
    {
      prime = true;
      ++candidate;
      // reissb -- 20111009 -- Not sure if we need to add one.
      const int maxTest =
        static_cast<int>(sqrt(static_cast<float>(candidate))) + 1;
      std::vector<int>::const_iterator divTest = primes->begin();
      for(; (divTest != primes->end()) && (maxTest >= *divTest); ++divTest)
      {
        const int r = candidate % *divTest;
        if (0 == r)
        {
          prime = false;
          break;
        }
      }
    } while (!prime);
    primes->push_back(candidate);
  }
}

int main(int argc, char** argv)
{
//  std::vector<int> primes;
//  FindPrimes(2000000, &primes);
//  const int p = primes.back();
//  std::cout << "p: " << p << "." << std::endl;

  srand(static_cast<unsigned int>(time(NULL)));
  testing::InitGoogleTest(&argc, argv);
  testing::FLAGS_gtest_catch_exceptions = false;
  return RUN_ALL_TESTS();
}
