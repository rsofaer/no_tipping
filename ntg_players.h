#ifndef _NO_TIPPING_GAME_NTG_PLAYERS_H_
#define _NO_TIPPING_GAME_NTG_PLAYERS_H_
#include "minimax.h"
#include "adversarial_utils.h"
#include "no_tipping_game.h"
#include "rand_bound_generator.h"

namespace hps
{
namespace ntg
{

/// <summary> Function to assist in choosing a losing move. </summary>
void AnyPlyWillDo(State* state, Ply* ply)
{
  assert(state);
  // Find open board space.
  for (int pos = -Board::Size; pos <= Board::Size; ++pos)
  {
    if (Board::Empty != state->board[pos])
    {
      *ply = Ply(pos);
      break;
    }
  }
  // when adding, find a weight to place.
  if (State::Phase_Adding == state->phase)
  {
    // Take anything from our hand.
    const Player* player = CurrentPlayer(state);
    for (int wIdx = 0; wIdx < Player::NumWeights; ++wIdx)
    {
      if (Player::Played != player->hand[wIdx])
      {
        ply->wIdx = wIdx;
        break;
      }
    }
  }
}

struct RandomPlayer
{
  RandomPlayer() : plys() {}

  /// <summary> Return next ply without mutating the state. </summary>
  void NextPly(State* state, Ply* ply)
  {
    assert(state);
    assert(ply);
    assert(!Tipped(state->board));

    // See if there are moves.
    plys.clear();
    PossiblePlys(*state, &plys);
    // Take random move.
    const size_t plyCount = plys.size();
    if (plyCount > 0)
    {
      int randPly = RandBound(plyCount);
      *ply = plys[randPly];
    }
    // We lose.
    else
    {
      AnyPlyWillDo(state, ply);
    }
  }

  std::vector<Ply> plys;
};

struct MinimaxPlayer
{
  MinimaxPlayer()
    : params(),
      evalFunc()
  {
#ifndef NDEBUG
    params.maxDepthAdding = 2;
    params.maxDepthRemoving = 3;
    //params.maxDepthRemoving = std::numeric_limits<int>::max();
#else
    params.maxDepthAdding = 4;
    params.maxDepthRemoving = 6;
#endif
  }

  /// <summary> Return next ply without mutating the state. </summary>
  void NextPly(State* state, Ply* ply)
  {
    assert(state);
    assert(ply);
    assert(!Tipped(state->board));

    // Get the minimax move.
    int minimax = Minimax::Run(&params, state, &evalFunc, ply);
    if (ply->pos < -Board::Size)
    {
      bool why = true;
    }
    if (std::numeric_limits<int>::min() == minimax)
    {
      bool lost = true;
    }
    if (std::numeric_limits<int>::min() == minimax)
    {
      AnyPlyWillDo(state, ply);
    }
  }

  Minimax::Params params;
  BoardEvaluationInverseDepthWinStates evalFunc;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NTG_PLAYERS_H_
