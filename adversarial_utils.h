#ifndef _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
#define _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
#include "combination.h"

namespace hps
{
namespace ntg
{

/// <summary> Functional helper to identify states that conflict a given
///   game board.
/// </summary>
struct ConflictWithBoard
{
  /// <summary> Fast lookup to comparison board. </summary>
  typedef std::map<int, int> BoardMap;

  ConflictWithBoard(const Board& board, State* state_)
    : boardMap(),
      state(state_)
  {
    assert(state);
    // Compute fast lookup to board.
    for (int pos = -Board::Size; pos <= Board::Size; ++pos)
    {
      const Weight w = board[pos];
      if (Board::Empty != w)
      {
        boardMap[pos] = w;
      }
    }
  }

  /// <summary> Test the given ply at the base state for conflicts to
  ///   the comparison board.
  /// </summary>
  inline bool operator()(const Ply& ply) const
  {
    // Mutate the board with this ply.
    DoPly(ply, state);
    // Find conflict in the board to the comparison board.
    const Board& board = state->board;
    bool conflict = false;
    for (BoardMap::const_iterator boardPos = boardMap.begin();
         boardPos != boardMap.end();
         ++boardPos)
    {
      const Weight testW = board[boardPos->first];
      const bool posOccupied = (Board::Empty != testW);
      const bool posConflict = (boardPos->second != testW);
      if (posOccupied && posConflict)
      {
        conflict = true;
        break;
      }
    }
    // Undo mutation and return conflict.
    UndoPly(ply, state);
    return conflict;
  }

  BoardMap boardMap;
  State* state;
};

/// <summary> State and associated plys. </summary>
typedef std::pair<State, std::vector<Ply> > StatePlysPair;

/// <summary> Set of stable plys with one weight. </summary>
/// <remarks>
///   <para> The player to go last loses if any state with one weight is
///     reached since he must remove the final weight and tip the board.
///   </para>
///   <para> To win with one weight there cannot be a pivot at 0. </para>
/// </remarks>
/// </summary>
int OneWeightFinalSet(StatePlysPair* set)
{
  assert(set);
  State& state = set->first;
  std::vector<Ply>& plys = set->second;

  // This is really just the plys possible from the board being empty.
  InitState(&state);
  const Board initBoard = state.board;
  ClearBoard(&state.board);
  state.turn = State::Turn_Blue;
  PossiblePlys(state, &plys);
  // Remove all conflicts to initial board.
  plys.erase(std::remove_if(plys.begin(), plys.end(),
                            ConflictWithBoard(initBoard, &state)),
             plys.end());
  return static_cast<int>(plys.size());
}

struct StablePlyRemoveIfHelper
{
  inline bool operator()(const Ply& rhs) const
  {
    return (lhs.pos == rhs.pos) &&
           (lhs.wIdx == rhs.wIdx);
  }
  Ply lhs;
};

/// <summary> Set of plys stable with two weights such that removal of either
///   weight leads to a loss.
/// </summary>
int TwoWeightFinalSet(std::vector<StatePlysPair>* set)
{
  assert(set);
  set->clear();
  int setFlattenedSize = 0;

  // This is the set of states reachable from the suicidal one states.
  State blankState;
  InitState(&blankState);
  const Board initBoard = blankState.board;
  ClearBoard(&blankState.board);
  blankState.turn = State::Turn_Blue;
  std::vector<Ply> suicidalPlys;
  SuicidalPlys(blankState, &suicidalPlys);
  // Erase all conflicts to initial board.
  suicidalPlys.erase(std::remove_if(suicidalPlys.begin(), suicidalPlys.end(),
                                    ConflictWithBoard(initBoard, &blankState)),
                     suicidalPlys.end());
  // Now get all reachable states from the suicidal set. Since each suicidal
  // state is unique, then the set of reachable states for each suicidal state
  // is also unique.
  set->resize(suicidalPlys.size());
  std::vector<StatePlysPair>::iterator statePlys = set->begin();
  Board boardChkPly;
  ClearBoard(&boardChkPly);
  for (std::vector<Ply>::const_iterator suicidePly = suicidalPlys.begin();
       suicidePly != suicidalPlys.end();
       ++suicidePly, ++statePlys)
  {
    State& state = statePlys->first;
    std::vector<Ply>& plys = statePlys->second;
    state = blankState;
    // Mutate state to suicidal state.
    DoPly(*suicidePly, &state);
    // Get possible plys from suicidal state.
    PossiblePlys(state, &plys);
    // Only keep plys that do not stand on their own.
    for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
    {
      const int pos = plys[plyIdx].pos;
      boardChkPly[pos] = CurrentPlayer(&state)->hand[plys[plyIdx].wIdx];
      if (!Tipped(boardChkPly))
      {
        plys[plyIdx] = Ply();
      }
      boardChkPly[pos] = Board::Empty;
    }
    plys.erase(std::remove_if(plys.begin(), plys.end(),
                              StablePlyRemoveIfHelper()), plys.end());
    setFlattenedSize += static_cast<int>(plys.size());
  }
  return setFlattenedSize;
}

// reissb -- 20111001 --
//   Board evaluation function:
//     * One point for every winning state still reachable times
//       the inverse depth squared?
//       Something like this would favor more reachable win states and also
//       being closer to a win state.
//     * MAXINT for a winning state.
//     * Subtract inverse depth squared for every winning state of opponent
//       still reachable.
//     * -MAXINT for an opponent winning state.
struct BoardEvaluationInverseDepthWinStates
{
  struct BoardPosWeight
  {
    // Assumes 32-bit.
    BoardPosWeight(const int p_, const Weight w_)
      : key((p_ << 16) | w_), p(p_), w(w_)
    {}

