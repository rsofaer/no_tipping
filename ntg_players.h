#ifndef _NO_TIPPING_GAME_NTG_PLAYERS_H_
#define _NO_TIPPING_GAME_NTG_PLAYERS_H_
#include "minimax.h"
#include "alphabetapruning.h"
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
  MinimaxPlayer(const State::Turn who_)
    : who(who_),
      params(),
      evalFunc(who)
  {
#if NDEBUG
    params.maxDepthAdding = 4;
    // Search the whole tree once we arrive at the remove phase.
    params.maxDepthRemoving = State::NumRemoved;
#else
    params.maxDepthAdding = 3;
    params.maxDepthRemoving = 3;
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

  State::Turn who;
  Minimax::Params params;
  BoardEvaluationReachableWinStates evalFunc;
};

struct AlphaBetaPruningPlayer
{
  AlphaBetaPruningPlayer(const State::Turn who_)
    : who(who_),
      params(),
      evalFunc(who)
  {
#if NDEBUG
    params.maxDepthAdding = 4;
    // Search the whole tree once we arrive at the remove phase.
    params.maxDepthRemoving = State::NumRemoved;
#else
    params.maxDepthAdding = 3;
    params.maxDepthRemoving = 3;
#endif
  }

  /// <summary> Return next ply without mutating the state. </summary>
  void NextPly(State* state, Ply* ply)
  {
    assert(state);
    assert(ply);
    assert(!Tipped(state->board));

    // Get the minimax move.
    AlphaBetaPruning::Run(&params, state, &evalFunc, ply);
    assert(ply->pos >= -Board::Size);
    assert(ply->pos <= Board::Size);
  }

  State::Turn who;
  AlphaBetaPruning::Params params;
  BoardEvaluationReachableWinStates evalFunc;
};

struct MonteCarloPlayer
{
  MonteCarloPlayer(const State::Turn who_) : who(who_), plyCountMap() {}

  /// <summary> Return next ply without mutating the state. </summary>
  void NextPly(const State* state, Ply* ply)
  {
    assert(state);
    assert(ply);
    assert(!Tipped(state->board));

    // Simulate.
    plyCountMap.clear();
    for (int trial = 0; trial < 1000; ++trial)
    {
      State modifyState = *state;
      // Players.
      RandomPlayer me(who);
      State::Turn other = who;
      NextTurn(&other);
      RandomPlayer you(other);
      // Get first ply.
      Ply firstPly;
      {
        me.NextPly(&modifyState, &firstPly);
      }
      *ply = firstPly;
      DoPly(*ply, &modifyState);
      while (!Tipped(modifyState.board))
      {
        if (who == modifyState.turn)
        {
          me.NextPly(&modifyState, ply);
        }
        else
        {
          you.NextPly(&modifyState, ply);
        }
        DoPly(*ply, &modifyState);
      }
      // Did I win?
      if (who == modifyState.turn)
      {
        ++plyCountMap[firstPly];
      }
    }
    // Gather best ply.
    int maxCount = std::numeric_limits<int>::min();
    for (PlyCountMap::const_iterator plyCount = plyCountMap.begin();
         plyCount != plyCountMap.end();
         ++plyCount)
    {
      if (plyCount->second > maxCount)
      {
        maxCount = plyCount->second;
        *ply = plyCount->first;
      }
    }
  }

  State::Turn who;

  typedef std::map<Ply, int> PlyCountMap;
  PlyCountMap plyCountMap;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NTG_PLAYERS_H_
