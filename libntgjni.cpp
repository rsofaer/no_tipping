#include "LibNtgJni.h"
#include "no_tipping_game.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

namespace hps
{
  namespace ntg
  {
    JNIEXPORT jstring JNICALL Java_LibNtgJni_Compute
    (JNIEnv* env, jobject obj, jstring command)
    {
      const char* buf = env->GetStringUTFChars(command,0);
      std::string sBuf(buf);
      //call evaluation function to get move.
      std::string move;
      char cBuf[move.length()];
      strcpy(cBuf,move.c_str());
      env->ReleaseStringUTFChars(command,buf);
      return env->NewStringUTF(cBuf);
    }
    
    void StateFromCommand(std::string &command, State& stateBuffer)
  {
    std::string curLine;
    std::stringstream ss(command);
    getline(ss, curLine);
    if(curLine == "ADDING")
    {
      stateBuffer.phase = State::Phase_Adding;
    } else {
      stateBuffer.phase = State::Phase_Removing;
    }

    while(getline(ss, curLine))
    {
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
        }else{
          // it is the green block.
          return;
        }
      } else{
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
  }
}
}
