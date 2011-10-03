#ifndef _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#define _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#include "minimax.h"
#include "adversarial_utils.h"
#include "no_tipping_game.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_minimax_gtest_h_
{
using namespace hps;

TEST(Minimax, Minimax)
{
  State state;
  InitState(&state);
  MiniMax::Params params;
  {
    params.depth = 0;
    params.maxDepth = 4;
    params.plys.resize(params.maxDepth - 1);
  }
  BoardEvaluationInverseDepthWinStates evalFunc;
  Ply ply;
  int minimax = MiniMax::Run(&params, &state, &evalFunc, &ply);
  std::cout << "Minimax is " << minimax << " with ply (position: " << ply.pos
            << ", weight: " << state.red.hand[ply.wIdx] << ")." << std::endl;
  bool checkit = true;
}

}

#endif //_NO_TIPPING_GAME_MINIMAX_GTEST_H_
