/* BEEBS aha-mont64 benchmark

   This version, copyright (C) 2013-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Some of this code is referenced by the book the Hacker's Delight (which
   placed it in the public domain).  See http://www.hackersdelight.org/ */

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 316

/* Computes a*b mod m using Montgomery multiplication (MM). a, b, and m
are unsigned numbers with a, b < m < 2**64, and m odd. The code does
some 128-bit arithmetic.
   The variable r is fixed at 2**64, and its log base 2 at 64.
   Works with gcc on Windows and Linux. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint64_t uint64;
typedef int64_t int64;

/* ---------------------------- xbinGCD ----------------------------- */

/* C program implementing the extended binary GCD algorithm. C.f.
http://www.ucl.ac.uk/~ucahcjm/combopt/ext_gcd_python_programs.pdf. This
is a modification of that routine in that we find s and t s.t.
    gcd(a, b) = s*a - t*b,
rather than the same expression except with a + sign.
   This routine has been greatly simplified to take advantage of the
facts that in the MM use, argument a is a power of 2, and b is odd. Thus
there are no common powers of 2 to eliminate in the beginning. The
parent routine has two loops. The first drives down argument a until it
is 1, modifying u and v in the process. The second loop modifies s and
t, but because a = 1 on entry to the second loop, it can be easily seen
that the second loop doesn't alter u or v. Hence the result we want is u
and v from the end of the first loop, and we can delete the second loop.
   The intermediate and final results are always > 0, so there is no
trouble with negative quantities. Must have a either 0 or a power of 2
<= 2**63. A value of 0 for a is treated as 2**64. b can be any 64-bit
value.
   Parameter a is half what it "should" be. In other words, this function
does not find u and v st. u*a - v*b = 1, but rather u*(2a) - v*b = 1. */

void
xbinGCD (uint64 a, uint64 b, volatile uint64 * pu, volatile uint64 * pv)
{
  uint64 alpha, beta, u, v;

  u = 1;
  v = 0;
  alpha = a;
  beta = b;			// Note that alpha is
  // even and beta is odd.

  /* The invariant maintained from here on is:
     a = u*2*alpha - v*beta. */

// printf("Before, a u v = %016llx %016llx %016llx\n", a, u, v);
  //while (a > 0)
  //{
    a = a >> 1;
    if ((u & 1) == 0)
    {			// Delete a common
      u = u >> 1;
      v = v >> 1;		// factor of 2 in
    }			// u and v.
    else
    {
    /* We want to set u = (u + beta) >> 1, but
      that can overflow, so we use Dietz's method. */
    u = ((u ^ beta) >> 1) + (u & beta);
    v = (v >> 1) + alpha;
    }
//    printf("After,  a u v = %016llx %016llx %016llx\n", a, u, v);
  //}

// printf("At end,    a u v = %016llx %016llx %016llx\n", a, u, v);
  *pu = u;
  *pv = v;
  return;
}

/* ------------------------------ main ------------------------------ */
static uint64 in_a, in_b, in_m;

int main() {
  in_m = 0xfae849273928f89fLL;	// Must be odd.
  in_b = 0x14736defb9330573LL;	// Must be smaller than m.
  in_a = 0x0549372187237fefLL;	// Must be smaller than m.

  uint64 a, b, m, hr, p1hi, p1lo, p1, p, abar, bbar;
  uint64 phi, plo;
  volatile uint64 rinv, mprime;

  m = in_m;			// Must be odd.
  b = in_b;			// Must be smaller than m.
  a = in_a;			// Must be smaller than m.

  /* The MM method uses a quantity r that is the smallest power of 2
      that is larger than m, and hence also larger than a and b. Here we
      deal with a variable hr that is just half of r. This is because r can
      be as large as 2**64, which doesn't fit in one 64-bit word. So we
      deal with hr, where 2**63 <= hr <= 1, and make the appropriate
      adjustments wherever it is used.
      We fix r at 2**64, and its log base 2 at 64. It doesn't hurt if
      they are too big, it's just that some quantities (e.g., mprime) come
      out larger than they would otherwise be. */

  hr = 0x8000000000000000LL;

  /* Now, for the MM method, first compute the quantities that are
  functions of only r and m, and hence are relatively constant. These
  quantities can be used repeatedly, without change, when raising a
  number to a large power modulo m.
  First use the extended GCD algorithm to compute two numbers rinv
  and mprime, such that

  r*rinv - m*mprime = 1

  Reading this nodulo m, clearly r*rinv = 1 (mod m), i.e., rinv is the
  multiplicative inverse of r modulo m. It is needed to convert the
  result of MM back to a normal number. The other calculated number,
  mprime, is used in the MM algorithm. */

  xbinGCD (hr, m, &rinv, &mprime);	// xbinGCD, in effect, doubles hr.
  printf("%lu %lu\n", rinv, mprime);
}
