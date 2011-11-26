#ifndef _NO_TIPPING_GAME_ALPHABETAPRUNING_H_
#define _NO_TIPPING_GAME_ALPHABETAPRUNING_H_
#include "ntg.h"
#include <omp.h>

namespace hps
{
namespace ntg
{

struct AlphaBetaPruning
{
  /// <summary> Helper struct to pass for thread-level processing. </summary>
  struct ThreadParams
  {
    ThreadParams()
      : state(),
        depth(-1),
        maxDepth(-1),
        bestMinimax(0),
        bestPlyIdx(-1),
        dfsPlys(State::NumRemoved + (2 * Player::NumWeights) - 2),
        victoryIsMine(NULL)
    {}

    State state;
    int depth;
    int maxDepth;
    int bestMinimax;
    int bestPlyIdx;
    std::vector<std::vector<Ply> > dfsPlys;
    volatile bool* victoryIsMine;
  };

  /// <summary> The parallel minimax parameters. </summary>
  struct Params
  {
    Params()
      : maxDepthAdding(3),
        maxDepthRemoving(8),
        depth(0),
        rootPlys(),
        threadData()
    {}

    int maxDepthAdding;
    int maxDepthRemoving;
    int depth;
    std::vector<Ply> rootPlys;
    std::vector<ThreadParams> threadData;
  };

  /// <summary> Run alpha-beta pruning to get the ply for the state. </summary>
  template <typename BoardEvaulationFunction>
  static int Run(Params* params,
                 State* state,
                 const BoardEvaulationFunction* evalFunc,
                 Ply* ply)
  {
    assert(params && state && evalFunc && ply);
    assert(!Tipped(state->board));

    int maxDepth;
    if (State::Phase_Adding == state->phase)
    {
      maxDepth = params->maxDepthAdding;
    }
    else
    {
      maxDepth = params->maxDepthRemoving;
    }
    int& depth = params->depth;
    assert(maxDepth > 1);
    assert(depth < maxDepth);
//    std::cout << "AlphaBetaPruning::Run() : maxDepth = " << maxDepth
//              << "." << std::endl;
    ++depth;

    // Get the children of the current state.
    std::vector<Ply>& plys = params->rootPlys;
    plys.clear();
    PossiblePlys(*state, &plys);
    std::sort(plys.begin(), plys.end(), PlyTorqueComp(state));
    // A leaf has no non-suicidal moves. Who won?
    volatile bool victoryIsMine = false;
    int minimax;
    if (plys.empty())
    {
      minimax = ScoreLeaf(depth, state, ply);
    }
    else
    {
      // Setup threads.
      std::vector<ThreadParams>& threadData = params->threadData;
      {
        const int numProcs = omp_get_num_procs();
        threadData.resize(numProcs);
        int initMinimax = std::numeric_limits<int>::min();
        for (int threadIdx = 0; threadIdx < numProcs; ++threadIdx)
        {
          ThreadParams& threadParams = threadData[threadIdx];
          {
            threadParams.bestMinimax = initMinimax;
            threadParams.bestPlyIdx = -1;
            threadParams.depth = depth;
            threadParams.maxDepth = maxDepth;
            threadParams.state = *state;
            threadParams.victoryIsMine = &victoryIsMine;
          }
        }
      }
      // Initialize alpha and beta for depth 1.
      const int alpha = std::numeric_limits<int>::min();
      const int beta = std::numeric_limits<int>::max();
      // Parallelize the first level.
#pragma omp parallel for schedule(dynamic)
      for (int plyIdx = 0; plyIdx < static_cast<int>(plys.size()); ++plyIdx)
      {
        const int threadIdx = omp_get_thread_num();
        ThreadParams& threadParams = threadData[threadIdx];
        if (!victoryIsMine)
        {
          // Apply the ply for this state.
          Ply& mkChildPly = plys[plyIdx];
          DoPly(mkChildPly, &threadParams.state);
          // Run on the subtree.
          const int minimax = RunThread(alpha, beta, &threadParams, evalFunc);
          // Undo the ply for the next worker.
          UndoPly(mkChildPly, &threadParams.state);
          // Collect best minimax for this thread.
          if ((-1 == threadParams.bestPlyIdx) ||
              (minimax > threadParams.bestMinimax))
          {
            threadParams.bestMinimax = minimax;
            threadParams.bestPlyIdx = plyIdx;
            if (std::numeric_limits<int>::max() == minimax)
            {
//              std::cout << "Thread " << threadIdx << " found victoryIsMine on "
//                        << "ply " << plyIdx << " of " << plys.size()
//                        << "." << std::endl;
              victoryIsMine = true;
            }
          }
        }
      }
      // Gather best result from all threads.
      {
        int bestPlyIdx;
        GatherRunThreadResults<std::greater<int> >(threadData,
                                                   &minimax, &bestPlyIdx);
        // Set MINIMax ply.
        *ply = plys[bestPlyIdx];
      }
    }

    --depth;
    if (std::numeric_limits<int>::min() == minimax)
    {
//      std::cout << "No guaranteed victory." << std::endl;
      // Select ply based on heuristic.
      DoPly(plys.front(), state);
      int bestPlyScore = (*evalFunc)(*state);
      *ply = plys.front();
      UndoPly(plys.front(), state);
      for (std::vector<Ply>::const_iterator testPly = plys.begin();
           testPly != plys.end();
           ++testPly)
      {
        DoPly(*testPly, state);
        int plyScore = (*evalFunc)(*state);
        if (plyScore > bestPlyScore)
        {
          bestPlyScore = plyScore;
          *ply = *testPly;
        }
        UndoPly(*testPly, state);
      }
    }
    return minimax;
  }

private:
  /// <summary> Sort plys for maximum torque. </summary>
  struct PlyTorqueComp
  {
    PlyTorqueComp(const State* state_)
    : state(state_)
    {}

