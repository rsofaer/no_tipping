#include "no_tipping_game.h"
#include "adversarial_utils.h"

using namespace hps;

// reissb -- 20111001 --
//   Board evaluation function:
//     * One point for every winning state still reachable times
//       the inverse depth squared?
//       Something like this would favor more reachable win states and also
//       being closer to a win state.
//     * Size(WinningStates) * (MAX_DEPTH^2) for a winning state.
//     * Subtract inverse depth squared for every winning state of opponent
//       still reachable.

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
    State stateOneW;
    std::vector<Ply> oneW;
    OneWeightFinalSet(&stateOneW, &oneW);
    std::cout << "Blue wins from " << oneW.size() << " possible states "
              << "since Red cannot remove a weight." << std::endl;
  }
  // Count states where red wins.
  {
    State stateTwoW;
    std::vector<Ply> twoW;
    TwoWeightFinalSet(&stateTwoW, &twoW);
    std::cout << "Red wins from " << twoW.size() << " possible states "
              << "since Blue cannot remove a weight." << std::endl;
  }
//  // Count plys brute-force.
//  {
//    const long long totalPlys = CountPossiblePlys(state);
//    std::cout << "There are " << totalPlys << " possible non-suicidal "
//              << "plys in the no tipping game." << std::endl;
//  }
  return 0;
}
