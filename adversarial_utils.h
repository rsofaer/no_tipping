#ifndef _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
#define _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
#include "combination.h"

#if NDEBUG
#define HPS_NTG_ASSERT_BOARD_HASH_NO_DUPS(states)
#else
#define HPS_NTG_ASSERT_BOARD_HASH_NO_DUPS(states)                              \
      for (BoardHashList::const_iterator hash = states.begin() + 1;            \
           hash != states.end();                                               \
           ++hash)                                                             \
      {                                                                        \
        assert(*hash != *(hash - 1));                                          \
      }
#endif

namespace hps
{
namespace ntg
{

namespace detail
{
/// <summary> Test board for conflict with a base board. </summary>
struct ConflictBoard
{
  ConflictBoard(const Board& board_)
    : board(board_)
  {}

  bool operator()(const Board& testBoard) const
  {
    // Find conflict in the board to the comparison board.
    Board::const_iterator baseW = board.begin();
    Board::const_iterator testW = testBoard.begin();
    for (; baseW != board.end(); ++baseW, ++testW)
    {
      const bool baseOccipied = (Board::Empty != *baseW);
      const bool testOccipied = (Board::Empty != *testW);
      const bool baseConflict = (*testW != *baseW);
      if (baseOccipied && testOccipied && baseConflict)
      {
        return true;
      }
    }
    return false;
  }

  Board board;
};

/// <summary> Identify states that conflict a given game board by applying a
///   ply to the given state.
/// </summary>
struct ConflictBoardFromState
{
  ConflictBoardFromState(const Board& board_, State* state_)
    : conflictBoard(board_),
      state(state_)
  {
    assert(state);
  }

  /// <summary> Test the given ply at the base state for conflicts to
  ///   the comparison board.
  /// </summary>
  inline bool operator()(const Ply& ply) const
  {
    DoPly(ply, state);
    const bool conflict = conflictBoard(state->board);
    UndoPly(ply, state);
    return conflict;
  }

  ConflictBoard conflictBoard;
  mutable State* state;
};

/// <summary> Adapter for single/double states exclusion. </summary>
template <typename ExcludeFunc>
struct ExclBoardAdapter
{
  ExclBoardAdapter(ExcludeFunc* exclFunc_, State* state_)
    : exclFunc(exclFunc_),
      state(state_)
  {}
  inline bool operator()(const Ply& ply)
  {
    DoPly(ply, state);
    const bool exclude = (*exclFunc)(state->board);
    UndoPly(ply, state);
    return exclude;
  }
  ExcludeFunc* exclFunc;
  State* state;
};

struct DoubleWeightWinStateHelper
{
  DoubleWeightWinStateHelper(const Board& board_)
    : board(board_)
  {}

  bool operator()(Board& testBoard) const
  {
    // Find conflict in the board to the comparison board.
    Board::const_iterator baseW = board.begin();
    Board::iterator testW = testBoard.begin();
    Weight tmpW = Board::Empty;
    for (; baseW != board.end(); ++baseW, ++testW)
    {
      const bool baseOccupied = (Board::Empty != *baseW);
      const bool testOccupied = (Board::Empty != *testW);
      const bool baseConflict = (*testW != *baseW);
      if (baseOccupied && testOccupied && baseConflict)
      {
        return true;
      }
      if (testOccupied)
      {
        std::swap(tmpW, *testW);
        const bool tipped = Tipped(testBoard);
        std::swap(tmpW, *testW);
        if (!tipped)
        {
          return true;
        }
      }
    }
    return false;
  }

