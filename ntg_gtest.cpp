#include "rand_bound_gtest.h"
#include "ntg_gtest.h"
#include "minimax_gtest.h"
#include "game_gtest.h"
#include "gtest/gtest.h"
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

int main(int argc, char** argv)
{
  srand(static_cast<unsigned int>(time(NULL)));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
