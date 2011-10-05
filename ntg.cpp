#include "no_tipping_game.h"
#include "adversarial_utils.h"
#include "minimax.h"
#include <string>
#include <sstream>
#include <iostream>
#include <istream>

using namespace hps;

void BuildState(std::istream &input, State* stateBuffer)
{
  //std::cout << "Building state\n";
  std::string curLine;
  int redWeightsRemaining=0;
  int blueWeightsRemaining=0;
  getline(input, curLine);

  //std::cout << curLine << "\n";

  if(curLine == "ADDING")
  {
    stateBuffer->phase = State::Phase_Adding;
  } 
  else 
  {
    stateBuffer->phase = State::Phase_Removing;
  }

  while(getline(input, curLine))
  {
    //std::cout << curLine << "\n";
    if(curLine == "STATE END")
    {
      break;
    }

    std::stringstream sstream(curLine);
    int onBoard;
    int position;
    std::string color;
    int weight;

    sstream >> onBoard >> position >> color >> weight;
    
    //std::cout << onBoard << " " << position << " " << color << " " << weight << "\n";

    ClearBoard(&(stateBuffer->board));
    
    if(onBoard == 1)
    {
      //std::cout << "Piece is on the board.\n";
      stateBuffer->board.SetPos(position, weight);
      if(color == "Red")
      {
        //std::cout << "Piece is red\n";
        stateBuffer->red.hand[weight-1] = Player::Played;
        redWeightsRemaining++;
      }else if(color == "Blue")
      {
        //std::cout << "Piece is blue\n";
        stateBuffer->blue.hand[weight-1] = Player::Played;
        blueWeightsRemaining++;
      }
      else
      {
        // it is the green block.
        break;
      }
    } 
    else
    {
      //std::cout << "Piece is not on the board.\n";
      if(color == "Red")
      {
        //std::cout << "Piece is red\n";
        stateBuffer->red.hand[weight-1] = weight;
        //std::cout << "hand is " << stateBuffer->red.hand[weight-1] << "at index " << weight-1 << "\n";
      } else if(color == "Blue")
      {
        //std::cout << "Piece is blue\n";
        stateBuffer->blue.hand[weight-1] = weight;
      } else 
      { 
        // 
        assert(false); 
      }
    }
  }
  //std::cout << "Red weights remaining: " << redWeightsRemaining << "\n";
  //std::cout << "Blue weights remaining: " << blueWeightsRemaining << "\n";

  if(redWeightsRemaining == blueWeightsRemaining){
    //Red's turn
    //std::cout << "Red's turn.\n";
    stateBuffer->turn = State::Turn_Red;
  } else if(blueWeightsRemaining > redWeightsRemaining)
  {
    //Blue's turn
    //std::cout << "Blue's turn.\n";

    stateBuffer->turn = State::Turn_Blue;
  } else { assert(false); }
}

void printState(State &state)
{
  std::cout << "Red's hand: ";
  for(int i = 0; i < 10; i++)
  {
    std::cout << state.red.hand[i] << " ";
  }
  
  std::cout << "\nBlue's hand: ";
  for(int j = 0; j < 10; j++)
  {
    std::cout << state.blue.hand[j] << " ";
  }
  
  std::cout << "\nThe board: ";
  for(int k = -15; k < 16; k++){
    std::cout << state.board.GetPos(k) << " ";
  }
  std::cout << "\n";
  std::cout << "Phase is adding?: " << (state.phase == State::Phase_Adding) << "\n";
  std::cout << "Turn is red?: " << (state.turn == State::Turn_Red) << "\n";
}

std::string CalculateMoveWrapper()
{
  State stateBuffer; // get Statebuffer's previous states from a method that has saved it.
  //CalculateMove gets called on every move of the opponent, maintain a state somewhere and get it back.
  BuildState(std::cin, &stateBuffer);
  printState(stateBuffer);
  
  State sampleState;
  InitState(&sampleState);
  printState(sampleState);
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
  std::cout << "Position: " << position << "\n";
  std::cout << "Weight: " << weight << "\n";
  ss << position << " " << weight;
  return ss.str();
}



int main(int argc, char** argv)
{
  // Get the string for the state.
  std::cout << CalculateMoveWrapper() << "\n";

  return 0;
}
