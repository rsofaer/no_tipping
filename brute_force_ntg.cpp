#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <assert.h>

typedef int Weight;

// Default game parameters.
enum { DefaultWeightCount = 7, };
enum { BoardSize = 10, };
enum { BoardPivotL = -3, };
enum { BoardPivotR = -1, };
enum { BoardWeight = 3, };
enum { BoardCofG = 0, };

/// <summary> A game board including played weights. </summary>
/// <remarks>
///   <para> The overloaded array operator is not zero-based. It is designed
///     for the layout of the game board such that Board[0] is the board
///     center, Board[n: n < 0] is the left side of the board and
///     Board[n: n > 0] is the right side of the board. For a board of size
///     N, the allowable indices are -N <= n <= N.
///   </para>
/// </remarks>
template <int PivotL_, int PivotR_, int Size_>
struct GenericBoard
{
  enum { Size = Size_ };
  enum { PivotL = PivotL_ };
  enum { PivotR = PivotR_ };
  enum { Empty = 0, };

  inline void SetPos(const int pos, const Weight w)
  {
    assert(pos >= -size);
    assert(pos <= size);
    // rsofaer -- 20110929 -- Assert board not tipped.
    // reissb -- 20111002 -- This should go into an external function
    //   bool BoardTipped(const Board&).
    positions[pos + 10] = w;
  }

  inline int GetPos(const int pos) const
  {
    return positions[pos + 10];
  }

  inline int& operator[](const int pos)
  {
    return positions[pos + 10];
  }

  inline const int& operator[](const int pos) const
  {
    return positions[pos + 10];
  }

  Weight positions[(2 * Size) + 1];
};

/// <summary> Player consists of a hand of weights. </summary>
template <int NumWeights_>
struct GenericPlayer
{
  enum { Played = -1, };
  enum { NumWeights = NumWeights_, };
  Weight hand[NumWeights_];
  int remain;
};

typedef GenericBoard<BoardPivotL, BoardPivotR, BoardSize> Board;
typedef GenericPlayer<DefaultWeightCount> Player;

/// <summary> A game state. Consists of board and players. </summary>
struct State
{
  enum Turn
  {
    Turn_Red = 0,
    Turn_Blue,
    Turn_Count,
  };

  enum Phase
  {
    Phase_Adding = 0,
    Phase_Removing,
    Phase_Count,
  };

  Board board;
  Player red;
  Player blue;
  Turn turn;
  Phase phase;
};

/// <summary> Get the current player from the state. </summary>
inline const Player* GetCurrentPlayer(const State& state)
{
  return (state.turn == State::Turn_Red) ? &state.red : &state.blue;
}

/// <summary> Initialize player to game defaults. </summary>
void InitPlayer(Player* player)
{
  static const Weight s_initW[Player::NumWeights] = { 1, 2, 3, 4, 5, 6, 7 };
  player->remain = Player::NumWeights;
  memcpy(player->hand, s_initW, sizeof(s_initW));
}

/// <summary> Clear the game board. </summary>
inline void ClearBoard(Board* board)
{
  assert(board);
  memset(board->positions, Board::Empty, sizeof(board->positions));
}

/// <summary> Initialize board to game defaults. </summary>
inline void InitBoard(Board* board)
{
  assert(board);
  ClearBoard(board);
  // Set the initial weight on the board.
  (*board)[-4] = 3;
}

/// <summary> Initialize state to game defaults. </summary>
inline void InitState(State* state)
{
  assert(state);
  InitBoard(&state->board);
  InitPlayer(&state->red);
  InitPlayer(&state->blue);
  state->turn = State::Turn_Red;
  state->phase = State::Phase_Adding;
}

/// <summary> Unexplored move that will spawn a new state. </summary>
/// <remarks>
///   <para> The ply must be managed in conjunction with its associated state.
///     the state is not copied for performance reasons. Note that all plys
///     are easily reversible. This property is ideal for tree traversal.
///   </para>
/// </remarks>
struct Ply
{
  Ply(const int target_, const Weight weight_)
    : target(target_),
      weight(weight_)
  {}

  int target;
  Weight weight;
};

namespace detail
{

/// <summary> Compute torque at a pivot for a given board. </summary>
template <int Pivot>
int Torque(const Board& board)
{
  int torque = (Pivot - BoardCofG) * BoardWeight;
  for (int pos = -Board::Size; pos <= Board::Size; ++pos)
  {
    torque += board[pos] * (Pivot - pos);
  }
  return torque;
}

/// <summary> Find all plys from a given state. </summary>
/// <remarks>
///   <para> The supplied LeftPivotOp determines the plys that are kept when
///     testing against the left pivot. The right pivot uses the opposite
///     logic. Passing the correct operator will yield non-suicidal moves,
///     suicidal moves, or similar sets with different boundaries.
///   </para>
/// </remarks>
template <typename LeftPivotOp>
void ExpandState(const State& state, std::vector<Ply>* plys)
{
  assert(plys->empty());

  const Board& board = state.board;
  const int torqueL = TorqueL(board);
  const int torqueR = TorqueR(board);
  const Player* currentPlayer = GetCurrentPlayer(state);
  int startPos = -Board::Size;
  for (const int* w = currentPlayer->hand;
       w < (currentPlayer->hand + Player::NumWeights);
       ++w)
  {
    // Is weight already played?
    if (Player::Played == *w)
    {
      continue;
    }
    // Compute for left pivot.
    {
      const std::binder2nd<LeftPivotOp> excl =
        std::bind2nd(LeftPivotOp(), 0);
      for (int pos = startPos; pos < Board::PivotL; ++pos)
      {
        if (board[pos] != Board::Empty)
        {
          continue;
        }
        const int weightTorque = (Board::PivotL - pos) * (*w);
        if (excl(weightTorque + torqueL))
        {
          continue;
        }
        else
        {
          plys->push_back(Ply(pos, *w));
        }
      }
    }
    // Compute between pivots.
    for (int pos = Board::PivotL; pos <= Board::PivotR; ++pos)
    {
      if (board[pos] != Board::Empty)
      {
        continue;
      }
      plys->push_back(Ply(pos, *w));
    }
    // Compute for right pivot.
    {
      const std::unary_negate<std::binder2nd<LeftPivotOp> > excl =
        std::not1(std::bind2nd(LeftPivotOp(), -1));
      for (int pos = Board::PivotR + 1; pos <= Board::Size; ++pos)
      {
        if (board[pos] != Board::Empty)
        {
          continue;
        }
        const int weightTorque = (Board::PivotR - pos) * (*w);
        if (excl(weightTorque + torqueR))
        {
          continue;
        }
        else
        {
          plys->push_back(Ply(pos, *w));
        }
      }
    }
  }
}

}

