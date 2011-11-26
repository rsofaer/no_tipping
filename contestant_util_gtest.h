#ifndef _HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
#define _HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
#include "contestant_util.h"
#include "ntg.h"
#include "gtest/gtest.h"
#include "gtest/gtest.h"
#include <fstream>
#include <omp.h>

namespace _hps_no_tipping_contestant_util_gtest_h_
{
using namespace hps;

const char* stateString = "ADDING\n"
                          "0 0 Red 1\n"
                          "0 0 Blue 1\n"
                          "0 0 Red 2\n"
                          "0 0 Blue 2\n"
                          "0 0 Red 3\n"
                          "0 0 Blue 3\n"
                          "0 0 Red 4\n"
                          "0 0 Blue 4\n"
                          "0 0 Red 5\n"
                          "0 0 Blue 5\n"
                          "0 0 Red 6\n"
                          "0 0 Blue 6\n"
                          "0 0 Red 7\n"
                          "0 0 Blue 7\n"
                          "0 0 Red 8\n"
                          "0 0 Blue 8\n"
                          "0 0 Red 9\n"
                          "0 0 Blue 9\n"
                          "0 0 Red 10\n"
                          "0 0 Blue 10\n"
                          "1 -4 Green 3\n"
                          "STATE END\n";

TEST(contestant_util, BuildState)
{
  std::stringstream ssStateString(stateString);
  State state;
  ASSERT_TRUE(BuildState(ssStateString, &state));
}

TEST(contestant_util, CalculateMoveWrapper)
{
  omp_set_num_threads(omp_get_num_procs());
  enum { InputFileSuffixMin = 1, };
  enum { InputFileSuffixMax = 7, };

  for (int inputIdx = InputFileSuffixMin;
       inputIdx <= InputFileSuffixMax;
       ++inputIdx)
  {
    std::stringstream ssInputFilename;
    ssInputFilename << "input" << inputIdx;
    std::ifstream file(ssInputFilename.str().c_str());
    ASSERT_TRUE(file.good());
    State state;
    BuildState(file, &state);
    const std::string move = CalculateMoveWrapper(&state);
    ASSERT_FALSE(move.empty());
  }
}

}

#endif //_HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
