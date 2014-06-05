/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef float LavMatrix[12];
typedef float LavVector[4];

/**A matrix is just 12 numbers.
Matrices are stored in row order, vectors are column matrices, and translation is the rightmost column.
index map:
0 1 2 3
4 5 6 7
8 9 10 11
And:
x=0, 1, 2
y=4, 5, 6
z=8, 9, 10
translation vector is in 3, 7, 11

For performance, these aren't objects.  They need to be included in other objects.*/
void initIdentityMatrix(LavMatrix* m);
void matrixSetTranslation(LavMatrix* m, LavVector *v);
void matrixGetTranslation(LavMatrix* m, LavVector *v);
void matrixApply(LavMatrix *m, LavVector* in, LavVector* out); //note: if in==out, this violates the aliasing rule and will break optimizers.

#ifdef __cplusplus
}
#endif