#ifndef _NO_TIPPING_GAME_GAME_GTEST_H_
#define _NO_TIPPING_GAME_GAME_GTEST_H_
#include "ntg.h"
#include "ntg_players.h"
#include "ntg_gtest_operators.h"
#include "timer.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_game_gtest_h_
{
using namespace hps;

void LogPly(const Ply& ply,
            const State& state,
            const int turnCount,
            const double& moveTime)
{
  std::cout << turnCount << ":";
  if (State::Turn_Red == state.turn)
  {
    std::cout << " red";
  }
  else
  {
    std::cout << " blue";
  }

  if (State::Phase_Adding == state.phase)
  {
    const Player* player = CurrentPlayer(&state);
    std::cout << " places " << player->hand[ply.wIdx]
              << " to position " << ply.pos;
  }
  // Removing.
  else
  {
    std::cout << " removes " << state.board[ply.pos]
              << " from position " << ply.pos;
  }
  std::cout << " using " << moveTime << "s." << std::endl;
}

void LogWinner(const State& state)
{
  if (State::Turn_Red == state.turn)
  {
    std::cout << "Red wins." << std::endl;
  }
  else
  {
    std::cout << "Blue wins." << std::endl;
  }
}

typedef std::map<Ply, int> PlyCountMap;
template <typename RedPlayer, typename BluePlayer, int Trials>
struct PlayerBattle
{
  /// <summary> Play with heuristics. </summary>
  static void Play(int* redWins)
  {
    Timer timer;
    *redWins = 0;
    for (int trial = 0; trial < Trials; ++trial)
    {
      double redTime = 0.0;
      double blueTime = 0.0;
      RedPlayer red(State::Turn_Red);
      BluePlayer blue(State::Turn_Blue);
      State state;
      InitState(&state);
      int turns = 0;
      for (;;)
      {
        ++turns;
        Ply ply;
        double moveTime;
        if (State::Turn_Red == state.turn)
        {
          timer.Reset();
          red.NextPly(&state, &ply);
          moveTime = timer.GetTime();
          redTime += moveTime;
        }
        else
        {
          timer.Reset();
          blue.NextPly(&state, &ply);
          moveTime = timer.GetTime();
          blueTime += moveTime;
        }
        ASSERT_GE(ply.pos, static_cast<int>(-Board::Size));
        ASSERT_LE(ply.pos, static_cast<int>(Board::Size));
        LogPly(ply, state, turns, moveTime);
        DoPly(ply, &state);
        if (Tipped(state.board))
        {
          *redWins += (State::Turn_Red == state.turn);
          LogWinner(state);
          break;
        }
      }
      EXPECT_LT(redTime, 120.0);
      EXPECT_LT(blueTime, 120.0);
      std::cout << "Red used " << redTime << "s." << std::endl;
      std::cout << "Blue used " << blueTime << "s." << std::endl;
    }
  }
};

TEST(NoTippingGames, RandomVsRandom)
{
  enum { Games = 100, };
  std::cout << "Playing " << Games << " games." << std::endl;
  int redWins;
  PlayerBattle<RandomPlayer, RandomPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(0.5f, redWinProportion, 0.05f);
}

TEST(NoTippingGames, DISABLED_RandomVsMinimax)
{
  enum { Games = 5, };
  int redWins;
  PlayerBattle<RandomPlayer, MinimaxPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(0.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, DISABLED_MinimaxVsRandom)
{
  enum { Games = 5, };
  int redWins;
  PlayerBattle<MinimaxPlayer, RandomPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, RandomVsAlphaBetaPruning)
{
  enum { Games = 100, };
  int redWins;
  PlayerBattle<RandomPlayer, AlphaBetaPruningPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(0.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, AlphaBetaPruningVsRandom)
{
  enum { Games = 100, };
  int redWins;
  PlayerBattle<AlphaBetaPruningPlayer, RandomPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, DISABLED_MinimaxVsMinimax)
{
  enum { Games = 2, };
  int redWins;
  PlayerBattle<MinimaxPlayer, MinimaxPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, DISABLED_RandomVsMonteCarlo)
{
  enum { Games = 100, };
  int redWins;
  PlayerBattle<RandomPlayer, MonteCarloPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

TEST(NoTippingGames, DISABLED_MinimaxVsMonteCarlo)
{
  enum { Games = 10, };
  int redWins;
  PlayerBattle<MinimaxPlayer, MonteCarloPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

}

#endif //_NO_TIPPING_GAME_GAME_GTEST_H_
