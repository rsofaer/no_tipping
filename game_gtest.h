#ifndef _NO_TIPPING_GAME_GAME_GTEST_H_
#define _NO_TIPPING_GAME_GAME_GTEST_H_
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

/// <summary> Default heuristic is no heuristic. </summary>
struct NoHeuristic
{
  template <typename PlayerType>
  inline void operator()(const int, const State&, PlayerType*) const
  { }
};

typedef std::map<Ply, int> PlyCountMap;
template <typename RedPlayer, typename BluePlayer, int Trials>
struct PlayerBattle
{
  /// <summary> Play without heuristics. </summary>
  inline static void Play(int* redWins)
  {
    Play(redWins, NoHeuristic(), NoHeuristic());
  }

  /// <summary> Play with heuristics. </summary>
  template <typename RedHeuristic, typename BlueHeuristic>
  static void Play(int* redWins,
                   const RedHeuristic redHeuristic,
                   const BlueHeuristic blueHeuristic)
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
          redHeuristic(turns, state, &red);
          red.NextPly(&state, &ply);
          moveTime = timer.GetTime();
          redTime += moveTime;
        }
        else
        {
          timer.Reset();
          blueHeuristic(turns, state, &blue);
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

//TEST(RandomVsRandom, NoTippingGames)
//{
//  enum { Games = 100, };
//  std::cout << "Playing " << Games << " games." << std::endl;
//  int redWins;
//  PlayerBattle<RandomPlayer, RandomPlayer, Games>::Play(&redWins);
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(0.5f, redWinProportion, 0.05f);
//}

//TEST(RandomVsMonteCarlo, NoTippingGames)
//{
//  enum { Games = 100, };
//  int redWins;
//  PlayerBattle<RandomPlayer, MonteCarloPlayer, Games>::Play(&redWins);
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
//}

struct MinimaxDepthHeuristics
{
  inline void operator()(const int turns,
                         const State& state,
                         MinimaxPlayer* player) const
  {
    assert(player);

    // Update win states when phase is changing.
    if ((turns > State::NumAdded) && (turns <= (State::NumAdded + 2)))
    {
      assert(State::Phase_Removing == state.phase);
      player->evalFunc.Update(state, 3, 7);
    }
    // Select minimax depth based on moves.
    if (turns > (State::NumAdded + 4))
    {
      player->params.maxDepthRemoving += 2;
    }
    else if (turns > (State::NumAdded + 2))
    {
      ++player->params.maxDepthRemoving;
    }
    else if (turns > State::NumAdded)
    {
#if NDEBUG
      player->params.maxDepthRemoving = 4;
#else
      player->params.maxDepthRemoving = 3;
#endif
    }
    else if (turns >= 17)
    {
#if NDEBUG
      player->params.maxDepthAdding = 4;
#endif
    }
    else
    {
#if NDEBUG
      player->params.maxDepthAdding = 3;
#else
      player->params.maxDepthAdding = 2;
#endif
    }
//    std::cout << "player->params.maxDepthAdding = "
//              << player->params.maxDepthAdding
//              << "." << std::endl;
//    std::cout << "player->params.maxDepthRemoving = "
//              << player->params.maxDepthRemoving
//              << "." << std::endl;
  }
};

//TEST(RandomVsMinimax, NoTippingGames)
//{
//  enum { Games = 100, };
//  int redWins;
//  PlayerBattle<RandomPlayer,
//               MinimaxPlayer,
//               Games>::Play(&redWins,
//                            NoHeuristic(),
//                            MinimaxDepthHeuristics());
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(0.0f, redWinProportion, 0.01f);
//}

//TEST(MinimaxVsRandom, NoTippingGames)
//{
//  enum { Games = 100, };
//  int redWins;
//  PlayerBattle<MinimaxPlayer,
//               RandomPlayer,
//               Games>::Play(&redWins,
//                            MinimaxDepthHeuristics(),
//                            NoHeuristic());
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
//}

struct AlphaBetaPruningDepthHeuristics
{
  inline void operator()(const int turns,
                         const State& state,
                         AlphaBetaPruningPlayer* player) const
  {
    assert(player);

    // Update win states when phase is changing.
    //if ((turns > State::NumAdded) && (turns <= (State::NumAdded + 2)))
    if ((turns > (State::NumAdded + 0)) && (turns <= (State::NumAdded + 2)))
    {
      //assert(State::Phase_Removing == state.phase);
      player->evalFunc.Update(state, 1, 5);
      std::cout << "Red win states: "
                << player->evalFunc.totalRedWinStates
                << ", Blue win states: "
                << player->evalFunc.totalBlueWinStates << std::endl;
    }
    // Select minimax depth based on moves.
    if (turns > (State::NumAdded + 2))
    {
#if NDEBUG
      player->params.maxDepthRemoving = State::MaxPlys;
#else
      player->params.maxDepthRemoving = 4;
#endif
    }
    else if (turns > State::NumAdded)
    {
#if NDEBUG
      player->params.maxDepthRemoving = 7;
#else
      player->params.maxDepthRemoving = 3;
#endif
    }
    else if (turns > 16)
    {
#if NDEBUG
      player->evalFunc.Update(state, 3, 5);
      player->params.maxDepthAdding = 4;
#else
      player->evalFunc.Update(state, 3, 5);
      player->params.maxDepthAdding = 2;
#endif
    }
    else
    {
#if NDEBUG
      player->evalFunc.Update(state, 3, 7);
      player->params.maxDepthAdding = 4;
#else
      player->evalFunc.Update(state, 3, 5);
      player->params.maxDepthAdding = 2;
#endif
    }
//    std::cout << "player->params.maxDepthAdding = "
//              << player->params.maxDepthAdding
//              << "." << std::endl;
//    std::cout << "player->params.maxDepthRemoving = "
//              << player->params.maxDepthRemoving
//              << "." << std::endl;
  }
};

TEST(RandomVsAlphaBetaPruning, NoTippingGames)
{
  enum { Games = 100, };
  int redWins;
  PlayerBattle<RandomPlayer,
               AlphaBetaPruningPlayer,
               Games>::Play(&redWins,
                            NoHeuristic(),
                            AlphaBetaPruningDepthHeuristics());
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(0.0f, redWinProportion, 0.01f);
}

TEST(AlphaBetaPruningVsRandom, NoTippingGames)
{
  enum { Games = 100, };
  int redWins;
  PlayerBattle<AlphaBetaPruningPlayer,
               RandomPlayer,
               Games>::Play(&redWins,
                            AlphaBetaPruningDepthHeuristics(),
                            NoHeuristic());
  std::cout << "Red won " << redWins << " times out of "
            << Games << " games." << std::endl;
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

//TEST(MinimaxVsMinimax, NoTippingGames)
//{
//  enum { Games = 10, };
//  int redWins;
//  PlayerBattle<MinimaxPlayer, MinimaxPlayer, Games>::Play(&redWins);
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
//}

//TEST(MinimaxVsMonteCarlo, NoTippingGames)
//{
//  enum { Games = 10, };
//  int redWins;
//  PlayerBattle<MinimaxPlayer, MonteCarloPlayer, Games>::Play(&redWins);
//  std::cout << "Red won " << redWins << " times out of "
//            << Games << " games." << std::endl;
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
//}

}

#endif //_NO_TIPPING_GAME_GAME_GTEST_H_
