#ifndef _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#define _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#include "minimax.h"
#include "adversarial_utils.h"
#include "ntg.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_minimax_gtest_h_
{
using namespace hps;

TEST(DISABLED_Minimax, Minimax)
{
  State state;
  InitState(&state);
  Minimax::Params params;
  {
#if NDEBUG
    params.maxDepthAdding = 4;
    params.maxDepthRemoving = State::NumRemoved;
#else
    params.maxDepthAdding = 3;
    params.maxDepthRemoving = State::NumRemoved;
#endif
  }
  std::cout << "maxDepthAdding: " << params.maxDepthAdding << std::endl;
  std::cout << "maxDepthRemoving: " << params.maxDepthRemoving << std::endl;
  BoardEvaluationReachableWinStates evalFunc(State::Turn_Red);
  Ply ply;
  int minimax = Minimax::Run(&params, &state, &evalFunc, &ply);
  std::cout << "Minimax is " << minimax << " with ply (position: " << ply.pos
            << ", weight: " << state.red.hand[ply.wIdx] << ")." << std::endl;
}

}

#endif //_NO_TIPPING_GAME_MINIMAX_GTEST_H_
