#include <cstddef>
#include <string>
#include <vector>
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
    Red,
    Blue,
  };

  const Player* GetCurrentPlayer() const
  {
    return (turn == Red) ? &red : &blue;
  }

  Board board;
  Player red;
  Player blue;
  Turn turn;
};

/// <summary> Initialize player to game defaults. </summary>
void InitPlayer(Player* player)
{
  static const Weight s_initW[Player::NumWeights] = { 1, 2, 3, 4, 5, 6, 7 };
  player->remain = Player::NumWeights;
  memcpy(player->hand, s_initW, sizeof(s_initW));
}

/// <summary> Initialize state to game defaults. </summary>
void InitState(State* state)
{
  state->turn = State::Red;
  InitPlayer(&state->red);
  InitPlayer(&state->blue);
  memset(state->board.positions, Board::Empty, sizeof(state->board));
  // Set the initial weight on the board.
  state->board[-4] = 3;
}

/// <summary> Unexplored move that will spawn a new state. </summary>
struct PossibleMove
{
  PossibleMove(const State& state_, const int target_, const Weight weight_)
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
}

/// <summary> Compute torque around left pivot. </summary>
inline int TorqueL(const Board& board)
{
  return detail::Torque<Board::PivotL>(board);
}

/// <summary> Compute torque around right pivot. </summary>
int TorqueR(const Board& board)
{
  return detail::Torque<Board::PivotR>(board);
}

/// <summary> Discover all non suicidal moves from a given state. </summary>
void PossibleMoves(const State& state, std::vector<PossibleMove>* possibleMoves)
{
  assert(possibleMoves->empty());

  const Board& board = state.board;
  const int torqueL = TorqueL(board);
  const int torqueR = TorqueR(board);
  const Player* currentPlayer = state.GetCurrentPlayer();
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
    for (int pos = startPos; pos < Board::PivotL; ++pos)
    {
      if (board[pos] != Board::Empty)
      {
        continue;
      }
      const int weightTorque = (Board::PivotL - pos) * (*w);
      if ((weightTorque + torqueL) > 0)
      {
        continue;
      }
      else
      {
        possibleMoves->push_back(PossibleMove(state, pos, *w));
      }
    }
    // Compute between pivots.
    for (int pos = Board::PivotL; pos <= Board::PivotR; ++pos)
    {
      if (board[pos] != Board::Empty)
      {
        continue;
      }
      possibleMoves->push_back(PossibleMove(state, pos, *w));
    }
    // Compute for right pivot.
    for (int pos = Board::PivotR + 1; pos <= Board::Size; ++pos)
    {
      if (board[pos] != Board::Empty)
      {
        continue;
      }
      const int weightTorque = (Board::PivotR - pos) * (*w);
      if ((weightTorque + torqueR) < 0)
      {
        continue;
      }
      else
      {
        possibleMoves->push_back(PossibleMove(state, pos, *w));
      }
    }
  }
}

int main(int /*argc*/, char** /*argv*/)
{
  State state;
  InitState(&state);
  std::vector<PossibleMove> moves;
  PossibleMoves(state, &moves);
  return 0;
}
