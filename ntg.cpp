#include "no_tipping_game.h"
#include "adversarial_utils.h"
#include "minimax.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace hps;

void BuildState(const std::string& command, State* stateBuffer)
{
  std::cout << "Building state\n";
  std::string curLine;
  std::stringstream ss(command);
  int redWeightsRemaining=0;
  int blueWeightsRemaining=0;
  getline(ss, curLine);

  std::cout << curLine << "\n";

  if(curLine == "ADDING")
  {
    stateBuffer->phase = State::Phase_Adding;
  } 
  else 
  {
    stateBuffer->phase = State::Phase_Removing;
  }

  while(getline(ss, curLine))
  {
    std::cout << curLine << "\n";
    std::stringstream sstream(curLine);
    int onBoard;
    int position;
    std::string color;
    int weight;

    sstream >> onBoard >> position >> color >> weight;

    if(onBoard == 1)
    {
      stateBuffer->board.SetPos(position, weight);
      if(color == "Red")
      {
        stateBuffer->red.hand[weight-1] = Player::Played;
      }else if(color == "Blue")
      {
        stateBuffer->blue.hand[weight-1] = Player::Played;
      }
      else
      {
        // it is the green block.
        return;
      }
    } 
    else
    {
      if(color == "Red")
      {
        stateBuffer->red.hand[weight-1] = weight;
      } else if(color == "Blue")
      {
        stateBuffer->blue.hand[weight-1] = weight;
      } else 
      { 
        // 
        assert(false); 
      }
    }
  }
  if(redWeightsRemaining == blueWeightsRemaining){
    //Red's turn
    stateBuffer->turn = State::Turn_Red;
  } else if(blueWeightsRemaining > redWeightsRemaining)
  {
    //Blue's turn
    stateBuffer->turn = State::Turn_Blue;
  } else { assert(false); }
}

std::string CalculateMoveWrapper(const std::string& command)
{
  State stateBuffer; // get Statebuffer's previous states from a method that has saved it.
  //CalculateMove gets called on every move of the opponent, maintain a state somewhere and get it back.
  BuildState(command, &stateBuffer);
  Minimax::Params params;
  {
    params.maxDepthAdding = 3;
    params.maxDepthRemoving = 5;
  }

  BoardEvaluationInverseDepthWinStates evalFunc(stateBuffer.turn);
  Ply ply;
  Minimax::Run(&params, &stateBuffer, &evalFunc, &ply);
  int position = ply.pos;
  int weight = CurrentPlayer(&stateBuffer)->hand[ply.wIdx];
  std::stringstream ss;
  ss << position << " " << weight;
  return ss.str();
}

/// <summary> Print the application help message to a stream. </summary>
void PrintHelpMsg(std::ostream& out, const char* applicationName)
{
  out << "Usage: " << applicationName << " STATE_STRING" << std::endl;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    const std::string appNameLong = argv[0];
    const std::string appName =
#ifdef WIN32
      appNameLong.substr(appNameLong.rfind('\\') + 1,
                         std::string::npos);
#else
      appNameLong.substr(appNameLong.rfind('/') + 1,
                         std::string::npos);
#endif
    PrintHelpMsg(std::cerr, appName.c_str());
    return 1;
  }

  // Get the string for the state.
  const std::string strCommand(argv[1]);
  std::cout << CalculateMoveWrapper(strCommand);;

  return 0;
}
