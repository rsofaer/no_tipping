#include "contestant_util.h"
#include "ntg.h"
#include <iostream>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace hps;

int main(int /*argc*/, char** /*argv*/)
{
#ifdef WIN32
  Sleep(1000);
#else
  sleep(1);
#endif
  // Load statebuffer from status sting.
  State stateBuffer;
  while (BuildState(std::cin, &stateBuffer))
  {
    // Proceed if input stream is still valid.
    std::cout << CalculateMoveWrapper(&stateBuffer) << std::endl;
  }
}
