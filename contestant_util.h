#ifndef _HPS_NO_TIPPING_GAME_NTG_H_
#define _HPS_NO_TIPPING_GAME_NTG_H_
#include <iostream>
#include <istream>
#include <string>

namespace hps
{
namespace ntg
{
struct State;

inline bool ReadMaxEmptyLines(std::istream &input,
                              const int maxEmptyLines,
                              std::string* curLine)
{
  int tries = 0;
  do
  {
    if (tries > maxEmptyLines)
    {
      return false;
    }
    getline(input, *curLine);
    ++tries;
  } while (curLine->empty());
  return true;
}

bool BuildState(std::istream &input, State* stateBuffer);

std::string CalculateMoveWrapper(State* stateBuffer);

}
using namespace ntg;
}

#endif _HPS_NO_TIPPING_GAME_NTG_H_