    inline bool operator<(const BoardPosWeight& rhs) const
    {
      return key < rhs.key;
    }
    inline bool operator==(const BoardPosWeight& rhs) const
    {
      return rhs.key == key;
    }
    
    int key;
    int p;
    Weight w;
  };

  typedef std::map<BoardPosWeight, bool> BoardPosMap;
  typedef
    std::map<BoardPosWeight, std::vector<BoardPosWeight> >
    BoardPosPairsMap;

  BoardPosWeight StateFirstBPW(const State& state) const
  {
    // Find BoardPosWeight for this state.
    const Board& board = state.board;
    int p = -Board::Size;
    Board::const_iterator w = board.begin();
    for (; w < board.end(); ++w, ++p)
    {
      if (Board::Empty != *w)
      {
        break;
      }
    }
    return BoardPosWeight (p, *w);
  }

  /// <summary> Count blue win states reachable from the given board. </summary>
  int BlueWinStatesReachable(const Board& board)
  {
    int count = 0;
    // Find all blue win states included in the board.
    int p = -Board::Size;
    Board::const_iterator w = board.begin();
    for (; w < board.end(); ++w, ++p)
    {
      count += (Board::Empty != *w) &&
               (blueWinsBoardMap.count(BoardPosWeight(p, *w)) > 0);
    }
    return count;
  }

  /// <summary> Count red win states reachable from the given board. </summary>
  int RedWinStatesReachable(const Board& board)
  {
    // Find all red win states included in the board.
    bpws.clear();
    bpws.reserve(Board::Positions);
    int p = -Board::Size;
    Board::const_iterator w = board.begin();
    for (; w < board.end(); ++w, ++p)
    {
      if (Board::Empty != *w)
      {
        bpws.push_back(BoardPosWeight(p, *w));
      }
    }
    // Need at least one pair.
    if (bpws.size() < 2)
    {
      return 0;
    }
    // Go through all combinations of the bpws.
    int count = 0;
    Combination cmb;
    FastCombinationIterator cmbIter(bpws.size(), 2, 0);
    unsigned long long m;
    for (unsigned long long cmbIdx = 0;
         cmbIdx < cmbIter.GetCombinationCount();
         ++cmbIdx)
    {
      cmbIter.Next(&m, &cmb);
      assert(cmb[0] < cmb[1]);
      const BoardPosWeight& first = bpws[cmb[0]];
      if (redWinsBoardMap.count(first) > 0)
      {
        const std::vector<BoardPosWeight>& scndList = redWinsBoardMap[first];
        const BoardPosWeight& second = bpws[cmb[1]];
        std::vector<BoardPosWeight>::const_iterator pairScnd =
          std::lower_bound(scndList.begin(), scndList.end(), second);
        count += (scndList.end() != pairScnd) && (*pairScnd == second);
      }
    }
    return count;
  }

