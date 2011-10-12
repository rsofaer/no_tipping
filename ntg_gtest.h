#ifndef _NO_TIPPING_GAME_NTG_GTEST_H_
#define _NO_TIPPING_GAME_NTG_GTEST_H_
#include "no_tipping_game.h"
#include "ntg_gtest_operators.h"
#include "ntg_gtest_utils.h"
#include "rand_bound_generator.h"
#include "gtest/gtest.h"

namespace _no_tipping_game_ntg_gtest_h_
{
using namespace hps;

TEST(Board, NoTippingComponents)
{
  // Board iterators.
  {
    SCOPED_TRACE("Iterators");
    Board board;
    const Board& boardConst = board;
    ASSERT_EQ(board.begin(), boardConst.begin());
    ASSERT_EQ(board.end(), boardConst.end());
  }
  // Test board indices.
  {
    SCOPED_TRACE("Accessors and mutators");
    enum { BoardTestSize = 50, };
    typedef detail::GenericBoard<0, 0, 0, BoardTestSize> TestBoard;
    TestBoard board;
    const size_t boardPositions = sizeof(board.positions) /
                                  sizeof(board.positions[0]);
    ASSERT_EQ(TestBoard::Positions, boardPositions);
    ASSERT_EQ((2 * BoardTestSize) + 1, boardPositions);
    // Fill board with numbers [-pos, pos].
    {
      const TestBoard& boardConst = board;
      Weight w = -BoardTestSize;
      for (; w <= BoardTestSize; ++w)
      {
        board[w] = w;
        EXPECT_EQ(w, board.positions[w + BoardTestSize]);
        EXPECT_EQ(w, board.GetPos(w));
        EXPECT_EQ(w, board[w]);
        EXPECT_EQ(w, boardConst.positions[w + BoardTestSize]);
        EXPECT_EQ(w, boardConst.GetPos(w));
        EXPECT_EQ(w, boardConst[w]);
        board[w] = Board::Empty;
        board.SetPos(w, w);
        EXPECT_EQ(w, board.positions[w + BoardTestSize]);
        EXPECT_EQ(w, board.GetPos(w));
        EXPECT_EQ(w, board[w]);
        EXPECT_EQ(w, boardConst.positions[w + BoardTestSize]);
        EXPECT_EQ(w, boardConst.GetPos(w));
        EXPECT_EQ(w, boardConst[w]);
      }
    }
  }
  // Clear the board.
  {
    SCOPED_TRACE("Clear");
    Board board;
    ClearBoard(&board);
    const Weight empty = Board::Empty;
    for (Board::const_iterator w = board.begin();
         w != board.end();
         ++w)
    {
      EXPECT_EQ(empty, *w);
    }
  }
  // Bare board should tip. Initial board should not tip.
  {
    SCOPED_TRACE("Tipped");
    Board board;
    ClearBoard(&board);
    EXPECT_TRUE(Tipped(board));
    InitBoard(&board);
    EXPECT_FALSE(Tipped(board));
  }
}

TEST(Player, NoTippingComponents)
{
  // Default player.
  {
    SCOPED_TRACE("InitPlayer");
    Player player;
    InitPlayer(&player);
    const size_t weightCount = sizeof(player.hand) / sizeof(player.hand[0]);
    EXPECT_EQ(weightCount, player.remain);
    Weight w = 1;
    for (size_t handIdx = 0; handIdx < weightCount; ++handIdx, ++w)
    {
      EXPECT_EQ(w, player.hand[handIdx]);
    }
  }
}

TEST(State, NoTippingComponents)
{
  // Default state.
  std::cout << "State::MaxPlys " << State::MaxPlys << "." << std::endl;
  std::cout << "State::NumRemoved " << State::NumRemoved << "." << std::endl;
  {
    SCOPED_TRACE("InitState");
    State state;
    InitState(&state);
    {
      Board board;
      InitBoard(&board);
      EXPECT_EQ(board, state.board);
    }
    {
      Player player;
      InitPlayer(&player);
      EXPECT_EQ(player, state.red);
      EXPECT_EQ(player, state.blue);
    }
    EXPECT_EQ(State::Turn_Red, state.turn);
    EXPECT_EQ(&state.red, CurrentPlayer(&state));
    EXPECT_EQ(State::Phase_Adding, state.phase);
  }
  // State take turns.
  {
    SCOPED_TRACE("State Phase_Adding -> DoPly(), UndoPly()");
    State state;
    InitState(&state);
    const Ply ply(-Board::Size, 0);
    const Player* player = CurrentPlayer(&state);
    const Player redPlayerInit = state.red;
    const Player bluePlayerInit = state.blue;
    const State initState = state;
    DoPly(ply, &state);
    {
      EXPECT_EQ(redPlayerInit.remain - 1, player->remain);
      EXPECT_EQ(Player::Played, player->hand[ply.wIdx]);
      EXPECT_EQ(redPlayerInit.hand[ply.wIdx], state.board[ply.pos]);
      EXPECT_EQ(State::Turn_Blue, state.turn);
      EXPECT_EQ(&state.blue, CurrentPlayer(&state));
      EXPECT_EQ(State::Phase_Adding, state.phase);
      EXPECT_NE(redPlayerInit, state.red);
      EXPECT_EQ(bluePlayerInit, state.blue);
    }
    UndoPly(ply, &state);
    {
      EXPECT_EQ(initState, state);
    }
  }
  // State full phases (almost a full game but no board checks).
  {
    SCOPED_TRACE("State both phases reversible");
    RandomPlayOrderGenerator<Player::NumWeights> playOrderGen;
    const std::vector<int> redPlayOrder = playOrderGen();
    const std::vector<int> bluePlayOrder = playOrderGen();
    State state;
    InitState(&state);
    std::vector<State> stateStack;
    stateStack.push_back(state);
    // Generate random order for add positions.
    std::vector<int> plyPosOrder;
    {
      const Board& board = state.board;
      for (int pos = -Board::Size; pos <= Board::Size; ++pos)
      {
        if (Board::Empty == board[pos])
        {
          plyPosOrder.push_back(pos);
        }
      }
      std::random_shuffle(plyPosOrder.begin(), plyPosOrder.end());
    }
    std::vector<Ply> plys;
    {
      // Find max number of plys to make.
      const size_t maxPly = std::min(redPlayOrder.size() + bluePlayOrder.size(),
                                     plyPosOrder.size());
      plys.reserve(maxPly);
      for (size_t plyIdx = 0; plyIdx < maxPly; ++plyIdx)
      {
        int wIdx;
        if (plyIdx & 1)
        {
          wIdx = bluePlayOrder[plyIdx / 2];
        }
        else
        {
          wIdx = redPlayOrder[(plyIdx + 1) / 2];
        }
        plys.push_back(Ply(plyPosOrder[plyIdx], wIdx));
      }
    }
    // Do all of the plys.
    for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
    {
      DoPly(plys[plyIdx], &state);
      stateStack.push_back(state);
    }
    // Hands empty.
    EXPECT_TRUE(PlayerEmptyHand(state.red));
    EXPECT_EQ(0, state.red.remain);
    EXPECT_TRUE(PlayerEmptyHand(state.blue));
    EXPECT_EQ(0, state.blue.remain);
    // Phase should switch to removing.
    EXPECT_EQ(State::Phase_Removing, state.phase);
    // Make removal plys.
    const size_t addingPlys = plys.size();
    {
      std::vector<int> removePhaseOrder;
      for (int pos = -Board::Size; pos <= Board::Size; ++pos)
      {
        if (Board::Empty != state.board[pos])
        {
          removePhaseOrder.push_back(pos);
        }
      }
      std::random_shuffle(removePhaseOrder.begin(), removePhaseOrder.end());
      for (size_t plyIdx = 0; plyIdx < removePhaseOrder.size(); ++plyIdx)
      {
        plys.push_back(Ply(removePhaseOrder[plyIdx]));
      }
    }
    const size_t removingPlys = plys.size() - addingPlys;
    // Keep going until the board is empty.
    for (size_t plyIdx = addingPlys; plyIdx < plys.size(); ++plyIdx)
    {
      DoPly(plys[plyIdx], &state);
      stateStack.push_back(state);
    }
    // Red hand empty.
    {
      EXPECT_TRUE(PlayerEmptyHand(state.red));
      const int redRemovals = (removingPlys + 1) / 2;
      EXPECT_EQ(-redRemovals, state.red.remain);
    }
    // Blue hand empty.
    {
      EXPECT_TRUE(PlayerEmptyHand(state.blue));
      const int blueRemovals = removingPlys / 2;
      EXPECT_EQ(-blueRemovals, state.blue.remain);
    }
    // Reversible.
    for (size_t plyIdx = plys.size(); plyIdx > 0; --plyIdx)
    {
      EXPECT_EQ(stateStack[plyIdx], state);
      UndoPly(plys[plyIdx - 1], &state);
      DoPly(plys[plyIdx - 1], &state);
      EXPECT_EQ(stateStack[plyIdx], state);
      UndoPly(plys[plyIdx - 1], &state);
      EXPECT_EQ(stateStack[plyIdx - 1], state);
      EXPECT_NE(stateStack[plyIdx], state);
    }
  }
}

TEST(PlysExhaustive, NoTippingComponents)
{
  {
    State state;
    InitState(&state);
    std::vector<Ply> plys;
    bool moreMoves = false;
    int turns = 0;
    do
    {
      plys.clear();
      PossiblePlys(state, &plys);
      for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
      {
        DoPly(plys[plyIdx], &state);
        EXPECT_FALSE(Tipped(state.board));
        UndoPly(plys[plyIdx], &state);
      }
      moreMoves = !plys.empty();
      if (moreMoves)
      {
        ++turns;
        int randomPly = RandBound(plys.size());
        DoPly(plys[randomPly], &state);
      }
    } while (moreMoves);
  }
}

TEST(Torque, NoTippingComponents)
{
  // Default board.
  {
    SCOPED_TRACE("Default board torque");
    Board board;
    InitBoard(&board);
    int torqueL;
    int torqueR;
    Torques(board, &torqueL, &torqueR);
    EXPECT_EQ(TorqueL(board), torqueL);
    EXPECT_EQ(TorqueR(board), torqueR);
    EXPECT_EQ(torqueL, -6);
    EXPECT_EQ(torqueR, 6);
  }
  // Some random board torques.
  {
    enum { BoardTestSize = 200, };
    enum { TestMaxWeight = 200, };
    typedef detail::GenericBoard<0, 0, 0, BoardTestSize> BoardTest;
    const RandBoundedGenerator randWeightGen(TestMaxWeight);
    const RandBoundedGenerator randPosGen(BoardTestSize);
    for (size_t trial = 0; trial < 1000; ++trial)
    {
      const Weight randW = randWeightGen() + 1;
      BoardTest board;
      assert(0 == BoardTest::PivotL);
      assert(0 == BoardTest::PivotR);
      detail::ClearBoard(&board);
      const int pos = randPosGen() - BoardTest::Size;
      // Place the weight.
      board[pos] = randW;
      const int torque = detail::Torque<0>(board);
      EXPECT_EQ(-pos * randW, torque);
      if (0 != pos)
      {
        const bool tipped = detail::Tipped(board);
        EXPECT_TRUE(tipped);
      }
    }
  }
}

TEST(AddingPhasePlys, NoTippingComponents)
{
  // Verify plys.
  {
    SCOPED_TRACE("PossiblePlys() adding");
    for (int trial = 0; trial < 100; ++trial)
    {
      State state;
      InitState(&state);
      EXPECT_FALSE(Tipped(state.board));
      std::vector<Ply> plys;
      do 
      {
        // Suicidal.
        plys.clear();
        SuicidalPlys(state, &plys);
        {
          for (size_t plyIdx = 0; plyIdx < plys.size(); ++plyIdx)
          {
            const State stateBefore = state;
            DoPly(plys[plyIdx], &state);
            EXPECT_TRUE(Tipped(state.board));
            UndoPly(plys[plyIdx], &state);
            EXPECT_EQ(stateBefore, state);
          }
        }
        // Non-suicidal.
        plys.clear();
        PossiblePlys(state, &plys);
        {
          const size_t plysFound = plys.size();
          if (plysFound > 0)
          {
            for (size_t plyIdx = 0; plyIdx < plysFound; ++plyIdx)
            {
              const State stateBefore = state;
              DoPly(plys[plyIdx], &state);
              EXPECT_FALSE(Tipped(state.board));
              UndoPly(plys[plyIdx], &state);
              EXPECT_EQ(stateBefore, state);
            }
            int doIdx = RandBound(plysFound);
            DoPly(plys[doIdx], &state);
          }
        }
      } while ((State::Phase_Adding == state.phase) && (!plys.empty()));
    }
  }
}

TEST(RemovingPhasePlys, NoTippingComponents)
{
  // Verify plys.
  {
    SCOPED_TRACE("PossiblePlys() removing");
    for (int trial = 0; trial < 1000; ++trial)
    {
      State state;
      RandomRemovingPhase(&state);
      EXPECT_FALSE(Tipped(state.board));
      std::vector<Ply> plys;
      do 
      {
        // Non-suicidal.
        plys.clear();
        PossiblePlys(state, &plys);
        {
          const size_t plysFound = plys.size();
          if (plysFound > 0)
          {
            for (size_t plyIdx = 0; plyIdx < plysFound; ++plyIdx)
            {
              const State stateBefore = state;
              DoPly(plys[plyIdx], &state);
              EXPECT_FALSE(Tipped(state.board));
              UndoPly(plys[plyIdx], &state);
              EXPECT_EQ(stateBefore, state);
            }
            int doIdx = RandBound(plysFound);
            DoPly(plys[doIdx], &state);
          }
        }
      } while (!plys.empty());
    }
  }
}

}

#endif //_NO_TIPPING_GAME_NTG_GTEST_H_
