#ifndef _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#define _NO_TIPPING_GAME_MINIMAX_GTEST_H_
#include "minimax.h"
#include "adversarial_utils.h"
#include "no_tipping_game.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_minimax_gtest_h_
{
using namespace hps;

TEST(WinStates, Adversarial)
{
  // Count states where blue wins.
  {
    StatePlysPair oneW;
    const int numWinStates = OneWeightFinalSet(&oneW);
    std::cout << "Blue wins from " << numWinStates << " possible states "
              << "since Red cannot remove a weight." << std::endl;
    State& state = oneW.first;
    std::vector<Ply>& plys = oneW.second;
    for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
    {
      DoPly(plys[plyIdx], &state);
      EXPECT_FALSE(Tipped(state.board));
      UndoPly(plys[plyIdx], &state);
      EXPECT_TRUE(Tipped(state.board));
    }
  }
  // Count states where red wins.
  {
    std::vector<StatePlysPair> twoW;
    const int numWinStates = TwoWeightFinalSet(&twoW);
    std::cout << "Red wins from " << numWinStates << " possible states "
              << "since Blue cannot remove a weight." << std::endl;
    for (size_t pairIdx = 0; pairIdx < twoW.size(); ++pairIdx)
    {
      StatePlysPair& pair = twoW[pairIdx];
      State& state = pair.first;
      std::vector<Ply>& plys = pair.second;
      // Locate the base pos from the state.
      int basePos = -Board::Size;
      for (; basePos <= Board::Size; ++basePos)
      {
        if (Board::Empty != state.board[basePos])
        {
          break;
        }
      }
      for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
      {
        EXPECT_TRUE(Tipped(state.board));
        DoPly(plys[plyIdx], &state);
        EXPECT_FALSE(Tipped(state.board));
        // Now the other weight.
        {
          Weight wTmp = Board::Empty;
          std::swap(wTmp, state.board[basePos]);
          EXPECT_TRUE(Tipped(state.board));
          std::swap(wTmp, state.board[basePos]);
        }
        UndoPly(plys[plyIdx], &state);
      }
    }
  }
}

TEST(Minimax, Minimax)
{
  State state;
  InitState(&state);
  Minimax::Params params;
  {
    params.maxDepthAdding = 3;
    params.maxDepthRemoving = 5;
  }
  std::cout << "maxDepthAdding: " << params.maxDepthAdding << std::endl;
  std::cout << "maxDepthRemoving: " << params.maxDepthRemoving << std::endl;
  BoardEvaluationInverseDepthWinStates evalFunc(State::Turn_Red);
  Ply ply;
  int minimax = Minimax::Run(&params, &state, &evalFunc, &ply);
  std::cout << "Minimax is " << minimax << " with ply (position: " << ply.pos
            << ", weight: " << state.red.hand[ply.wIdx] << ")." << std::endl;
}

}

#endif //_NO_TIPPING_GAME_MINIMAX_GTEST_H_
