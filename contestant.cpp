#include "contestant_util.h"
#include "ntg.h"
#include <iostream>

using namespace hps;

int main(int /*argc*/, char** /*argv*/)
{
  // Load statebuffer from status sting.
  State stateBuffer;
  while (BuildState(std::cin, &stateBuffer))
  {
    //PrintState(stateBuffer);
    // Proceed if input stream is still valid.
    std::cout << CalculateMoveWrapper(&stateBuffer) << std::endl;
  }
}
