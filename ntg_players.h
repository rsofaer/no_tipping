#ifndef _NO_TIPPING_GAME_NTG_PLAYERS_H_
#define _NO_TIPPING_GAME_NTG_PLAYERS_H_
#include "alphabetapruning.h"
#include "minimax.h"
#include "adversarial_utils.h"
#include "ntg.h"
#include "rand_bound.h"

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
  /// <summary> Modify depth to achieve max performance. </summary>
  struct MinimaxDepthHeuristics
  {
    inline static void Apply(const int turns,
                             const State& state,
                             MinimaxPlayer* player)
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

    // Optimize parameters.
    if (State::Phase_Adding == state->phase)
    {
      const int turns = State::NumAdded -
                        (state->red.remain + state->blue.remain);
      MinimaxDepthHeuristics::Apply(turns + 1, *state, this);
    }
    else
    {
      const int turns = State::NumAdded +
                        abs(state->red.remain + state->blue.remain);
      MinimaxDepthHeuristics::Apply(turns + 1, *state, this);
    }

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
  /// <summary> Modify depth to achieve max performance. </summary>
  struct AlphaBetaPruningDepthHeuristics
  {
    inline static void Apply(const int turns,
                             const State& state,
                             AlphaBetaPruningPlayer* player)
    {
      assert(player);

      // Update win states when phase is changing.
      //if ((turns > State::NumAdded) && (turns <= (State::NumAdded + 2)))
      if ((turns > (State::NumAdded + 0)) && (turns <= (State::NumAdded + 2)))
      {
        //assert(State::Phase_Removing == state.phase);
        player->evalFunc.Update(state, 1, 7);
//        std::cout << "Red win states: "
//                  << player->evalFunc.totalRedWinStates
//                  << ", Blue win states: "
//                  << player->evalFunc.totalBlueWinStates << std::endl;
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
        player->params.maxDepthRemoving = 6;
  #else
        player->params.maxDepthRemoving = 3;
  #endif
      }
      else if (turns > 16)
      {
  #if NDEBUG
        player->evalFunc.Update(state, 3, 7);
        player->params.maxDepthAdding = 5;
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

    // Optimize parameters.
    if (State::Phase_Adding == state->phase)
    {
      const int turns = State::NumAdded -
                        (state->red.remain + state->blue.remain);
      AlphaBetaPruningDepthHeuristics::Apply(turns + 1, *state, this);
    }
    else
    {
      const int turns = State::NumAdded +
                        abs(state->red.remain + state->blue.remain);
      AlphaBetaPruningDepthHeuristics::Apply(turns + 1, *state, this);
    }

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
  struct PlyLtOp
  {
    inline bool operator()(const Ply& lhs, const Ply& rhs) const
    {
      return (lhs.pos < rhs.pos) && (lhs.wIdx < rhs.wIdx);
    }
  };

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

  typedef std::map<Ply, int, PlyLtOp> PlyCountMap;
  PlyCountMap plyCountMap;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NTG_PLAYERS_H_