    inline bool operator()(const Ply& lhs, const Ply& rhs)
    {
      if (lhs.wIdx >= 0)
      {
        const Weight* hand = CurrentPlayer(state)->hand;
        return abs(lhs.pos * hand[lhs.wIdx]) > abs(rhs.pos * hand[rhs.wIdx]);
      }
      else
      {
        const Board& board = state->board;
        return abs(lhs.pos * board[lhs.pos]) > abs(rhs.pos * board[rhs.pos]);
      }
    }

    const State* state;
  };

  inline static bool IdentifyMax(const int depth)
  {
    return depth & 1;
  }

  inline static int ScoreLeaf(const int depth, State* state, Ply* ply)
  {
    AnyPlyWillDo(state, ply);
    if (IdentifyMax(depth))
    {
      // I have no moves, so I lose.
      return std::numeric_limits<int>::min();
    }
    else
    {
      // You have no moves, so you lose.
      return std::numeric_limits<int>::max();
    }
  }

  template <typename MinimaxFunc>
  static void GatherRunThreadResults(const std::vector<ThreadParams>& data,
                                     int* minimax, int* bestPlyIdx)
  {
    std::vector<ThreadParams>::const_iterator result = data.begin();
    *minimax = result->bestMinimax;
    *bestPlyIdx = result->bestPlyIdx;
    MinimaxFunc minimaxFunc;
    for (; result < data.end(); ++result)
    {
      if (minimaxFunc(result->bestMinimax, *minimax))
      {
        *minimax = result->bestMinimax;
        *bestPlyIdx = result->bestPlyIdx;
      }
    }
  }

  template <typename MinimaxFunc, typename BoardEvaulationFunction>
  static void ABPruningChildrenHelper(ThreadParams* params,
                                      std::vector<Ply>::const_iterator testPly,
                                      std::vector<Ply>::const_iterator endPly,
                                      const BoardEvaulationFunction* evalFunc,
                                      const int* alpha,
                                      const int* beta,
                                      int* minimax)
  {
    assert(params && evalFunc);

    State* state = &params->state;
    volatile bool* victoryIsMine = params->victoryIsMine;
    // Score all plys to find minimax.
    MinimaxFunc minimaxFunc;
    for (; testPly != endPly; ++testPly)
    {
      DoPly(*testPly, state);
      int score = RunThread(*alpha, *beta, params, evalFunc);
      if (minimaxFunc(score, *minimax))
      {
        *minimax = score;
      }
      UndoPly(*testPly, state);
      if (*alpha >= *beta)
      {
        break;
      }
      if (*victoryIsMine)
      {
        //std::cout << "Got victory signal." << std::endl;
        *minimax = 0;
        break;
      }
    }
  }

  template <typename BoardEvaulationFunction>
  static int RunThread(const int a,
                       const int b,
                       ThreadParams* params,
                       const BoardEvaulationFunction* evalFunc)
  {
    assert(params && evalFunc);

    int& depth = params->depth;
    const int& maxDepth = params->maxDepth;
    State* state = &params->state;
    assert(!Tipped(state->board));
    assert(depth < maxDepth);
    ++depth;

    // Get the children of the current state.
    std::vector<Ply>& plys = params->dfsPlys[params->depth - 2];
    plys.clear();
    PossiblePlys(*state, &plys);
    std::sort(plys.begin(), plys.end(), PlyTorqueComp(state));
    // Collect incoming a and b.
    int alpha = a;
    int beta = b;
    // Is this a leaf?
    int minimax;
    if (plys.empty())
    {
      Ply tossPly;
      minimax = ScoreLeaf(depth, state, &tossPly);
    }
    // If depth bound reached, return score current state.
    else if (maxDepth == depth)
    {
      minimax = (*evalFunc)(*state);
    }
    else
    {
      // Init score.
      std::vector<Ply>::const_iterator testPly = plys.begin();
      {
        DoPly(*testPly, state);
        minimax = RunThread(alpha, beta, params, evalFunc);
        UndoPly(*testPly, state);
      }
      // If I am MAX, then maximize my score.
      if (IdentifyMax(depth))
      {
        alpha = minimax;
        ABPruningChildrenHelper<std::greater<int> >(params, ++testPly,
                                                    plys.end(),
                                                    evalFunc,
                                                    &alpha, &beta, &alpha);
        if (alpha >= beta)
        {
          minimax = beta;
        }
        else
        {
          minimax = alpha;
        }
      }
      // I am MIN. Minimize it.
      else
      {
        beta = minimax;
        ABPruningChildrenHelper<std::less<int> >(params, ++testPly,
                                                 plys.end(),
                                                 evalFunc,
                                                 &alpha, &beta, &beta);
        if (alpha >= beta)
        {
          minimax = alpha;
        }
        else
        {
          minimax = beta;
        }
      }
    }
    --depth;
    return minimax;
  }
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_ALPHABETAPRUNING_H_