  BoardEvaluationInverseDepthWinStates()
    : blueWinsBoardMap(),
      redWinsBoardMap(),
      bpws()
  {
    // Get states where blue wins.
    {
      StatePlysPair oneW;
      OneWeightFinalSet(&oneW);
      std::vector<Ply>& plys = oneW.second;
      const Weight* hand = CurrentPlayer(&oneW.first)->hand;
      for (std::vector<Ply>::const_iterator ply = plys.begin();
           ply != plys.end();
           ++ply)
      {
        assert(0 == blueWinsBoardMap.count(BoardPosWeight(ply->pos,
                                                          hand[ply->wIdx])));
        blueWinsBoardMap[BoardPosWeight(ply->pos, hand[ply->wIdx])] = true;
      }
    }
    // Get states where red wins.
    {
      std::vector<StatePlysPair> twoW;
      TwoWeightFinalSet(&twoW);
      // Construct a map from a BoardPosWeight to all of the corresponding
      // BoardPosWeight that are in a winning state pair. Always address a pair
      // using the lowest position occupied.
      for (std::vector<StatePlysPair>::iterator statePlys = twoW.begin();
           statePlys != twoW.end();
           ++statePlys)
      {
        // Find the BoardPosWeight for the state.
        State& state = statePlys->first;
        const BoardPosWeight stateBPW = StateFirstBPW(state);
        // Find the BoardPosWeight for each ply.
        const Weight* hand = CurrentPlayer(&state)->hand;
        const std::vector<Ply>& plys = statePlys->second;
        for (std::vector<Ply>::const_iterator ply = plys.begin();
             ply != plys.end();
             ++ply)
        {
          const BoardPosWeight plyBPW(ply->pos, hand[ply->wIdx]);
          assert(plyBPW.p != stateBPW.p);
          if (plyBPW.p < stateBPW.p)
          {
            redWinsBoardMap[plyBPW].push_back(stateBPW);
          }
          else
          {
            redWinsBoardMap[stateBPW].push_back(plyBPW);
          }
        }
      }
      // Sort all lists of BoardPosWeight.
      for (BoardPosPairsMap::iterator pair = redWinsBoardMap.begin();
           pair != redWinsBoardMap.end();
           ++pair)
      {
        std::vector<BoardPosWeight>& pairSecondBpws = pair->second;
        std::sort(pairSecondBpws.begin(), pairSecondBpws.end());
      }
    }
  }

  /// <summary> Score a board. </summary>
  int operator()(const State& state)
  {
    // TODO(reissb) -- 201111003 -- Note that this is ASSUMING the red player.
    //   Need to make this parameterized for the actual MAX player.
    
    // Count win states reachable.
    const int redWinStatesReachable = RedWinStatesReachable(state.board);
    const int blueWinStatesReachable = BlueWinStatesReachable(state.board);
    // Return score based on win states.
    if ((1 == redWinStatesReachable) && (0 == blueWinStatesReachable))
    {
      return std::numeric_limits<int>::max();
    }
    else if ((0 == redWinStatesReachable) && (1 == blueWinStatesReachable))
    {
      return std::numeric_limits<int>::min();
    }
    else
    {
      return redWinStatesReachable - blueWinStatesReachable;
    }
  }

  BoardPosMap blueWinsBoardMap;
  BoardPosPairsMap redWinsBoardMap;

  /// <summary> Internal temporary. </summary>
  std::vector<BoardPosWeight> bpws;
};

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
