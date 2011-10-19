#ifndef _MATH_COMBINATION_H_
#define _MATH_COMBINATION_H_
#include <vector>
#include <algorithm>
#include <functional>
#include <assert.h>

namespace hps
{
namespace math
{
 
/// <summary> A combination is a list of indices. </summary>
typedef std::vector<size_t> Combination;

/// <summary> Compute n! using a cache to increase performance. </summary>
unsigned long long Factorial(const unsigned int n);

/// <summary> Compute n choose k. </summary>
unsigned long long Choose(const unsigned int n, const unsigned int k);


/// <summary> Compute the combinadic of a number. </summary>
/// <remarks>
///   <para> Method to compute the mth lexicographic combination taken from
///     http://msdn.microsoft.com/en-us/library/aa289166(v=vs.71).aspx.
///   </para>
/// </remarks>
void Combinadic(const unsigned int n, const unsigned int k,
                const unsigned long long m,
                Combination* const combinadic);

/// <summary> Get the mth combination in lexicographic order. </summary>
/// <remarks>
///   <para> Method to compute the mth lexicographic combination taken from
///     http://msdn.microsoft.com/en-us/library/aa289166(v=vs.71).aspx.
///   </para>
/// </remarks>
inline void LexicographicCombination(const unsigned int n, const unsigned int k,
                                     const unsigned long long m,
                                     Combination* const combination)
{
  assert(combination);
  Combinadic(n, k, m, combination);
  std::transform(combination->begin(), combination->end(), combination->begin(),
                 std::bind1st(std::minus<Combination::value_type>(), n - 1));
}

namespace detail
{
/// <summary> Assist optimized generation of combinadic.
class CachedCombinadicGenerator
{
public:
  CachedCombinadicGenerator(const unsigned int n, const unsigned int k);

  /// <summary> Get the mth combinadic for the family nCk. </summary>
  void GetCombinadic(const unsigned long long m, Combination* const combinadic) const;

  inline unsigned int GetN() const
  {
    return m_n;
  }
  inline unsigned int GetK() const
  {
    return m_k;
  }
  inline const unsigned long long& GetCombinationCount() const
  {
    return m_nCk;
  }

private:
  /// <summary> Cache of nCk values to find the combinadic. </summary>
  std::vector<unsigned long long> m_chooseCache;
  size_t m_chooseCachePerK;
  unsigned int m_n;
  unsigned int m_k;
  unsigned long long m_nCk;
};
}

/// <summary> Access lexicograhic combinations from the same nCk family. </summary>
class RandomAccessLexicographicCombinations
  : public detail::CachedCombinadicGenerator
{
public:
  RandomAccessLexicographicCombinations(const unsigned int n, const unsigned int k);

  /// <summary> Get the mth lexicographic combination for nCk. </summary>
  inline void GetCombination(unsigned long long m, Combination* const combination) const
  {
    assert(combination);
    GetCombinadic(m, combination);
    std::transform(combination->begin(), combination->end(), combination->begin(),
                   std::bind1st(std::minus<Combination::value_type>(), GetN() - 1));
  }
};

/// <summary> Iterate the family of combinations nCk in O(k) time per iteration. </summary>
class FastCombinationIterator
{
public:
  FastCombinationIterator(const unsigned int n, const unsigned int k,
                          const unsigned long long nextM)
    : m_n(n),
      m_k(k),
      m_nCk(Choose(n, k)),
      m_m(0ULL),
      m_combination(k)
  {
    assert(nextM < m_nCk);
    if (0 == nextM)
    {
      m_m = m_nCk - 1;
    }
    else
    {
      m_m = nextM - 1;
    }
    LexicographicCombination(n, k, m_m, &m_combination);
  }

  void Next(unsigned long long* const m, Combination* const combination);

  inline unsigned long long GetCombinationCount() const
  {
    return m_nCk;
  }

private:
  unsigned int m_n;
  unsigned int m_k;
  unsigned long long m_nCk;
  unsigned long long m_m;
  Combination m_combination;
};

}
using namespace math;
}

#endif // _MATH_COMBINATION_H_

