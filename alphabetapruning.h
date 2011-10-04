#ifndef _NO_TIPPING_GAME_ALPHABETAPRUNING_H_
#define _NO_TIPPING_GAME_ALPHABETAPRUNING_H_
#include "no_tipping_game.h"
#include "algorithm"

namespace hps
{
namespace ntg
{

  //<summary> This stcruture will essentially remain the same, except for the functions. It's a separate file to make it easier to decide what to package in the final build.
struct MiniMax
{
  struct Params
  {
    int maxDepth;
    int depth;
    std::vector<std::vector<Ply> > plys;
  };

  template <typename MinimaxFunc, typename BoardEvaulationFunction>
  static void AlphaBetaChildrenHelper(Params* params,
                                    State* state,
                                    std::vector<Ply>::const_iterator testPly,
                                    std::vector<Ply>::const_iterator endPly,
                                    BoardEvaulationFunction* evalFunc,
				    Ply* ply)

  {
    assert(params && state && evalFunc && ply);
    // Score all plys to find minimax.

    MinimaxFunc minimaxFunc;
    int minimax;
    for (; testPly != endPly; ++testPly)
    {
      
	DoPly(*testPly, state);
	int score = Run(params, state, evalFunc, ply);
	if (minimaxFunc(score, minimax))
	{
	  minimax = score;
	  *ply = *testPly;
	}
	UndoPly(*testPly, state);
    }
    return minimax;
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
    // Required here, we don't wanna go beyond a certain depth.
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
    // stop if beta <= alpha, we cannot do better. 
    // This may look suboptimal but the key point to note is that the other player
    // is also bounded by time.
    int alpha;
    int beta;
    int retVal;
    // I am MAX?
    if (params->depth & 1)
    {
      alpha = std::numeric_limits<int>::min();
      int score = AlphaBetaHelper<std::greater<int> >(params, state,
                                                plys.begin(), plys.end(),
                                                evalFunc, ply);
      alpha = max(alpha, score);
      if(beta <= alpha)
      {
	retVal = alpha;
	break;
      }
    }
    // I am MIN.
    else
    {
      beta = std::numeric_limits<int>::max();
      int score = AlphaBetaHelper<std::less<int> >(params, state,
                                             plys.begin(), plys.end(),
                                             evalFunc, ply);
      beta = min(beta, score);
      if(beta <= alpha)
      {
	retVal = beta;
	return beta;
      }
    }
    --params->depth;
    return retVal;
  }
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_ALPHABETAPRUNING_H_
