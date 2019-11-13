/* BEEBS minver benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor Pierre Langlois <pierre.langlois@embecosm.com>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   *************************************************************************
   *                                                                       *
   *   SNU-RT Benchmark Suite for Worst Case Timing Analysis               *
   *   =====================================================               *
   *                              Collected and Modified by S.-S. Lim      *
   *                                           sslim@archi.snu.ac.kr       *
   *                                         Real-Time Research Group      *
   *                                        Seoul National University      *
   *                                                                       *
   *                                                                       *
   *        < Features > - restrictions for our experimental environment   *
   *                                                                       *
   *          1. Completely structured.                                    *
   *               - There are no unconditional jumps.                     *
   *               - There are no exit from loop bodies.                   *
   *                 (There are no 'break' or 'return' in loop bodies)     *
   *          2. No 'switch' statements.                                   *
   *          3. No 'do..while' statements.                                *
   *          4. Expressions are restricted.                               *
   *               - There are no multiple expressions joined by 'or',     *
   *                'and' operations.                                      *
   *          5. No library calls.                                         *
   *               - All the functions needed are implemented in the       *
   *                 source file.                                          *
   *                                                                       *
   *                                                                       *
   *************************************************************************
   *                                                                       *
   *  FILE: minver.c                                                       *
   *  SOURCE : Turbo C Programming for Engineering by Hyun Soo Ahn         *
   *                                                                       *
   *  DESCRIPTION :                                                        *
   *                                                                       *
   *     Matrix inversion for 3x3 floating point matrix.                   *
   *                                                                       *
   *  REMARK :                                                             *
   *                                                                       *
   *  EXECUTION TIME :                                                     *
   *                                                                       *
   *                                                                       *
   *************************************************************************

*/

#include <math.h>
#include <string.h>
#include <stdio.h>

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 329

static float a_ref[3][3] = {
  {3.0, -6.0, 7.0},
  {9.0, 0.0, -5.0},
  {5.0, -8.0, 6.0},
};

static float b[3][3] = {
  {-3.0, 0.0, 2.0},
  {3.0, -2.0, 0.0},
  {0.0, 2.0, -3.0},
};

static float a[3][3], c[3][3], d[3][3], det;

static float
minver_fabs (float n)
{
  float f;

  if (n >= 0)
    f = n;
  else
    f = -n;
  return f;
}

float
minver (int row, int col, float eps)
{
  int work[500], i, j, k, r, iw, u, v;
  float w, wmax, pivot, api, w1;

  r = w = 0;
  //if (row < 2 || row > 500 || eps <= 0.0)
  //  return (999);
  //w1 = 1.0;
  //for (i = 0; i < row; i++)
  //  work[i] = i;
  k = 0;
  //for (k = 0; k < row; k++)
  //{
    wmax = 2.0;
    i = k;
    //for (i = k; i < row; i++)
	  //{
	    w = minver_fabs (a_ref[i][k]); // was a[i][k]
	    if (w > wmax)
	    {
	      wmax = w;
	      r = i;
	    }
	  //}
  //}

  return wmax;
}

int main() {
  float ret = minver(0,0,0.001f);
  printf("%f\n", ret);
}