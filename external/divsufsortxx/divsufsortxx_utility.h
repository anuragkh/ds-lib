/*
 * divsufsortxx_utility.h
 * Copyright (c) 2003-2007 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// r1

#ifndef _DIVSUFSORTXX_UTILITY_H_
#define _DIVSUFSORTXX_UTILITY_H_

#include <iostream>


namespace divsufsortxx {

/* Checks the suffix array SA of the string T. */
template<typename StringIterator_type, typename SAIterator_type, typename alphabetsize_type>
int
check(const StringIterator_type T,
      SAIterator_type first, SAIterator_type last,
      alphabetsize_type alphabetsize, int verbose) {
typedef typename std::iterator_traits<SAIterator_type>::value_type pos_type;
typedef typename std::iterator_traits<StringIterator_type>::value_type value_type;

  pos_type *C;
  SAIterator_type i;
  pos_type j, p, q, t, n;
  value_type c;

  if(1 <= verbose) { std::cerr << "sufchecker: "; }

  /* Check arguments. */
  n = last - first;
  if(n == 0) {
    if(1 <= verbose) { std::cerr << "Done." << std::endl; }
    return 0;
  }
  if(n < 0) {
    if(1 <= verbose) { std::cerr << "Invalid arguments." << std::endl; }
    return -1;
  }

  /* ranges. */
  for(i = first; i < last; ++i) {
    if((*i < 0) || (n <= *i)) {
      if(1 <= verbose) {
        std::cerr << "Out of the range [0, " << (n - 1) << "]." << std::endl;
        std::cerr << "SA[" << i - first << "]=" << *i << std::endl;
      }
      return -2;
    }
  }

  /* first characters. */
  for(i = first + 1; i < last; ++i) {
    if(T[*(i - 1)] > T[*i]) {
      if(1 <= verbose) {
        std::cerr << "Suffixes in wrong order." << std::endl;
        std::cerr << "  T[SA[" << i - first << "] = " << *i << "] = " << T[*i] << " > ";
        std::cerr << "T[SA[" << i + 1 - first << "] = " << *(i + 1) << "] = " << T[*(i + 1)] << "," << std::endl;
      }
      return -3;
    }
  }

  /* suffixes. */
  C = new pos_type[alphabetsize];
  for(j = 0; j < alphabetsize; ++j) { C[j] = 0; }
  for(j = 0; j < n; ++j) { ++C[T[j]]; }
  for(j = 0, p = 0; j < alphabetsize; ++j) {
    t = C[j];
    C[j] = p;
    p += t;
  }
  q = C[T[n - 1]];
  C[T[n - 1]] += 1;

  for(i = first; i < last; ++i) {
    p = *i;
    if(0 < p) {
      c = T[--p];
      t = C[c];
    } else {
      c = T[p = n - 1];
      t = q;
    }
    if(p != first[t]) {
      std::cerr << "Suffix in wrong position." << std::endl;
      if(0 <= t) { std::cerr << "  SA[" << t << "] = " << first[t] << " or" << std::endl; }
      std::cerr << "  SA[" << i - first << "] = " << *i << std::endl;
      delete[] C;
      return -4;
    }
    if(t != q) {
      C[c] += 1;
      if((n <= C[c]) || (T[first[C[c]]] != c)) { C[c] = -1; }
    }
  }

  if(1 <= verbose) { std::cerr << "Done." << std::endl; }

  delete[] C;
  return 0;
}


template<typename StringIterator_type, typename pos_type>
int
compare(StringIterator_type T, StringIterator_type T_end,
        StringIterator_type P, StringIterator_type P_end,
        pos_type &match) {
  StringIterator_type Ti;
  StringIterator_type Pi;

  for(Ti = T + match, Pi = P + match;
      (Ti < T_end) && (Pi < P_end) && (*Ti == *Pi); ++Ti, ++Pi) { }
  match = Pi - P;

  return ((Ti < T_end) && (Pi < P_end)) ? (*Pi < *Ti) * 2 - 1 : -(Pi != P_end);
}

/* Search for the pattern P in the string T. */
template<typename StringIterator_type, typename SAIterator_type>
typename std::iterator_traits<SAIterator_type>::value_type
search(StringIterator_type T, StringIterator_type T_end,
       StringIterator_type P, StringIterator_type P_end,
       SAIterator_type SA, SAIterator_type SA_end,
       typename std::iterator_traits<SAIterator_type>::value_type &outidx) {
typedef typename std::iterator_traits<SAIterator_type>::value_type pos_type;
  pos_type size, lsize, rsize, half;
  pos_type match, lmatch, rmatch;
  pos_type llmatch, lrmatch, rlmatch, rrmatch;
  pos_type i, j, k;
  int r;

  outidx = -1;
  if((T_end < T) || (P_end < P) || (SA_end < SA)) { return -1; }
  if((T == T_end) || (SA == SA_end)) { return 0; }
  if(P == P_end) { outidx = 0; return SA_end - SA; }

  for(i = j = k = 0, lmatch = rmatch = 0, size = SA_end - SA, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    match = std::min(lmatch, rmatch);
    r = compare(T + SA[i + half], T_end, P, P_end, match);
    if(r < 0) {
      i += half + 1;
      half -= (size & 1) ^ 1;
      lmatch = match;
    } else if(r > 0) {
      rmatch = match;
    } else {
      lsize = half, j = i, rsize = size - half - 1, k = i + half + 1;

      /* left part */
      for(llmatch = lmatch, lrmatch = match, half = lsize >> 1;
          0 < lsize;
          lsize = half, half >>= 1) {
        lmatch = std::min(llmatch, lrmatch);
        r = compare(T + SA[j + half], T_end, P, P_end, lmatch);
        if(r < 0) {
          j += half + 1;
          half -= (lsize & 1) ^ 1;
          llmatch = lmatch;
        } else {
          lrmatch = lmatch;
        }
      }

      /* right part */
      for(rlmatch = match, rrmatch = rmatch, half = rsize >> 1;
          0 < rsize;
          rsize = half, half >>= 1) {
        rmatch = std::min(rlmatch, rrmatch);
        r = compare(T + SA[k + half], T_end, P, P_end, rmatch);
        if(r <= 0) {
          k += half + 1;
          half -= (rsize & 1) ^ 1;
          rlmatch = rmatch;
        } else {
          rrmatch = rmatch;
        }
      }

      break;
    }
  }

  outidx = (0 < (k - j)) ? j : i;
  return k - j;
}

} /* namespace divsufsortxx */

#endif /* _DIVSUFSORT_UTILITY_H_ */