  Board board;
};

/// <summary> Set of stable boards with one weight. </summary>
/// <remarks>
///   <para> The player to go last loses if any state with one weight is
///     reached since he must remove the final weight and tip the board.
///   </para>
///   <para> To win with one weight there cannot be a pivot at 0. </para>
/// </remarks>
/// </summary>
template <typename StorageType,
          typename StorageAdapterFunc,
          typename ExcludeFunc>
int SingleWeightStates(std::vector<StorageType>* set,
                       StorageAdapterFunc* adaptFunc,
                       ExcludeFunc* exclFunc)
{
  assert(set);
  assert(adaptFunc);
  assert(exclFunc);

  // Generate the plys possible from the board being empty.
  State state;
  InitState(&state);
  ClearBoard(&state.board);
  state.turn = State::Turn_Blue;
  std::vector<Ply> plys;
  PossiblePlys(state, &plys);
  ExclBoardAdapter<ExcludeFunc> exclAdapter(exclFunc, &state);
  plys.erase(std::remove_if(plys.begin(), plys.end(), exclAdapter), plys.end());
  // Adapt boards into storage.
  set->reserve(plys.size());
  set->clear();
  for (std::vector<Ply>::const_iterator ply = plys.begin();
       ply != plys.end();
       ++ply)
  {
    DoPly(*ply, &state);
    set->push_back((*adaptFunc)(state.board));
    UndoPly(*ply, &state);
  }
  return static_cast<int>(set->size());
}

template <typename PassType>
struct PassthroughAdapter
{
  inline const PassType& operator()(const PassType& type)
  {
    return type;
  }
};
template <typename ExclType>
struct ExcludeNoneFunc
{
  inline const bool operator()(const ExclType&)
  {
    return false;
  }
};

namespace detail
{
struct PlySortRecord
{
  PlySortRecord(const Ply& ply_)
    : ply(ply_),
      key((ply.pos << 16) | (ply.wIdx & 0xFFFF))
  {}
  inline bool operator<(const PlySortRecord& rhs) const
  {
    return key < rhs.key;
  }
  Ply ply;
  int key;
};
}

/// <summary> Set of plys stable with two weights. </summary>
template <typename StorageType,
          typename StorageAdapterFunc,
          typename ExcludeFunc>
int DoubleWeightStates(std::vector<StorageType>* set,
                       StorageAdapterFunc* adaptFunc,
                       ExcludeFunc* exclFunc)
{
  assert(set);
  assert(adaptFunc);
  assert(exclFunc);
  typedef
    std::map<detail::PlySortRecord, std::map<detail::PlySortRecord, int> >
    StateVisitedMap;
  StateVisitedMap statesVisited;

  // Set of all stable double weight states.
  State blankState;
  InitState(&blankState);
  Board& board = blankState.board;
  ClearBoard(&board);
  blankState.turn = State::Turn_Blue;
  std::vector<Ply> suicidalPlys;
  SuicidalPlys(blankState, &suicidalPlys);
  ExclBoardAdapter<ExcludeFunc> exclAdapter(exclFunc, &blankState);
  suicidalPlys.erase(std::remove_if(suicidalPlys.begin(),
                                    suicidalPlys.end(),
                                    exclAdapter), suicidalPlys.end());
  // Now get all reachable states from the suicidal set. Since each suicidal
  // state is unique, then the set of reachable states for each suicidal state
  // is also unique.
  std::vector<Ply> plys;
  for (std::vector<Ply>::const_iterator suicidePly = suicidalPlys.begin();
       suicidePly != suicidalPlys.end();
       ++suicidePly)
  {
    // Mutate state to suicidal state.
    DoPly(*suicidePly, &blankState);
    // Get possible plys from suicidal state.
    plys.clear();
    PossiblePlys(blankState, &plys);
    const size_t setNewSize = set->size() + plys.size();
    set->reserve(setNewSize);
    // Only keep plys that do not stand on their own.
    for (std::vector<Ply>::const_iterator ply = plys.begin();
         ply != plys.end();
         ++ply)
    {
      // Always key by lowest pos.
      bool visited;
      {
        detail::PlySortRecord suicidePlySortRec(*suicidePly);
        detail::PlySortRecord plySortRec(*ply);
        if (suicidePly->pos < ply->pos)
        {
          visited = (statesVisited[suicidePlySortRec].count(plySortRec) > 0);
          ++statesVisited[suicidePlySortRec][plySortRec];
        }
        else
        {
          visited = (statesVisited[plySortRec].count(suicidePlySortRec) > 0);
          ++statesVisited[plySortRec][suicidePlySortRec];
        }
      }
      if (!visited)
      {
        DoPly(*ply, &blankState);
        if (!(*exclFunc)(board))
        {
          set->push_back((*adaptFunc)(board));
        }
        UndoPly(*ply, &blankState);
      }
    }
    // Revert state.
    UndoPly(*suicidePly, &blankState);
  }
  return static_cast<int>(set->size());
}
}

/// <summary> Get states with one weight. </summary>
inline int SingleWeightStates(std::vector<Board>* set)
{
  static detail::PassthroughAdapter<Board> s_passthrough;
  static detail::ExcludeNoneFunc<Board> s_excl;
  return detail::SingleWeightStates(set, &s_passthrough, &s_excl);
}

/// <summary> Get states with one weight that do not conflict with the
///   given board.
/// </summary>
inline int SingleWeightStatesNoConflict(const Board& conflictBoard,
                                        std::vector<Board>* set)
{
  static detail::PassthroughAdapter<Board> s_passthrough;
  detail::ConflictBoard s_excl(conflictBoard);
  return detail::SingleWeightStates(set, &s_passthrough, &s_excl);
}

/// <summary> Get states with two weights. </summary>
inline int DoubleWeightStates(std::vector<Board>* set)
{
  static detail::PassthroughAdapter<Board> s_passthrough;
  static detail::ExcludeNoneFunc<Board> s_excl;
  return detail::DoubleWeightStates(set, &s_passthrough, &s_excl);
}

inline int DoubleWeightStatesNoConflictNoRemove(const Board& conflictBoard,
                                                std::vector<Board>* set)
{
  static detail::PassthroughAdapter<Board> s_passthrough;
  detail::DoubleWeightWinStateHelper s_excl(conflictBoard);
  return detail::DoubleWeightStates(set, &s_passthrough, &s_excl);
}

namespace detail
{
inline void HashPosWeight(const int p, const int w, unsigned int* h)
{
  assert(Board::Empty != w);

  // Encode position.
  {
    const unsigned int highorder = *h & 0xf8000000;
    *h = *h << 5;
    *h = *h ^ (highorder >> 27);
    *h = *h ^ p;
  }
  // Encode weight.
  {
    const unsigned int highorder = *h & 0xf8000000;
    *h = *h << 5;
    *h = *h ^ (highorder >> 27);
    *h = *h ^ w;
  }
}
}

unsigned int HashBoardCRC(const Board& board)
{
  // reissb -- 20111009 -- Taken from
  //   http://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/
  //     homework10/hashfuncs.html
  unsigned int h = 0;
  int p = -Board::Size;
  for (Board::const_iterator w = board.begin(); w != board.end(); ++w, ++p)
  {
    if (Board::Empty != *w)
    {
      detail::HashPosWeight(p, *w, &h);
    }
  }
  return h;
}

unsigned int HashPosWeightPairsCRC(const int count,
                                   const std::pair<int, int>* posWeightPairs)
{
  // reissb -- 20111009 -- Taken from
  //   http://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/
  //     homework10/hashfuncs.html
  unsigned int h = 0;
  const std::pair<int, int>* posWeightPairsEnd = posWeightPairs + count;
  for (const std::pair<int, int>* posWeightPair = posWeightPairs;
       posWeightPair != posWeightPairsEnd;
       ++posWeightPair)
  {
    detail::HashPosWeight(posWeightPair->first, posWeightPair->second, &h);
  }
  return h;
}

/// <summary> A key for a board state. </summary>
struct BoardHashKey
{
  explicit BoardHashKey(const unsigned int key_) : key(key_) {}
  BoardHashKey(const Board& board) : key(HashBoardCRC(board)) {}
  inline bool operator<(const BoardHashKey& rhs) const
  {
    return key < rhs.key;
  }
  inline bool operator==(const BoardHashKey& rhs) const
  {
    return key == rhs.key;
  }
  inline bool operator!=(const BoardHashKey& rhs) const
  {
    return !(*this == rhs);
  }
  unsigned int key;
};

struct BoardEvaluationReachableWinStates
{
  typedef std::vector<BoardHashKey> BoardHashList;
  struct NumWeightsWinStates
  {
    int numWeights;
    BoardHashList states;
  };
  typedef std::vector<NumWeightsWinStates> WinStateList;

