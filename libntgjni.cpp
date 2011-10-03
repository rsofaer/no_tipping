#include "LibNtgJni.h"
//#include "no_tipping_game.h"
#include <string>
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
  }
}
