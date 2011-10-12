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
  State state;
  InitState(&state);
  // Count states where blue wins.
  {
    std::vector<Board> oneW;
    const int numWinStates = SingleWeightStatesNoConflict(state.board, &oneW);
    std::cout << "Blue wins from " << numWinStates << " possible states "
              << "since Red cannot remove a weight." << std::endl;
    for (int boardIdx = 0; boardIdx < numWinStates; ++boardIdx)
    {
      const Board& board = oneW[boardIdx];
      EXPECT_FALSE(Tipped(board));
      int weights = 0;
      for (Board::const_iterator w = board.begin(); w != board.end(); ++w)
      {
        weights += (*w != Board::Empty);
      }
      EXPECT_EQ(1, weights);
    }
  }
  // Count states where red wins.
  {
    std::vector<Board> twoW;
    const int numWinStates = DoubleWeightStatesNoConflictNoRemove(state.board,
                                                                  &twoW);
    std::cout << "Red wins from " << numWinStates << " possible states "
              << "since blue cannot remove a weight." << std::endl;
    Weight tmpW = Board::Empty;
    for (int boardIdx = 0; boardIdx < numWinStates; ++boardIdx)
    {
      Board& board = twoW[boardIdx];
      EXPECT_FALSE(Tipped(board));
      int weights = 0;
      for (Board::iterator w = board.begin(); w != board.end(); ++w)
      {
        if (*w != Board::Empty)
        {
          ++weights;
          std::swap(tmpW, *w);
          EXPECT_TRUE(Tipped(board));
          std::swap(tmpW, *w);
        }
      }
      EXPECT_EQ(2, weights);
    }
  }
}

TEST(Minimax, Minimax)
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
