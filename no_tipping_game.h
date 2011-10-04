#ifndef _NO_TIPPING_GAME_NO_TIPPING_GAME_H_
#define _NO_TIPPING_GAME_NO_TIPPING_GAME_H_
#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <limits>
#include <cstring>
#include <assert.h>

namespace hps
{
namespace ntg
{

typedef int Weight;
// Default game parameters.
namespace detail
{

enum { Board_Size = 15, };
enum { Board_PivotL = -3, };
enum { Board_PivotR = -1, };
enum { Board_BoardWeight = 3, };
enum { Player_NumWeights = 10, };
const int Board_InitWeightPairs[][2] = { {-4, 3} };
enum { Board_InitWeights = sizeof(Board_InitWeightPairs) /
                           sizeof(Board_InitWeightPairs[0]), };
template <int BoardWeight_, int PivotL_, int PivotR_, int Size_>
struct GenericBoard;
template <int NumWeights_>
struct GenericPlayer;

} // end ns detail

typedef detail::GenericBoard<detail::Board_BoardWeight,
                             detail::Board_PivotL,
                             detail::Board_PivotR,
                             detail::Board_Size> Board;
typedef detail::GenericPlayer<detail::Player_NumWeights> Player;


namespace detail
{

/// <summary> A game board including played weights. </summary>
/// <remarks>
///   <para> The overloaded array operator is not zero-based. It is designed
///     for the layout of the game board such that Board[0] is the board
///     center, Board[n: n < 0] is the left side of the board and
///     Board[n: n > 0] is the right side of the board. For a board of size
///     N, the allowable indices are -N <= n <= N.
///   </para>
/// </remarks>
template <int BoardWeight_, int PivotL_, int PivotR_, int Size_>
struct GenericBoard
{
  enum { BoardWeight = BoardWeight_, };
  enum { PivotL = PivotL_, };
  enum { PivotR = PivotR_, };
  enum { Size = Size_, };
  enum { CenterOfGravity = 0, };
  enum { Positions = (2 * Size) + 1, };
  enum { Empty = 0, };

  typedef Weight value_type;
  typedef Weight& reference;
  typedef const Weight& const_reference;
  typedef Weight* iterator;
  typedef const Weight* const_iterator;

  inline void SetPos(const int pos, const Weight w)
  {
    assert(pos >= -Size);
    assert(pos <= Size);
    // rsofaer -- 20110929 -- Assert board not tipped.
    // reissb -- 20111002 -- This should go into an external function
    //   bool BoardTipped(const Board&).
    positions[pos + Size] = w;
  }
  inline Weight GetPos(const int pos) const
  {
    return positions[pos + Size];
  }

  inline reference operator[](const int pos)
  {
    return positions[pos + Size];
  }
  inline const_reference operator[](const int pos) const
  {
    return positions[pos + Size];
  }

  inline iterator begin()
  {
    return positions;
  }
  inline const_iterator begin() const
  {
    return positions;
  }

  inline iterator end()
  {
    return positions + Positions;
  }
  inline const_iterator end() const
  {
    return positions + Positions;
  }

  Weight positions[Positions];
};

/// <summary> Player consists of a hand of weights. </summary>
template <int NumWeights_>
struct GenericPlayer
{
  enum { Played = -1, };
  enum { NumWeights = NumWeights_, };
  Weight hand[NumWeights];
  int remain;
};

template <typename BoardType>
inline void ClearBoard(BoardType* board)
{
  assert(board);
  memset(board->positions, BoardType::Empty, sizeof(board->positions));
}

} // end ns detail

/// <summary> A game state. Consists of board and players. </summary>
struct State
{
  enum Turn
  {
    Turn_Red = 0,
    Turn_Blue,
  };

  enum Phase
  {
    Phase_Adding = 0,
    Phase_Removing,
  };

  enum { NumRemoved = (2 * Player::NumWeights) + detail::Board_InitWeights, };

