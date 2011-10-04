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

struct RandomPlayer
{
  RandomPlayer(const State::Turn) : plys() {}

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
  MinimaxPlayer(const State::Turn who)
    : params(),
      evalFunc(who)
  {
#ifndef NDEBUG
    params.maxDepthAdding = 3;
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
    Minimax::Run(&params, state, &evalFunc, ply);
    assert(ply->pos >= -Board::Size);
    assert(ply->pos <= Board::Size);
  }

  Minimax::Params params;
  BoardEvaluationInverseDepthWinStates evalFunc;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NTG_PLAYERS_H_
