#include "contestant_util.h"
#include "ntg.h"
#include "ntg_players.h"
#include <string>
#include <sstream>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <time.h>
#endif

namespace hps
{
namespace ntg
{

bool BuildState(std::istream& input, State* stateBuffer)
{
  assert(stateBuffer);
  enum { MaxEmptyLines = 10, };

  //std::cout << "Building state\n";
  int redWeightsRemaining=0;
  int blueWeightsRemaining=0;
  int weightsOnBoard = 0;
  InitState(stateBuffer);
  ClearBoard(&stateBuffer->board);

  //Get first line for phase.
  std::string curLine;
  // Read until input pipe closes or until there is a line.
  do
  {
    const bool lineGood = ReadMaxEmptyLines(input, MaxEmptyLines, &curLine);
    if (!lineGood)
    {
#ifdef WIN32
      Sleep(20);
#else
      timespec t;
      {
        t.tv_sec = 0;
        t.tv_nsec = 20 * 1000 * 1000;
      }
      nanosleep(&t, NULL);
#endif
    }
    if (input.bad())
    {
      return false;
    }
    input.clear();
  } while(curLine.empty());
  //std::cout << curLine << "\n";
  if( "ADDING" == curLine)
  {
    stateBuffer->phase = State::Phase_Adding;
  } 
  else 
  {
    assert("REMOVING" == curLine);
    stateBuffer->phase = State::Phase_Removing;
  }
  //Read lines until STATE END.
  for (;;)
  {
    // Read until input pipe closes or until there is a line.
    do
    {
      const bool lineGood = ReadMaxEmptyLines(input, MaxEmptyLines, &curLine);
      if (!lineGood)
      {
#ifdef WIN32
        Sleep(20);
#else
        timespec t;
        {
          t.tv_sec = 0;
          t.tv_nsec = 20 * 1000 * 1000;
        }
        nanosleep(&t, NULL);
#endif
      }
      if (input.bad())
      {
        return false;
      }
      input.clear();
    } while(curLine.empty());
    //std::cout << curLine << "\n";
    if("STATE END" == curLine)
    {
      break;
    }
    //Parse line.
    std::stringstream sstream(curLine);
    int onBoard;
    int position;
    std::string color;
    int weight;
    sstream >> onBoard >> position >> color >> weight;
    //std::cout << onBoard << " " << position << " " << color << " " << weight << "\n";
    //Get weight status.
    if(1 == onBoard)
    {
      ++weightsOnBoard;
      stateBuffer->board.SetPos(position, weight);
      //std::cout << "Piece is on the board.\n";
      if("Red" == color)
      {
        //std::cout << "Piece is red\n";
        stateBuffer->red.hand[weight-1] = Player::Played;
      }
      else if("Blue" == color)
      {
        //std::cout << "Piece is blue\n";
        stateBuffer->blue.hand[weight-1] = Player::Played;
      }
      else if("Green" != color)
      {
        return false;
      }
    } 
    else
    {
      //std::cout << "Piece is not on the board.\n";
      if("Red" == color)
      {
        //std::cout << "Piece is red\n";
        stateBuffer->red.hand[weight-1] = weight;
        ++redWeightsRemaining;
        //std::cout << "hand is " << stateBuffer->red.hand[weight-1] << "at index " << weight-1 << "\n";
      }
      else if("Blue" == color)
      {
        //std::cout << "Piece is blue\n";
        stateBuffer->blue.hand[weight-1] = weight;
        ++blueWeightsRemaining;
      }
      else if("Green" != color)
      {
        return false;
      }
    }
  }

  //std::cout << "Red weights remaining: " << redWeightsRemaining << "\n";
  //std::cout << "Blue weights remaining: " << blueWeightsRemaining << "\n";
  if(stateBuffer->phase == State::Phase_Adding)
  {
    assert(blueWeightsRemaining >= redWeightsRemaining);
    //Red's turn?
    if(redWeightsRemaining == blueWeightsRemaining)
    {
      //std::cout << "Red's turn.\n";
      stateBuffer->turn = State::Turn_Red;
    }
    // Blue's turn
    else
    {
      //std::cout << "Blue's turn.\n";
      stateBuffer->turn = State::Turn_Blue;
    }
  }
  else
  {
    //Phase is removing.
    if(weightsOnBoard % 2 == 1)
    {
      stateBuffer->turn = State::Turn_Red;
    }
    else
    {
      stateBuffer->turn = State::Turn_Blue;
    }
  }
  stateBuffer->red.remain = redWeightsRemaining;
  stateBuffer->blue.remain = blueWeightsRemaining;
  return true;
}

void PrintState(State &state)
{
  std::cout << "Red has " << state.red.remain << " weights remaining: ";
  for(int i = 0; i < 10; i++)
  {
    std::cout << state.red.hand[i] << " ";
  }
  std::cout << std::endl;
  
  std::cout << "Blue has " << state.blue.remain << " weights remaining: ";
  for(int j = 0; j < 10; j++)
  {
    std::cout << state.blue.hand[j] << " ";
  }
  std::cout << std::endl;
  
  std::cout << "The board: ";
  for(int k = -15; k < 16; k++){
    std::cout << state.board.GetPos(k) << " ";
  }
  std::cout << std::endl
            << "Phase is adding?: " << (state.phase == State::Phase_Adding)
            << std::endl
            << "Turn is red?: " << (state.turn == State::Turn_Red)
            << std::endl;
}

std::string CalculateMoveWrapper(State* stateBuffer)
{
  assert(stateBuffer);

  // Make a move.
  AlphaBetaPruningPlayer player(stateBuffer->turn);
  Ply ply;
  player.NextPly(stateBuffer, &ply);
  // Determing affected weight.
  int weight;
  if (stateBuffer->phase == State::Phase_Adding)
  {
    weight = CurrentPlayer(stateBuffer)->hand[ply.wIdx];
  }
  else
  {
    weight = stateBuffer->board.GetPos(ply.pos);
  }
  // Build move string.
  std::stringstream ss;
  ss << ply.pos << " " << weight;
  return ss.str();
}

}
}
