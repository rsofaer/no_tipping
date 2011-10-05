#include "no_tipping_game.h"
#include "adversarial_utils.h"
#include "minimax.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace hps;

void BuildState(State* stateBuffer)
{
  //std::cout << "Building state\n";
  std::string curLine;
  int redWeightsRemaining=0;
  int blueWeightsRemaining=0;
  getline(std::cin, curLine);

  //std::cout << curLine << "\n";

  if(curLine == "ADDING")
  {
    stateBuffer->phase = State::Phase_Adding;
  } 
  else 
  {
    stateBuffer->phase = State::Phase_Removing;
  }

  while(getline(std::cin, curLine))
  {
    //std::cout << curLine << "\n";
    if(curLine == "STATE END")
    {
      return;
    }

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

std::string CalculateMoveWrapper()
{
  State stateBuffer; // get Statebuffer's previous states from a method that has saved it.
  //CalculateMove gets called on every move of the opponent, maintain a state somewhere and get it back.
  BuildState(&stateBuffer);
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

int main(int argc, char** argv)
{
  // Get the string for the state.
  std::cout << CalculateMoveWrapper() << "\n";

  return 0;
}
