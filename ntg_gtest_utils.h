#ifndef _NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
#define _NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
#include "ntg.h"
#include <vector>
#include <algorithm>

template <int Length>
struct RandomPlayOrderGenerator
{
  RandomPlayOrderGenerator()
    : seq(Length)
  {
    for (int i = 0; i < static_cast<int>(seq.size()); ++i)
    {
      seq[i] = i;
    }
  }
  std::vector<int> operator()() const
  {
    std::random_shuffle(seq.begin(), seq.end());
    return seq;
  }
  mutable std::vector<int> seq;
};

bool PlayerEmptyHand(const hps::Player& player)
{
  if (player.remain > 0)
  {
    return false;
  }
  for (int handIdx = 0; handIdx < hps::Player::NumWeights; ++handIdx)
  {
    if (hps::Player::Played != player.hand[handIdx])
    {
      return false;
    }
  }
  return true;
}

void RandomRemovingPhase(hps::State* state)
{
  using namespace hps;
  Board& board = state->board;
  InitState(state);
  // Fill with weights.
  {
    assert(Board::Positions >= (2 * Player::NumWeights));
    Weight* boardVal = board.begin();
    for (int wIdx = 0; wIdx < Player::NumWeights; ++wIdx)
    {
      {
        Player* player = &state->red;
        Weight* w = player->hand + wIdx;
        while (Board::Empty != *boardVal) { ++boardVal; }
        *boardVal++ = *w;
        *w = Player::Played;
        --player->remain;
      }
      {
        Player* player = &state->blue;
        Weight* w = player->hand + wIdx;
        while (Board::Empty != *boardVal) { ++boardVal; }
        *boardVal++ = *w;
        *w = Player::Played;
        --player->remain;
      }
    }
  }
  // Find stable ordering.
  do
  {
    std::random_shuffle(board.begin(), board.end());
  } while (Tipped(board));
  // Switch phase.
  state->phase = State::Phase_Removing;
}

#endif //_NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
