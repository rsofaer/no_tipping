#include "LibNtgJni.h"
#include "no_tipping_game.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include "minimax.h"
#include "adversarial_utils.h"



namespace hps
{
namespace ntg
{

void BuildState(std::string command,State& stateBuffer)
{
  std::string curLine;
  std::stringstream ss(command);

  int onBoard;
  int position;
  std::string color;
  int weight;
  
  int redWeightsRemaining = 0;
  int blueWeightsRemaining = 0;

  getline(ss, curLine);

  if(curLine == "ADDING" || curLine=="REMOVING")
  {
    if(getline(ss, curLine))
    {
      std::stringstream st(curLine);
      st >> onBoard >> position >> color >> weight;
    }
    if(curLine == "ADDING")
    {  
      stateBuffer.phase = State::Phase_Adding;
    }
    else
    {
      stateBuffer.phase = State::Phase_Removing;	  
    }
  } 
  else 
  {
    std::stringstream st(curLine);
    st >> onBoard >> position >> color >> weight;
  }

  if(onBoard == 1)
  {
    stateBuffer.board.SetPos(position, weight);
    if(color == "Red")
    {
      stateBuffer.red.hand[weight-1] = Player::Played;
    }
    else if(color == "Blue")
    {
      stateBuffer.blue.hand[weight-1] = Player::Played;
    }
  }
  else
  {
    std::cout<<"position: "<<position<<"weight: "<<weight<<std::endl;
    std::cout<<"color: "<<color<<std::endl;
    if(color == "Red")
    {
      stateBuffer.red.hand[weight-1] = weight;
      redWeightsRemaining++;
    }
    else if(color == "Blue")
    {
      stateBuffer.blue.hand[weight-1] = weight;
      blueWeightsRemaining++;
    } 
    else 
    { 
      assert(false); 
    }
  }
  
  if(redWeightsRemaining == blueWeightsRemaining){
    //Red's turn
    stateBuffer.turn = State::Turn_Red;
  } else if(blueWeightsRemaining > redWeightsRemaining)
  {
    //Blue's turn
    stateBuffer.turn = State::Turn_Blue;
  } else { assert(false); }
}

std::string CalculateMoveWrapper(std::string command)
{
  State stateBuffer; // get Statebuffer's previous states from a method that has saved it.
  //CalculateMove gets called on every move of the opponent, maintain a state somewhere and get it back.
  BuildState(command,stateBuffer);
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

}
}


enum { MaxCmdStrLen = 4096, };

JNIEXPORT jstring JNICALL Java_LibNtgJni_Compute(JNIEnv* env, 
						 jobject /*obj*/, 
						 jstring command)
{
  const char* buf = env->GetStringUTFChars(command,0);
  std::string sCommand(buf);
  std::string move = hps::ntg::CalculateMoveWrapper(sCommand);

  char cBuf[MaxCmdStrLen];
  strcpy(cBuf,move.c_str());
  env->ReleaseStringUTFChars(command,buf);

  return env->NewStringUTF(cBuf);
}
