#ifndef _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
#define _NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_

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

/// <summary> Set of stable plys with one weight. </summary>
/// <remarks>
///   <para> The player to go last loses if any state with one weight is
///     reached since he must remove the final weight and tip the board.
///   </para>
///   <para> To win with one weight there cannot be a pivot at 0. </para>
/// </remarks>
/// </summary>
void OneWeightFinalSet(State* state, std::vector<Ply>* set)
{
  assert(set);
  assert(state);
  set->clear();

  // This is really just the plys possible from the board being empty.
  InitState(state);
  const Board initBoard = state->board;
  ClearBoard(&state->board);
  state->turn = State::Turn_Blue;
  PossiblePlys(*state, set);
  // Remove all conflicts to initial board.
  set->erase(std::remove_if(set->begin(), set->end(),
                            ConflictWithBoard(initBoard, state)),
             set->end());
}

/// <summary> Set of plys stable with two weights such that removal of either
///   weight leads to a loss.
/// </summary>
void TwoWeightFinalSet(State* state, std::vector<Ply>* set)
{
  assert(set);
  assert(state);
  set->clear();

  // This is the set of states reachable from the suicidal one states.
  InitState(state);
  const Board initBoard = state->board;
  ClearBoard(&state->board);
  state->turn = State::Turn_Blue;
  std::vector<Ply> suicidalPlys;
  SuicidalPlys(*state, &suicidalPlys);
  // Erase all conflicts to initial board.
  suicidalPlys.erase(std::remove_if(suicidalPlys.begin(), suicidalPlys.end(),
                                    ConflictWithBoard(initBoard, state)),
                     suicidalPlys.end());
  // Now get all reachable states from the suicidal set. Since each suicidal
  // state is unique, then the set of reachable states for each suicidal state
  // is also unique.
  std::vector<Ply> plys;
  for (std::vector<Ply>::const_iterator suicidePly = suicidalPlys.begin();
       suicidePly != suicidalPlys.end();
       ++suicidePly)
  {
    plys.clear();
    // Mutate state to suicidal state.
    DoPly(*suicidePly, state);
    // Get possible plys from suicidal state.
    PossiblePlys(*state, &plys);
    set->reserve(set->size() + plys.size());
    std::copy(plys.begin(), plys.end(), std::back_inserter(*set));
    // Undo mutated stated.
    UndoPly(*suicidePly, state);
  }
}

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_ADVERSARIAL_UTILS_H_
