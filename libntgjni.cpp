/*
ADDING
0 0 Red 1
0 0 Blue 1
0 0 Red 2
0 0 Blue 2
0 0 Red 3
0 0 Blue 3
0 0 Red 4
0 0 Blue 4
0 0 Red 5
0 0 Blue 5
0 0 Red 6
0 0 Blue 6
0 0 Red 7
0 0 Blue 7
0 0 Red 8
0 0 Blue 8
0 0 Red 9
0 0 Blue 9
0 0 Red 10
0 0 Blue 10
1 -4 Green 3

The format is a list of the pieces and their positions.
If a piece is on the board, column 0 is 1, otherwise, it is 0.
If a piece is on the board, column 2 is the position of that piece,
otherwise it is 0.

*/  

#include "no_tipping_game.h"
#include <iostream>
#include <string>
#include <sstream>


using namespace std;

namespace hps
{
  namespace ntg
  {
  void build_state(istream &command, State& stateBuffer)
  {
    string curLine;
    
    getline(command, curLine);
    if(curLine == "ADDING")
    {
      stateBuffer.phase = State::Phase_Adding;
    } else {
      stateBuffer.phase = State::Phase_Removing;
    }

    while(getline(command, curLine))
    {
      stringstream sstream(curLine);
      int onBoard;
      int position;
      string color;
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
          assert(false); 
        }
      }
    }
  }
  }
}
