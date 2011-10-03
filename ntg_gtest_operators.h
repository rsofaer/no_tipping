#ifndef _NO_TIPPING_GAME_NTG_GTEST_OPERATORS_H_
#define _NO_TIPPING_GAME_NTG_GTEST_OPERATORS_H_
#include "no_tipping_game.h"

namespace hps
{
namespace ntg
{
namespace detail
{

template <int BoardWeight_, int PivotL_, int PivotR_, int Size_>
bool operator==(const GenericBoard<BoardWeight_, PivotL_, PivotR_, Size_>& lhs,
                const GenericBoard<BoardWeight_, PivotL_, PivotR_, Size_>& rhs)
{
  typedef GenericBoard<BoardWeight_, PivotL_, PivotR_, Size_> BoardType;
  BoardType::const_iterator lhsWt = lhs.begin();
  BoardType::const_iterator lhsWtE = lhs.end();
  BoardType::const_iterator rhsWt = rhs.begin();
  for (; lhsWt != lhsWtE; ++lhsWt, ++rhsWt)
  {
    if (*lhsWt != *rhsWt)
    {
      return false;
    }
  }
  return true;
}

template <int BoardWeight_, int PivotL_, int PivotR_, int Size_>
inline bool operator!=(const GenericBoard<BoardWeight_, PivotL_, PivotR_, Size_>& lhs,
                       const GenericBoard<BoardWeight_, PivotL_, PivotR_, Size_>& rhs)
{
  return !(lhs == rhs);
}

template <int NumWeights_>
bool operator==(const GenericPlayer<NumWeights_>& lhs,
                const GenericPlayer<NumWeights_>& rhs)
{
  typedef GenericPlayer<NumWeights_> PlayerType;
  const Weight* lhsWt = lhs.hand;
  const Weight* lhsWtE = lhs.hand + NumWeights_;
  const Weight* rhsWt = rhs.hand;
  if (lhs.remain != rhs.remain)
  {
    return false;
  }
  for (; lhsWt != lhsWtE; ++lhsWt, ++rhsWt)
  {
    if (*lhsWt != *rhsWt)
    {
      return false;
    }
  }
  return true;
}

template <int NumWeights_>
inline bool operator!=(const GenericPlayer<NumWeights_>& lhs,
                       const GenericPlayer<NumWeights_>& rhs)
{
  return !(lhs == rhs);
}

}

inline bool operator==(const State& lhs, const State& rhs)
{
  return (lhs.board == rhs.board) &&
         (lhs.red == rhs.red) &&
         (lhs.blue == rhs.blue) &&
         (lhs.turn == rhs.turn) &&
         (lhs.phase == rhs.phase);
}

inline bool operator!=(const State& lhs, const State& rhs)
{
  return !(lhs == rhs);
}

}
using namespace ntg;
}


#endif //_NO_TIPPING_GAME_NTG_GTEST_OPERATORS_H_
