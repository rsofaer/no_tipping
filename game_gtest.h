#ifndef _NO_TIPPING_GAME_GAME_GTEST_H_
#define _NO_TIPPING_GAME_GAME_GTEST_H_
#include "ntg_players.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_game_gtest_h_
{
using namespace hps;

void PrintPly(const Ply& ply, const State& state, const int turnCount)
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
              << " to position " << ply.pos << "." << std::endl;
  }
  // Removing.
  else
  {
    std::cout << " removes " << state.board[ply.pos]
              << " from position " << ply.pos << "." << std::endl;
  }
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

template <typename RedPlayer, typename BluePlayer, int Trials>
struct PlayerBattle
{
  static void Play(int* redWins)
  {
    *redWins = 0;
    for (int trial = 0; trial < Trials; ++trial)
    {
      RedPlayer red(State::Turn_Red);
      BluePlayer blue(State::Turn_Blue);
      State state;
      InitState(&state);
      int turns = 0;
      for (;;)
      {
        ++turns;
        Ply ply;
        if (State::Turn_Red == state.turn)
        {
          red.NextPly(&state, &ply);
        }
        else
        {
          blue.NextPly(&state, &ply);
        }
        //PrintPly(ply, state, turns);
        DoPly(ply, &state);
        if (Tipped(state.board))
        {
          *redWins += (State::Turn_Red == state.turn);
          LogWinner(state);
          break;
        }
      }
    }
  }
};

//TEST(RandomVsRandom, NoTippingGames)
//{
//  enum { RandomGames = 1000, };
//  int redWins;
//  PlayerBattle<RandomPlayer, RandomPlayer, RandomGames>::Play(&redWins);
//  const float redWinProportion = static_cast<float>(redWins) / RandomGames;
//  EXPECT_NEAR(0.5f, redWinProportion, 0.05f);
//}

//TEST(RandomVsMiniMax, NoTippingGames)
//{
//  enum { Games = 1000, };
//  int redWins;
//  PlayerBattle<RandomPlayer, MinimaxPlayer, Games>::Play(&redWins);
//  const float redWinProportion = static_cast<float>(redWins) / Games;
//  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
//}

TEST(MiniMaxVsRandom, NoTippingGames)
{
  enum { Games = 10, };
  int redWins;
  PlayerBattle<MinimaxPlayer, RandomPlayer, Games>::Play(&redWins);
  std::cout << "Red won " << redWins << " times\n";
  const float redWinProportion = static_cast<float>(redWins) / Games;
  EXPECT_NEAR(1.0f, redWinProportion, 0.01f);
}

}

#endif //_NO_TIPPING_GAME_GAME_GTEST_H_
