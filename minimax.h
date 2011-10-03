#ifndef _NO_TIPPING_GAME_MINIMAX_H_
#define _NO_TIPPING_GAME_MINIMAX_H_
#include "no_tipping_game.h"

namespace hps
{
namespace ntg
{

struct MiniMax
{
  struct Params
  {
    int maxDepth;
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
    for (; testPly != endPly; ++testPly)
    {
      DoPly(*testPly, state);
      int score = Run(params, state, evalFunc, ply);
      if (minimaxFunc(score, *minimax))
      {
        *minimax = score;
        *ply = *testPly;
      }
      UndoPly(*testPly, state);
    }
  }

  template <typename BoardEvaulationFunction>
  static int Run(Params* params,
                 State* state,
                 BoardEvaulationFunction* evalFunc,
                 Ply* ply)
  {
    assert(params && state && evalFunc && ply);
    assert(params->maxDepth > 0);
    assert(params->depth < params->maxDepth);
    ++params->depth;
    // If depth bound reached, return score current state.
    if (params->maxDepth == params->depth)
    {
      --params->depth;
      return (*evalFunc)(*state);
    }
    // Get the children of the current state.
    std::vector<Ply>& plys = params->plys[params->depth - 1];
    plys.clear();
    PossiblePlys(*state, &plys);
    // If this is a leaf, return score current state.
    if (0 == plys.size())
    {
      --params->depth;
      return (*evalFunc)(*state);
    }
    // If I am MAX, then maximize my score. If I am MIN then minimize it.
    int minimax;
    // I am MAX?
    if (params->depth & 1)
    {
      minimax = std::numeric_limits<int>::min();
      MinimaxChildrenHelper<std::greater<int> >(params, state,
                                                plys.begin(), plys.end(),
                                                evalFunc, ply, &minimax);
    }
    // I am MIN.
    else
    {
      minimax = std::numeric_limits<int>::max();
      MinimaxChildrenHelper<std::less<int> >(params, state,
                                             plys.begin(), plys.end(),
                                             evalFunc, ply, &minimax);
    }
    --params->depth;
    return minimax;
  }
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_MINIMAX_H_
