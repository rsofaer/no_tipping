#ifndef _HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
#define _HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
#include "contestant_util.h"
#include "ntg.h"
#include "gtest/gtest.h"

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

TEST(BuildState, contestant_util)
{
  std::stringstream ssStateString(stateString);
  State state;
  ASSERT_TRUE(BuildState(ssStateString, &state));
}

}

#endif _HPS_NO_TIPPING_CONTESTANT_UTIL_GTEST_H_
