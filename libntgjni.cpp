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
  std::cout << "Building state\n";
    std::string curLine;
    std::stringstream ss(command);
    int redWeightsRemaining=0;
    int blueWeightsRemaining=0;
    std::string::size_type next = command.find_first_of(";");
    int prev=0;
    curLine = command.substr(prev, next-prev);
    std::cout << curLine << "\n";

    if(curLine == "ADDING")
    {
        stateBuffer.phase = State::Phase_Adding;
    } 
    else 
    {
        stateBuffer.phase = State::Phase_Removing;
    }
    
    while(next!=std::string::npos)
    {
      prev = next;
      next = command.find_first_of(";",prev);
      curLine = command.substr(prev, next-prev);
        std::stringstream sstream(curLine);
        int onBoard;
        int position;
        std::string color;
        int weight;
        
        sstream >> onBoard >> position >> color >> weight;
        
        if(onBoard == 1)
        {
            stateBuffer.board.SetPos(position, weight);
            if(color == "Red")
            {
                stateBuffer.red.hand[weight-1] = Player::Played;
            }else if(color == "Blue")
            {
                stateBuffer.blue.hand[weight-1] = Player::Played;
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
                stateBuffer.red.hand[weight-1] = weight;
            } else if(color == "Blue")
            {
                stateBuffer.blue.hand[weight-1] = weight;
            } else 
            { 
                // 
                assert(false); 
            }
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

    BoardEvaluationReachableWinStates evalFunc(stateBuffer.turn);
    Ply ply;
    Minimax::Run(&params, &stateBuffer, &evalFunc, &ply);
    int position = ply.pos;
    int weight = CurrentPlayer(&stateBuffer)->hand[ply.wIdx];
    std::stringstream ss;
    ss << position << " " << weight;
    return "yo!";
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
#if WIN32
  strcpy_s(cBuf,sizeof(cBuf),move.c_str());
#else
  strcpy(cBuf,move.c_str());
#endif
  env->ReleaseStringUTFChars(command,buf);

  return env->NewStringUTF(cBuf);
}