  /// <summary> Adapt boards to keys. </summary>
  struct BoardHashStorageAdapter
  {
    inline BoardHashKey operator()(const Board& board) const
    {
      return BoardHashKey(board);
    }
  };

  BoardEvaluationReachableWinStates(const State::Turn who_)
    : who(who_),
      redWinStates(),
      totalRedWinStates(0),
      blueWinStates(),
      totalBlueWinStates(0)
  {
    State initState;
    InitState(&initState);
    // Get states where blue wins. These are all states where the board is
    // balanced with one weight left.
    {
      blueWinStates.push_back(NumWeightsWinStates());
      NumWeightsWinStates& singleWinStates = blueWinStates.back();
      singleWinStates.numWeights = 1;
      BoardHashList& states = singleWinStates.states;
      BoardHashStorageAdapter adapter;
      detail::ConflictBoard exclFunc(initState.board);
      detail::SingleWeightStates(&states, &adapter, &exclFunc);
      std::sort(states.begin(), states.end());
      HPS_NTG_ASSERT_BOARD_HASH_NO_DUPS(states);
      totalBlueWinStates = static_cast<int>(states.size());
    }
    // Get states where red wins. These are all states with two weights where
    // the removal of either weight is suicidal.
    {
      redWinStates.push_back(NumWeightsWinStates());
      NumWeightsWinStates& doubleWinStates = redWinStates.back();
      doubleWinStates.numWeights = 2;
      BoardHashList& states = doubleWinStates.states;
      BoardHashStorageAdapter adapter;
      detail::DoubleWeightWinStateHelper exclFunc(initState.board);
      detail::DoubleWeightStates(&states, &adapter, &exclFunc);
      std::sort(states.begin(), states.end());
      HPS_NTG_ASSERT_BOARD_HASH_NO_DUPS(states);
      totalRedWinStates = static_cast<int>(states.size());
    }
  }

