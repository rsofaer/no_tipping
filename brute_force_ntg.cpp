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
enum { BoardWeight = 3, };
enum { BoardCofG = 0, };

/// <summary> A game board including played weights. </summary>
template <int PivotL_, int PivotR_, int Size_>
struct GenericBoard
{
  enum { Empty = 0, };

  inline void SetPos(const int pos, const Weight w)
  {
    assert(pos >= -size);
    assert(pos <= size);
    // rsofaer -- 20110929 -- Assert board not tipped.
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

  enum { Size = Size_ };
  enum { PivotL = PivotL_ };
  enum { PivotR = PivotR_ };
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

typedef GenericBoard<-3, -1, 10> Board;
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

  Board board;
  Player red;
  Player blue;
  Turn turn;
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
  state->turn = State::Turn_Red;
  InitPlayer(&state->red);
  InitPlayer(&state->blue);
  InitBoard(&state->board);
}

/// <summary> Unexplored move that will spawn a new state. </summary>
struct Ply
{
  Ply(const State& state_, const int target_, const Weight weight_)
    : state(state_),
      target(target_),
      weight(weight_)
  {}

  State state;
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
          plys->push_back(Ply(state, pos, *w));
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
      plys->push_back(Ply(state, pos, *w));
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
          plys->push_back(Ply(state, pos, *w));
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
inline void DoPly(const Ply& ply, State* result)
{
  assert(result);
  assert(Board::Empty == ply.state.board[ply.target]);
  *result = ply.state;
  // Update the board.
  result->board[ply.target] = ply.weight;
  // Swap the active player.
  {
    State::Turn& turn = result->turn;
    ++reinterpret_cast<int&>(turn);
    reinterpret_cast<int&>(turn) %= State::Turn_Count;
  }
}

/// <summary> Apply the ply and get the resulting state. </summary>
inline State DoPly(const Ply& ply)
{
  State newState;
  DoPly(ply, &newState);
  return newState;
}

/// <summary> Count possible plys in the game brute-force. </summary>
long long CountPossiblePlys(const State& root)
{
  long long plys = 0LL;
  std::vector<State> plysToExplore;
  plysToExplore.push_back(root);
  std::vector<Ply> currentPlys;
  long long loopCount = 0;
  enum { LoopsToReport = 1000000, };
  do
  {
    const State* currentState = &plysToExplore.back();
    currentPlys.clear();
    PossiblePlys(*currentState, &currentPlys);;
    plysToExplore.pop_back();
    std::transform(currentPlys.begin(), currentPlys.end(),
                   std::back_inserter(plysToExplore),
                   std::ptr_fun<const Ply&, State>(DoPly));
    plys += currentPlys.size();
    if (0 == (++loopCount % LoopsToReport))
    {
      std::cout << "After " << loopCount << " loops found "
                << plys << " plys with " << plysToExplore.size()
                << " plys left to explore." << std::endl;
    }
  } while (!plysToExplore.empty());
  return plys;
}

/// <summary> Functional helper to identify states that conflict a given
///   game board.
/// </summary>
struct ConflictWithBoard
{
  typedef std::map<int, int> BoardMap;

  ConflictWithBoard(const Board& board)
    : boardMap()
  {
    for (int pos = -Board::Size; pos <= Board::Size; ++pos)
    {
      const Weight w = board[pos];
      if (Board::Empty != w)
      {
        boardMap[pos] = w;
      }
    }
  }

  inline bool operator()(const State& state) const
  {
    const Board& board = state.board;
    for (BoardMap::const_iterator boardPos = boardMap.begin();
         boardPos != boardMap.end();
         ++boardPos)
    {
      const Weight testW = board[boardPos->first];
      const bool posOccupied = (Board::Empty != testW);
      const bool posConflict = (boardPos->second != testW);
      if (posOccupied && posConflict)
      {
        return true;
      }
    }
    return false;
  }

  BoardMap boardMap;
};

/// <summary> Set of stable states with one weight. </summary>
/// <remarks>
///   <para> The player to go last loses if any state with one weight is
///     reached since he must remove the final weight and tip the board.
///   </para>
///   <para> To win with one weight there cannt be a pivot at 0. </para>
///   </para>
/// </remarks>
/// </summray>
void OneWeightFinalSet(std::vector<State>* set)
{
  assert(set);
  set->clear();

  // This is really just the plys possible from the board being empty.
  State state;
  InitState(&state);
  const Board initBoard = state.board;
  ClearBoard(&state.board);
  state.turn = State::Turn_Blue;
  std::vector<Ply> plys;
  PossiblePlys(state, &plys);
  set->resize(plys.size());
  std::transform(plys.begin(), plys.end(),
                 set->begin(),
                 std::ptr_fun<const Ply&, State>(DoPly));
  // Remove all conflicts to initial board.
  set->erase(std::remove_if(set->begin(), set->end(),
                            ConflictWithBoard(initBoard)),
             set->end());
}

/// <summary> Set of states stable with two weights such that removal of either
///   weight leads to a loss.
/// </summary>
void TwoWeightFinalSet(std::vector<State>* set)
{
  assert(set);
  set->clear();

  // This is the set of states reachable from the suicidal one states.
  State state;
  InitState(&state);
  const Board initBoard = state.board;
  ClearBoard(&state.board);
  state.turn = State::Turn_Blue;
  std::vector<Ply> plys;
  SuicidalPlys(state, &plys);
  std::vector<State> suicidalOneSet;
  suicidalOneSet.resize(plys.size());
  std::transform(plys.begin(), plys.end(),
                 suicidalOneSet.begin(),
                 std::ptr_fun<const Ply&, State>(DoPly));
  // Erase all conflicts to initial board.
  suicidalOneSet.erase(std::remove_if(suicidalOneSet.begin(),
                                      suicidalOneSet.end(),
                                      ConflictWithBoard(initBoard)),
                       suicidalOneSet.end());
  // Now get all reachable states from the suicidal set. Since each suicidal
  // state is unique, then the set of reachable states for each suicidal state
  // is also unique.
  for (std::vector<State>::const_iterator suicideState = suicidalOneSet.begin();
       suicideState != suicidalOneSet.end();
       ++suicideState)
  {
    plys.clear();
    PossiblePlys(*suicideState, &plys);
    std::transform(plys.begin(), plys.end(),
                   std::back_inserter(*set),
                   std::ptr_fun<const Ply&, State>(DoPly));
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
    std::vector<State> oneW;
    OneWeightFinalSet(&oneW);
    std::cout << "Blue wins from " << oneW.size() << " possible states "
              << "since Red cannot remove a weight." << std::endl;
  }
  // Count states where red wins.
  {
    std::vector<State> twoW;
    TwoWeightFinalSet(&twoW);
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