  Board board;
  Player red;
  Player blue;
  Turn turn;
  Phase phase;
  Weight removed[NumRemoved];
};

/// <summary> Switch a turn. </summary>
inline void NextTurn(State::Turn* turn)
{
  assert(turn);
  switch (*turn)
  {
  case State::Turn_Red:
    *turn = State::Turn_Blue;
    break;
  case State::Turn_Blue:
    *turn = State::Turn_Red;
    break;
  }
}

/// <summary> Switch a phase. </summary>
inline void NextPhase(State::Phase* phase)
{
  assert(phase);
  switch (*phase)
  {
  case State::Phase_Adding:
    *phase = State::Phase_Removing;
    break;
  case State::Phase_Removing:
    *phase = State::Phase_Adding;
    break;
  }
}

/// <summary> Get the current player from the state. </summary>
inline Player* CurrentPlayer(State* state)
{
  assert(state);
  return (state->turn == State::Turn_Red) ? &state->red : &state->blue;
}
/// <summary> Get the current player from the state. </summary>
inline const Player* CurrentPlayer(const State* state)
{
  assert(state);
  return (state->turn == State::Turn_Red) ? &state->red : &state->blue;
}

/// <summary> Initialize player to game defaults. </summary>
void InitPlayer(Player* player)
{
  player->remain = Player::NumWeights;
  for (int wIdx = 0; wIdx < Player::NumWeights; ++wIdx)
  {
    player->hand[wIdx] = wIdx + 1;
  }
}

/// <summary> Clear the game board. </summary>
inline void ClearBoard(Board* board)
{
  detail::ClearBoard(board);
}

/// <summary> Initialize board to game defaults. </summary>
inline void InitBoard(Board* board)
{
  assert(board);
  ClearBoard(board);
  // Set the initial weight on the board.
  for (int pairIdx = 0; pairIdx < detail::Board_InitWeights; ++pairIdx)
  {
    const int* pair = detail::Board_InitWeightPairs[pairIdx];
    (*board)[pair[0]] = pair[1];
  }
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
  memset(state->removed, Board::Empty, sizeof(state->removed));
}

/// <summary> Unexplored move that will spawn a new state. </summary>
/// <remarks>
///   <para> The ply must be managed in conjunction with its associated state.
///     the state is not copied for performance reasons. Note that all plys
///     are easily reversible. This property is ideal for tree traversal.
///   </para>
///   <para> Plys during the removal phase have no associated weight index
///     since they are not placed back into the player hand.
///   </para>
/// </remarks>
struct Ply
{
  // Default constructor for stl containers.
  Ply()
    : pos(std::numeric_limits<int>::min()),
      wIdx(std::numeric_limits<int>::min())
  {}
  // Constructor for adding phase.
  Ply(const int pos_, const int wIdx_) : pos(pos_), wIdx(wIdx_) {}
  // Constructor for removing phase.
  Ply(const int pos_) : pos(pos_), wIdx(std::numeric_limits<int>::min()) {}
  int pos;
  int wIdx;
};

namespace detail
{

/// <summary> Compute torque at a pivot for a given board. </summary>
template <int Pivot, typename BoardType>
int Torque(const BoardType& board)
{
  int torque = (Pivot - BoardType::CenterOfGravity) * BoardType::BoardWeight;
  for (int pos = -BoardType::Size; pos <= BoardType::Size; ++pos)
  {
    torque += board[pos] * (Pivot - pos);
  }
  return torque;
}

template <typename BoardType>
inline bool Tipped(const BoardType& board)
{
  return (Torque<BoardType::PivotL>(board) > 0) ||
         (Torque<BoardType::PivotR>(board) < 0);
}

} // end ns detail

/// <summary> Compute torque around left pivot. </summary>
inline int TorqueL(const Board& board)
{
  return detail::Torque<Board::PivotL, Board>(board);
}

/// <summary> Compute torque around right pivot. </summary>
inline int TorqueR(const Board& board)
{
  return detail::Torque<Board::PivotR, Board>(board);
}

inline bool Tipped(const Board& board)
{
  return (TorqueL(board) > 0) || (TorqueR(board) < 0);
}

namespace detail
{

/// <summary> Find all plys from a given state. </summary>
/// <remarks>
///   <para> The supplied LeftPivotOp determines the plys that are kept when
///     testing against the left pivot. The right pivot uses the opposite
///     logic. Passing the correct operator will yield non-suicidal moves,
///     suicidal moves, or similar sets with different boundaries.
///   </para>
/// </remarks>
template <typename LeftPivotOp, typename RightPivotOp,
          typename CombineOp>
class ExpandState
{
public:
  static void Run(const State& state, std::vector<Ply>* plys)
  {
    assert(plys->empty());

    const Board& board = state.board;
    const int torqueL = TorqueL(board);
    const int torqueR = TorqueR(board);
    typedef std::binder2nd<LeftPivotOp> BoundLeftPivotOp;
    typedef std::binder2nd<RightPivotOp> BoundRightPivotOp;
    const BoundLeftPivotOp exclL = std::bind2nd(LeftPivotOp(), 0);
    const BoundRightPivotOp exclR = std::bind2nd(RightPivotOp(), 0);
    const CombineOp combineOp;
    // Removing phase just uses whatever is on the board.
    if (State::Phase_Removing == state.phase)
    {
      int pos = -Board::Size;
      const Weight* w = board.begin();
      int leverL = pos - Board::PivotL;
      int leverR = Board::PivotR - pos;
      for (; pos < Board::Size; ++pos, ++w, --leverL, --leverR)
      {
        // Weight is placed?
        if (Board::Empty != *w)
        {
          // Torque due to this weight.
          const int weightTorqueL = leverL * (*w);
          const int weightTorqueR = leverR * (*w);
          const bool leftIncl = !exclL(torqueL - weightTorqueL);
          const bool rightIncl = !exclR(torqueR - weightTorqueR);
          // Keep this ply?
          if (combineOp(leftIncl, rightIncl))
          {
            plys->push_back(Ply(pos));
          }
        }
      }
    }
    else
    {
      const Player* currentPlayer = CurrentPlayer(&state);
      int wIdx = 0;
      for (const int* w = currentPlayer->hand;
           w < (currentPlayer->hand + Player::NumWeights);
           ++w, ++wIdx)
      {
        // Is weight already played?
        if (Player::Played == *w)
        {
          continue;
        }
        int pos = -Board::Size;
        const Weight* boardVal = board.begin();
        int leverL = Board::PivotL - pos;
        int leverR = Board::PivotR - pos;
        for (; pos <= Board::Size; ++pos, ++boardVal, --leverL, --leverR)
        {
          if (Board::Empty == *boardVal)
          {
            const int weightTorqueL = leverL * (*w);
            const int weightTorqueR = leverR * (*w);
            const bool leftIncl = !exclL(torqueL + weightTorqueL);
            const bool rightIncl = !exclR(torqueR + weightTorqueR);
            if (combineOp(leftIncl, rightIncl))
            {
              plys->push_back(Ply(pos, wIdx));
            }
          }
        }
      }
    }
  }
};

template <typename PhaseOp>
inline void PlyMutateState(const Ply& ply, State* state)
{
  Player* player = CurrentPlayer(state);
  Weight* hand = player->hand;
  assert(state);
  const bool adding = (State::Phase_Adding == state->phase);
  if (PhaseOp()(State::Phase_Adding, state->phase))
  {
    assert(Board::Empty == state->board[ply.pos]);
    if (adding)
    {
      // Update the board.
      state->board[ply.pos] = hand[ply.wIdx];
      // Update hand.
      hand[ply.wIdx] = Player::Played;
      --player->remain;
    }
    else
    {
      ++player->remain;
      const int removeIdx = abs(state->red.remain + state->blue.remain);
      assert(removeIdx >= 0);
      assert(removeIdx < State::NumRemoved);
      // Update the board.
      state->board[ply.pos] = state->removed[removeIdx];
      // Update removed;
      state->removed[removeIdx] = Board::Empty;
    }
  }
  else
  {
    assert(Board::Empty != state->board[ply.pos]);
    // Update hand.
    if (adding)
    {
      hand[ply.wIdx] = state->board[ply.pos];
      ++player->remain;
    }
    else
    {
      const int removeIdx = abs(state->red.remain + state->blue.remain);
      assert(removeIdx >= 0);
      assert(removeIdx < State::NumRemoved);
      state->removed[removeIdx] = state->board[ply.pos];
      --player->remain;
    }
    // Update the board.
    state->board[ply.pos] = Board::Empty;
  }
}

} // end ns detail

/// <summary> Discover all non suicidal plys from a given state. </summary>
inline void PossiblePlys(const State& state, std::vector<Ply>* plys)
{
  detail::ExpandState<std::greater<Weight>,
                      std::less<Weight>,
                      std::logical_and<Weight> >::Run(state, plys);
}

/// <summary> Discover all suicidal plys from a given state. </summary>
inline void SuicidalPlys(const State& state, std::vector<Ply>* plys)
{
  // TODO(reissb) -- 20111004 -- Suicidal plys not computed properly
  //   when removing. Don't need them for anything right now.
  assert(State::Phase_Removing != state.phase);
  detail::ExpandState<std::less_equal<Weight>,
                      std::greater_equal<Weight>,
                      std::logical_or<Weight> >::Run(state, plys);
}

/// <summary> Apply the ply and get the resulting state. </summary>
/// <remarks>
///   <para> This function will mutate the incoming state. Its actions may
///     be undone easily with UndoPly(const Ply&, State*).
///   </para>
/// </remarks>
inline void DoPly(const Ply& ply, State* state)
{
  detail::PlyMutateState<std::equal_to<State::Phase> >(ply, state);

  // Swap the active player.
  NextTurn(&state->turn);
  // If both hands are empty, switch the phase.
  {
    const bool redEmpty = (0 == state->red.remain);
    const bool blueEmpty = (0 == state->blue.remain);
    if (redEmpty && blueEmpty)
    {
      NextPhase(&state->phase);
    }
  }
}

/// <summary> Revserse the ply and get the resulting state. </summary>
///   <para> This function will mutate the incoming state. Its actions may
///     be undone easily with DoPly(const Ply&, State*).
///   </para>
inline void UndoPly(const Ply& ply, State* state)
{
  // If both hands are empty, switch the phase.
  {
    const bool redEmpty = (0 == state->red.remain);
    const bool blueEmpty = (0 == state->blue.remain);
    if (redEmpty && blueEmpty)
    {
      NextPhase(&state->phase);
    }
  }
  // Swap the active player.
  NextTurn(&state->turn);

  detail::PlyMutateState<std::not_equal_to<State::Phase> >(ply, state);
}

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NO_TIPPING_GAME_H_
