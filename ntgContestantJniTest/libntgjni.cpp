#include "LibNtgJni.h"
//#include "no_tipping_game.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

namespace hps
{

  namespace ntg
  {
    std::string CalculateMove(std::string command)
    {
      std::cout<<"inside CalculateMove cpp: "<<std::endl;
      std::string curLine;
      std::cout<<"command: "<<command<<std::endl;
      std::stringstream ss(command);

      int onBoard;
      int position;
      std::string color;
      int weight;

      getline(ss, curLine);
      std::cout<<"curLine: "<<curLine<<std::endl;
      if(curLine == "ADDING" || curLine=="REMOVING")
      {
	std::cout<<"phase: "<<curLine<<std::endl;
	if(getline(ss, curLine))
	{
	  std::stringstream st(curLine);
	  st >> onBoard >> position >> color >> weight;
	}
	//	stateBuffer.phase = State::Phase_Adding;
      } 
      else 
      {
	std::cout<<"other player's move: "<<curLine<<std::endl;
      	std::stringstream st(curLine);
	st >> onBoard >> position >> color >> weight;
	//stateBuffer.phase = State::Phase_Removing;
      }
      
      
      
	if(onBoard == 1)
	{
	  //  stateBuffer.board.SetPos(position, weight);
	  std::cout<<"position: "<<position<<"weight: "<<weight<<std::endl;
	  std::cout<<"color: "<<color<<std::endl;
	  if(color == "Red")
	  {
	    //	    stateBuffer.red.hand[weight-1] = Player::Played;
	  }
	  else if(color == "Blue")
	  {
	    // stateBuffer.blue.hand[weight-1] = Player::Played;
	  }
	}
	else
	{
	  std::cout<<"position: "<<position<<"weight: "<<weight<<std::endl;
	  std::cout<<"color: "<<color<<std::endl;
	  if(color == "Red")
	  {
	    //stateBuffer.red.hand[weight-1] = weight;
	  }
	  else if(color == "Blue")
	  {
	    //  stateBuffer.blue.hand[weight-1] = weight;
	  } 
	  else 
	  { 
	    // assert(false); 
	  }
	}
      return "-4 3";
    }
  }
}
    
   JNIEXPORT jstring JNICALL Java_LibNtgJni_Compute
    (JNIEnv* env, jobject obj, jstring command)
    {
      const char* buf = env->GetStringUTFChars(command,0);
      std::string sCommand(buf);
      std::cout<<"inside compute, native"<<std::endl;
      //call evaluation function to get move.
      std::string move = hps::ntg::CalculateMove(sCommand);
      std::cout<<"move: "<<move<<std::endl;
      char cBuf[move.length()];
      strcpy(cBuf,move.c_str());
      env->ReleaseStringUTFChars(command,buf);
      return env->NewStringUTF(cBuf);
    }
