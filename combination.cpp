#include "combination.h"

namespace hps
{
namespace math
{

unsigned long long Factorial(const unsigned int n)
{
  // 21! > max unsigned long long.
  enum { MaxFactorial = 20, };
  static std::vector<unsigned long long> s_factorialCache(MaxFactorial, 0);
  assert(n <= MaxFactorial);

  if (n < 1)
  {
    return 1ULL;
  }
  else
  {
    unsigned long long& nFactorial = s_factorialCache[n - 1];
    if (0 != nFactorial)
    {
      return nFactorial;
    }
    else
    {
      nFactorial = n * Factorial(n - 1);
      return nFactorial;
    }
  }
}

unsigned long long Choose(const unsigned int n, const unsigned int k)
{
  const bool zeroParam = (0 == (n * k));
  const bool chooseMoreThanN = (k > n);
  if (zeroParam || chooseMoreThanN)
  {
    return 0;
  }
  else if (n == k)
  {
    return 1;
  }

  const unsigned int k_dual = (k > ((n + 1) / 2)) ? (n - k) : k;
  unsigned long long delta = n - k_dual;
  unsigned long long nCk = delta + 1;
  for (unsigned int i = 2; i <= k_dual; ++i)
  {
    nCk *= (delta + i);
    nCk /= i;
  }
  return nCk;
}

void Combinadic(const unsigned int n, const unsigned int k,
                const unsigned long long m,
                Combination* const combinadic)
{
  assert(combinadic);
  combinadic->resize(k);

  // Find numbers such that m = Choose(n_1, k) + ... + Chooes(n_k, 1)
  const unsigned long long nCk = Choose(n, k);
  const unsigned long long dual = nCk - static_cast<unsigned long long>(m + 1);
  unsigned long long remain = dual;
  unsigned int maxN = n;
  for (unsigned int cmbIdx = 0UL; cmbIdx < k; ++cmbIdx)
  {
    const unsigned int kSearch = k - cmbIdx;
    unsigned long long largestV = Choose(--maxN, kSearch);
    while (largestV > remain)
    {
      largestV = Choose(--maxN, kSearch);
    }
    remain -= largestV;
    combinadic->at(cmbIdx) = static_cast<size_t>(maxN);
  }
  assert(0 == remain);
}

detail::CachedCombinadicGenerator::
CachedCombinadicGenerator(const unsigned int n, const unsigned int k)
: m_chooseCache(),
  m_chooseCachePerK(0UL),
  m_n(n),
  m_k(k),
  m_nCk(0ULL)
{
  assert(n >= k);
  // Init cache storage with all nCk that we need to compute combinadic.
  const size_t cacheSize = (n > (k + 2)) ? (n - k - 1) * k : 0;
  if (cacheSize > 0)
  {
    m_chooseCachePerK = (n - k - 1);
    m_chooseCache.reserve(cacheSize);
    for (unsigned int kIncr = k; kIncr > 0; --kIncr)
    {
      for (unsigned int nIncr = kIncr + m_chooseCachePerK; nIncr > kIncr; --nIncr)
      {
        m_chooseCache.push_back(Choose(nIncr, kIncr));
      }
    }
  }
  // Store number of combinations.
  m_nCk = Choose(m_n, m_k);
}

void detail::CachedCombinadicGenerator::
GetCombinadic(const unsigned long long m, Combination* const combinadic) const
{
  assert(combinadic);
  combinadic->resize(m_k);

  // Find numbers such that m = Choose(n_1, k) + ... + Chooes(n_k, 1)
  const unsigned long long dual = m_nCk - static_cast<unsigned long long>(m + 1);
  unsigned long long remain = dual;
  unsigned int maxN = m_n - 1;
  // Use cache to compute nCk values.
  const std::vector<unsigned long long>& chooseCache = m_chooseCache;
  const size_t cacheSize = m_chooseCache.size();
  size_t cacheIdx = 0UL; 
  size_t maxSearchIdx = m_chooseCachePerK;
  for (unsigned int cmbIdx = 0UL; cmbIdx < m_k; ++cmbIdx, --maxN)
  {
    // Find the maxN such that Choose(maxN, k - n_cmbIdx) <= remain.
    // Only search when maxSearchIdx is enabled (not reached trivial cases
    //   nCn or (n-1)Cn).
    size_t searchIdx;
    bool foundValue = false;
    for (searchIdx = 0;
         (searchIdx < maxSearchIdx) && (cacheIdx < cacheSize);
         ++searchIdx, ++cacheIdx, --maxN)
    {
      const unsigned long long& cacheVal = chooseCache.at(cacheIdx);
      if (cacheVal <= remain)
      {
        foundValue = true;
        remain -= cacheVal;
        combinadic->at(cmbIdx) = maxN;
        cacheIdx += m_chooseCachePerK;
        break;
      }
    }
    // Else need to check 1Ck or 0Ck cases.
    if (!foundValue)
    {
      // Everything from now on is either 0 or one case.
      maxSearchIdx = 0;
      if (remain > 0)
      {
        // When we hit zero, the next round drops by one.
        combinadic->at(cmbIdx) = GetK() - cmbIdx;
        --remain;
      }
      else
      {
        combinadic->at(cmbIdx) = GetK() - cmbIdx - 1;
      }
    }
  }
  assert(0 == remain);
}

RandomAccessLexicographicCombinations::
RandomAccessLexicographicCombinations(const unsigned int n, const unsigned int k)
: detail::CachedCombinadicGenerator(n, k)
{}

void FastCombinationIterator::
Next(unsigned long long* const m, Combination* const combination)
{
  // Generate the next combination by traversing the wheels in reverse.
  unsigned int kIdx = 0UL;
  unsigned int nextWheelValue = 0UL;
  Combination::reverse_iterator wheelRev = m_combination.rbegin();
  Combination::iterator wheel = m_combination.end();
  for (; kIdx < m_k; ++kIdx, ++wheelRev, --wheel)
  {
    nextWheelValue = *wheelRev + 1;
    // Did we find the next iteration?
    if (nextWheelValue < (m_n - kIdx))
    {
      --wheel;
      break;
    }
  }
  nextWheelValue %= (m_n - kIdx + 1); 
  for (; wheel != m_combination.end(); ++wheel, ++nextWheelValue)
  {
    *wheel = nextWheelValue;
  }
  // Compute this m and return combination.
  m_m = ++m_m % m_nCk;
  *m = m_m;
  *combination = m_combination;
}

}
}
