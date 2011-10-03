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
enum { Player_NumWeights = 7, };
template <int BoardWeight_, int PivotL_, int PivotR_, int Size_>
struct GenericBoard;
template <int NumWeights_>
struct GenericPlayer;
}
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

}

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
  };

  Board board;
  Player red;
  Player blue;
  Turn turn;
  Phase phase;
};

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
  static const Weight s_initW[Player::NumWeights] = { 1, 2, 3, 4, 5, 6, 7 };
  player->remain = Player::NumWeights;
  memcpy(player->hand, s_initW, sizeof(s_initW));
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
  Ply()
    : pos(std::numeric_limits<int>::min()),
      wIdx(std::numeric_limits<Weight>::min())
  {}
  Ply(const int pos_, const int wIdx_) : pos(pos_), wIdx(wIdx_) {}
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
  const Player* currentPlayer = CurrentPlayer(&state);
  int startPos = -Board::Size;
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
          plys->push_back(Ply(pos, wIdx));
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
      plys->push_back(Ply(pos, wIdx));
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
          plys->push_back(Ply(pos, wIdx));
        }
      }
    }
  }
}

template <typename PhaseOp>
inline void PlyMutateState(const Ply& ply, State* state)
{
  Player* player = CurrentPlayer(state);
  Weight* hand = player->hand;
  assert(state);
  if (PhaseOp()(State::Phase_Adding, state->phase))
  {
    assert(Board::Empty == state->board[ply.pos]);
    assert(Player::Played != hand[ply.wIdx]);
    // Update the board.
    state->board[ply.pos] = hand[ply.wIdx];
    hand[ply.wIdx] = Player::Played;
    --player->remain;
  }
  else
  {
    assert(Board::Empty != state->board[ply.pos]);
    assert(Player::Played == hand[ply.wIdx]);
    // Update the board.
    hand[ply.wIdx] = state->board[ply.pos];
    state->board[ply.pos] = Board::Empty;
    ++player->remain;
  }
}

template <typename BoardType>
inline bool Tipped(const BoardType& board)
{
  return (Torque<BoardType::PivotL>(board) > 0) ||
         (Torque<BoardType::PivotR>(board) < 0);
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

inline bool Tipped(const Board& board)
{
  return (TorqueL(board) > 0) || (TorqueR(board) < 0);
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
  detail::PlyMutateState<std::equal_to<State::Phase> >(ply, state);
  // Swap the active player.
  {
    State::Turn& turn = state->turn;
    ++reinterpret_cast<int&>(turn);
    reinterpret_cast<int&>(turn) %= State::Turn_Count;
  }
}

/// <summary> Revserse the ply and get the resulting state. </summary>
///   <para> This function will mutate the incoming state. Its actions may
///     be undone easily with DoPly(const Ply&, State*).
///   </para>
inline void UndoPly(const Ply& ply, State* state)
{
  // Swap the active player.
  {
    State::Turn& turn = state->turn;
    ++reinterpret_cast<int&>(turn);
    reinterpret_cast<int&>(turn) %= State::Turn_Count;
  }
  detail::PlyMutateState<std::not_equal_to<State::Phase> >(ply, state);
}

}
using namespace ntg;
}

#endif //_NO_TIPPING_GAME_NO_TIPPING_GAME_H_
