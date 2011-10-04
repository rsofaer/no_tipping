#ifndef _MATH_RAND_BOUND_GENERATOR_H_
#define _MATH_RAND_BOUND_GENERATOR_H_
#include <math.h>
#include <cstdlib>

namespace hps
{
namespace math
{

/// <summary> Partition consecutive intervals of size bound mapped to the
///   numbers [0, bound - 1].
/// </summary>
int RandBound(const int bound)
{
  assert(bound <= RAND_MAX);
  int factor = ((RAND_MAX - bound) / bound) + 1;
  int limit = factor * bound;
  int r;
  do
  {
    r = rand();
  } while (r >= limit);
  return r / factor;
}

/// <summary> Partition consecutive intervals of size bound mapped to the
///   numbers [0, bound - 1].
/// </summary>
struct RandBoundedGenerator
{
  RandBoundedGenerator(const int bound_)
  : bound(bound_),
    factor(((RAND_MAX - bound) / bound) + 1),
    limit(factor * bound)
  {
    assert(bound <= RAND_MAX);
  }

  inline int operator()() const
  {
    int r;
    do
    {
      r = rand();
    } while (r >= limit);
    return r / factor;
  }

  int bound;
  int factor;
  int limit;
};

}
using namespace math;
}

#endif //_MATH_RAND_BOUND_GENERATOR_H_
