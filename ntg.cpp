#include "no_tipping_game.h"
#include "adversarial_utils.h"

using namespace hps;

int main(int /*argc*/, char** /*argv*/)
{
  // Find plys from initial state.
  {
    State state;
    InitState(&state);
    std::vector<Ply> plys;
    PossiblePlys(state, &plys);
    std::cout << "There are " << plys.size() << " plys from the initial "
              << "game state." << std::endl;
  }
  // Count states where blue wins.
  {
    StatePlysPair oneW;
    const int numWinStates = OneWeightFinalSet(&oneW);
    std::cout << "Blue wins from " << numWinStates << " possible states "
              << "since Red cannot remove a weight." << std::endl;
  }
  // Count states where red wins.
  {
    std::vector<StatePlysPair> twoW;
    const int numWinStates = TwoWeightFinalSet(&twoW);
    std::cout << "Red wins from " << numWinStates << " possible states "
              << "since Blue cannot remove a weight." << std::endl;
  }
  {
    State state;
    InitState(&state);
    BoardEvaluationInverseDepthWinStates evalFunc(State::Turn_Red);
    evalFunc(state);
  }
  return 0;
}