  /// <summary> Gather the occupied positions from a board. </summary>
  static void CollectOccupiedPositions(const Board& board,
                                       int* positionsOccupied,
                                       int* positions)
  {
    assert(positionsOccupied);
    assert(positions);
    *positionsOccupied = 0;
    int *posOut = positions;
    int pos = -Board::Size;
    for (Board::const_iterator w = board.begin(); w != board.end(); ++w, ++pos)
    {
      if (Board::Empty != *w)
      {
        *posOut = pos;
        ++posOut;
        ++(*positionsOccupied);
      }
    }
  }

  /// <summary> Count blue win states reachable from the given board. </summary>
  int WinStatesReachable(const Board& board, const WinStateList& winStates) const
  {
    int count = 0;
    int positionsOccupied;
    int positions[Board::Positions];
    CollectOccupiedPositions(board, &positionsOccupied, positions);
    Combination cmb;
    std::pair<int, int> posWeightPairs[Board::Positions];
    unsigned long long m;
    // Find all win states included in the board.
    for (WinStateList::const_iterator winState = winStates.begin();
         winState != winStates.end();
         ++winState)
    {
      // Are there enough weights to satisfy these win states?
      const int numWeights = winState->numWeights;
      if (positionsOccupied < numWeights)
      {
        continue;
      }
      // See if any of the win states are reachable. Make combinations of
      // the filled board positions.
      const BoardHashList& states = winState->states;
      FastCombinationIterator cmbIter(positionsOccupied, numWeights, 0ULL);
      const int numCmb = static_cast<int>(cmbIter.GetCombinationCount());
      for (int cmbIdx = 0; cmbIdx < numCmb; ++cmbIdx)
      {
        // Get next combination and collect board pairs.
        cmbIter.Next(&m, &cmb);
        {
          std::pair<int, int>* posWeightPair = posWeightPairs;
          for (int cmbEleIdx = 0;
               cmbEleIdx < numWeights;
               ++cmbEleIdx, ++posWeightPair)
          {
            const int pos = positions[cmb[cmbEleIdx]];
            posWeightPair->first = pos;
            posWeightPair->second = board[pos];
          }
        }
        // See if this board is a win state.
        const BoardHashKey boardKey(HashPosWeightPairsCRC(numWeights,
                                                          posWeightPairs));
        const bool winState = std::binary_search(states.begin(),
                                                 states.end(),
                                                 boardKey);
        count += winState;
      }
    }
    return count;
  }

