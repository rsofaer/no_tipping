#ifndef _NO_TIPPING_GAME_MINIMAX_H_
#define _NO_TIPPING_GAME_MINIMAX_H_
#include "no_tipping_game.h"
#include <omp.h>

namespace hps
{
namespace ntg
{

struct Minimax
{
  struct Params
  {
    Params()
      : maxDepthAdding(3),
        maxDepthRemoving(8),
        depth(0),
        plys(State::NumRemoved + (2 * Player::NumWeights) - 1)
    {}
    int maxDepthAdding;
    int maxDepthRemoving;
    int depth;
    std::vector<std::vector<Ply> > plys;
  };

  template <typename MinimaxFunc, typename BoardEvaulationFunction>
  static void MinimaxChildrenHelper(Params* params,
                                    State* state,
                                    std::vector<Ply>::const_iterator testPly,
                                    std::vector<Ply>::const_iterator endPly,
                                    BoardEvaulationFunction* evalFunc,
                                    Ply* ply,
                                    int* minimax)
  {
    assert(params && state && evalFunc && ply && minimax);
    // Score all plys to find minimax.
    MinimaxFunc minimaxFunc;
    // Parallelize the first level.
//    if (1 == params->depth)
//    {
//    }
//    else
    {
      for (; testPly != endPly; ++testPly)
      {
        DoPly(*testPly, state);
        Ply tossPly;
        int score = Run(params, state, evalFunc, &tossPly);
        if (minimaxFunc(score, *minimax))
        {
          *minimax = score;
          *ply = *testPly;
        }
        UndoPly(*testPly, state);
      }
    }
  }

  template <typename BoardEvaulationFunction>
  static int Run(Params* params,
                 State* state,
                 BoardEvaulationFunction* evalFunc,
                 Ply* ply)
  {
    assert(params && state && evalFunc && ply);
    int maxDepth = params->maxDepthAdding;
    bool completedAddingPhase = false;
    int removingDepth = -1;
    {
      const int remainSum = state->red.remain + state->blue.remain;
      if ((remainSum <= 0) && (-remainSum >= params->depth))
      {
        maxDepth = params->maxDepthRemoving;
        completedAddingPhase = true;
        removingDepth = -remainSum - params->depth + 1;
      }
    }
    assert(maxDepth > 1);
    assert(params->depth < maxDepth);
    ++params->depth;
    // Get the children of the current state.
    std::vector<Ply>& plys = params->plys[params->depth - 1];
    plys.clear();
    PossiblePlys(*state, &plys);
    // A leaf has no non-suicidal moves. Who won?
    int minimax;
    if (0 == plys.size())
    {
      AnyPlyWillDo(state, ply);
      if (params->depth & 1)
      {
        // I have no moves, so I lose.
        minimax = std::numeric_limits<int>::min();
      }
      else
      {
        // You have no moves, so you lose.
        minimax = std::numeric_limits<int>::max();
      }
      --params->depth;
      return minimax;
    }
    // If depth bound reached, return score current state.
    if (maxDepth == params->depth)
    {
      --params->depth;
      return (*evalFunc)(*state);
    }
    // Init score.
    std::vector<Ply>::const_iterator testPly = plys.begin();
    {
      *ply = *testPly;
      DoPly(*ply, state);
      Ply tossPly;
      minimax = Run(params, state, evalFunc, &tossPly);
      UndoPly(*ply, state);
    }
    // If I am MAX, then maximize my score. If I am MIN then minimize it.
    // I am MAX?
    if (params->depth & 1)
    {
      MinimaxChildrenHelper<std::greater<int> >(params, state,
                                                ++testPly, plys.end(),
                                                evalFunc, ply, &minimax);
    }
    // I am MIN.
    else
    {
      MinimaxChildrenHelper<std::less<int> >(params, state,
                                                ++testPly, plys.end(),
                                             evalFunc, ply, &minimax);
    }
    --params->depth;
    // reissb -- 20111004 -- Try to increse search as the search space
    //   decreases. This is becuase there are less moves further down
    //   the tree.
    if (completedAddingPhase && (0 == params->depth))
    {
      params->maxDepthRemoving += (0 == params->depth) * (removingDepth * 2);
      std::cout << "Increased depth to " << params->maxDepthRemoving
                << " from " << maxDepth << "." << std::endl;
    }
    return minimax;
  }
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_MINIMAX_H_