/// <summary> Compute torque around left pivot. </summary>
inline int TorqueL(const Board& board)
{
  return detail::Torque<Board::PivotL>(board);
}

/// <summary> Compute torque around right pivot. </summary>
inline int TorqueR(const Board& board)
{
  return detail::Torque<Board::PivotR>(board);
}

/// <summary> Discover all non suicidal plys from a given state. </summary>
inline void PossiblePlys(const State& state, std::vector<Ply>* plys)
{
  detail::ExpandState<std::greater<Weight> >(state, plys);
}

/// <summary> Discover all suicidal plys from a given state. </summary>
inline void SuicidalPlys(const State& state, std::vector<Ply>* plys)
{
  detail::ExpandState<std::less_equal<Weight> >(state, plys);
}

/// <summary> Apply the ply and get the resulting state. </summary>
/// <remarks>
///   <para> This function will mutate the incoming state. Its actions may
///     be undone easily with UndoPly(const Ply&, State*).
///   </para>
/// </remarks>
inline void DoPly(const Ply& ply, State* state)
{
  assert(state);
  if (State::Phase_Adding == state->phase)
  {
    assert(Board::Empty == state->board[ply.target]);
    // Update the board.
    state->board[ply.target] = ply.weight;
    // Swap the active player.
    {
      State::Turn& turn = state->turn;
      ++reinterpret_cast<int&>(turn);
      reinterpret_cast<int&>(turn) %= State::Turn_Count;
    }
  }
  else
  {
    assert(State::Phase_Removing == state->phase);
    assert(ply.weight == state->board[ply.target]);
    // Update the board.
    state->board[ply.target] = Board::Empty;
    // Swap the active player.
    {
      State::Turn& turn = state->turn;
      ++reinterpret_cast<int&>(turn);
      reinterpret_cast<int&>(turn) %= State::Turn_Count;
    }
  }
}

/// <summary> Revserse the ply and get the resulting state. </summary>
inline void UndoPly(const Ply& ply, State* state)
{
  assert(state);
  if (State::Phase_Adding == state->phase)
  {
    assert(ply.weight == state->board[ply.target]);
    // Update the board.
    state->board[ply.target] = Board::Empty;
    // Swap the active player.
    {
      State::Turn& turn = state->turn;
      ++reinterpret_cast<int&>(turn);
      reinterpret_cast<int&>(turn) %= State::Turn_Count;
    }
  }
  else
  {
    assert(State::Phase_Removing == state->phase);
    assert(Board::Empty == state->board[ply.target]);
    // Update the board.
    state->board[ply.target] = ply.weight;
    // Swap the active player.
    {
      State::Turn& turn = state->turn;
      ++reinterpret_cast<int&>(turn);
      reinterpret_cast<int&>(turn) %= State::Turn_Count;
    }
  }
}

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

// reissb -- 20111001 --
//   Board evaluation function:
//     * One point for every winning state still reachable times
//       the inverse depth squared?
//       Something like this would favor more reachable win states and also
//       being closer to a win state.
//     * Size(WinningStates) * (MAX_DEPTH^2) for a winning state.
//     * Subtract inverse depth squared for every winning state of opponent
//       still reachable.

int main(int /*argc*/, char** /*argv*/)
{
  // Find plys from initial state.
  {
    State state;
    InitState(&state);
    std::vector<Ply> plys;
    PossiblePlys(state, &plys);
    std::cout << "There are " << plys.size() << " plys from the initial "
              << "game state." << std::endl;
  }
  // Count states where blue wins.
  {
    State stateOneW;
    std::vector<Ply> oneW;
    OneWeightFinalSet(&stateOneW, &oneW);
    std::cout << "Blue wins from " << oneW.size() << " possible states "
              << "since Red cannot remove a weight." << std::endl;
  }
  // Count states where red wins.
  {
    State stateTwoW;
    std::vector<Ply> twoW;
    TwoWeightFinalSet(&stateTwoW, &twoW);
    std::cout << "Red wins from " << twoW.size() << " possible states "
              << "since Blue cannot remove a weight." << std::endl;
  }
//  // Count plys brute-force.
//  {
//    const long long totalPlys = CountPossiblePlys(state);
//    std::cout << "There are " << totalPlys << " possible non-suicidal "
//              << "plys in the no tipping game." << std::endl;
//  }
  return 0;
}
