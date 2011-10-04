#ifndef _NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
#define _NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
#include "no_tipping_game.h"
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

#endif //_NO_TIPPING_GAME_NTG_GTEST_UTILS_H_