  /// <summary> Compute win states from the current state. </summary>
  void Update(const State& state,
              const int invDepthBegin,
              const int invDepthEnd)
  {
    assert(invDepthBegin < invDepthEnd);
    const Board& currentBoard = state.board;

    // Enumerate all possible combinations of weights at depth.
    int positionsOccupied;
    int positions[Board::Positions];
    CollectOccupiedPositions(currentBoard, &positionsOccupied, positions);
    Combination cmb;
    unsigned long long m;
    Board testBoard;
    ClearBoard(&testBoard);
    for (int invDepth = invDepthBegin;
         (invDepth != invDepthEnd) && (positionsOccupied >= invDepth);
         ++invDepth)
    {
      // Blue wins at odd depths and red at even.
      WinStateList* winStates;
      int* winStateCount;
      if (invDepth & 1)
      {
        winStates = &blueWinStates;
        winStateCount = &totalBlueWinStates;
      }
      else
      {
        winStates = &redWinStates;
        winStateCount = &totalRedWinStates;
      }
      // See if we have computed for this depth.
      // reissb -- 20111009 -- Just assert that we have not.
      for (WinStateList::iterator depthState = winStates->begin();
           depthState != winStates->end();
           ++depthState)
      {
        assert(depthState->numWeights != invDepth);
      }
      winStates->push_back(NumWeightsWinStates());
      NumWeightsWinStates& depthState = winStates->back();
      depthState.numWeights = invDepth;
      BoardHashList& states = depthState.states;
      // Enumerate the board weight combinations.
      FastCombinationIterator cmbIter(positionsOccupied, invDepth, 0);
      const int cmbCount = static_cast<int>(cmbIter.GetCombinationCount());
      for (int cmbIdx = 0; cmbIdx < cmbCount; ++cmbIdx)
      {
        cmbIter.Next(&m, &cmb);
        // Build the board;
        for (Combination::const_iterator cmbEle = cmb.begin();
             cmbEle != cmb.end();
             ++cmbEle)
        {
          const int pos = positions[*cmbEle];
          assert(Board::Empty == testBoard[pos]);
          testBoard[pos] = currentBoard[pos];
        }
        const bool stable = !Tipped(testBoard);
        if (stable)
        {
          // See if I can't remove a weight.
          bool winState = true;
          for (Combination::const_iterator cmbEle = cmb.begin();
               cmbEle != cmb.end();
               ++cmbEle)
          {
            const int pos = positions[*cmbEle];
            testBoard[pos] = Board::Empty;
            const bool tipped = !Tipped(testBoard);
            testBoard[pos] = currentBoard[pos];
            if (!tipped)
            {
              winState = false;
              break;
            }
          }
          if (winState)
          {
            states.push_back(BoardHashKey(testBoard));
            ++(*winStateCount);
          }
        }
        // Reset the board.
        for (Combination::const_iterator cmbEle = cmb.begin();
             cmbEle != cmb.end();
             ++cmbEle)
        {
          const int pos = positions[*cmbEle];
          assert(Board::Empty != testBoard[pos]);
          testBoard[pos] = Board::Empty;
        }
      }
      if (!states.empty())
      {
        std::sort(states.begin(), states.end());
        HPS_NTG_ASSERT_BOARD_HASH_NO_DUPS(states);
      }
      else
      {
        winStates->pop_back();
      }
    }
  }

  /// <summary> Score a board. </summary>
  int operator()(const State& state) const
  {
    assert(!Tipped(state.board));

    // Count win states reachable.
    const int redWinStatesReachable = WinStatesReachable(state.board,
                                                         redWinStates);
    const int blueWinStatesReachable = WinStatesReachable(state.board,
                                                          blueWinStates);
    // If we have no information about win states...
    const bool blind = (redWinStatesReachable + blueWinStatesReachable) > 0;
    int score;
    // I am red?
    if (State::Turn_Red == who)
    {
      score = redWinStatesReachable - blueWinStatesReachable;
      if (blind)
      {
        score = -blueWinStatesReachable;
      }
    }
    // I am blue.
    else
    {
      score = blueWinStatesReachable - redWinStatesReachable;
      if (blind)
      {
        score = -redWinStatesReachable;
      }
    }
    return score;
  }

  /// <summary> The player for whom we evaluate the board state. </sumamry>
  State::Turn who;
  /// <summary> List of red win states. </summary>
  WinStateList redWinStates;
  int totalRedWinStates;
  /// <summary> List of blue win states. </summary>
  WinStateList blueWinStates;
  int totalBlueWinStates;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
