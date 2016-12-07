/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once

namespace libaudioverse_implementation {

//Apparently, C's mod is in fact not the discrete math operation.
//This function handles that.
int ringmodi(int dividend, int divisor);
float ringmodf(float dividend, float divisor);
double ringmod(double dividend,double divisor);

//Complex absolute value.
float cabs(float real, float imag);

//Work with decibals.
//These two are for gain conversions, and assume 0 db is 1.0.
double gainToDb(double gain);
double dbToGain(double gain);

double scalarToDb(double scalar, double reference);
double dbToScalar(double db, double reference);

//Run euclid's algorithm on two integers:
//Only works on positive integers.
int greatestCommonDivisor(int a, int b);

//Fill a buffer with a hadamard matrix of order n (n must be power of 2).
void hadamard(int n, float* buffer, bool shouldNormalize=true);

//Fill a buffer with a matrix representing a reflectiona bout a plane whose normal is (1, 1, 1, 1...)
//This is also known as a householder matrix.
void householder(int n, float* buffer, bool shouldNormalize =true);
}