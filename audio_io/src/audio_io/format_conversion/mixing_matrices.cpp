#include <audio_io/private/mixing_matrices.hpp>

namespace audio_io {
namespace implementation {

/*These are the default mixing matrices.

Each matrix is stored in row-major order.  The rows represent output channels and the columns represent input channels.  Each entry should be between 0 and 1, and no row should sum to more than 1.

These are matrix multiplied with an n-column vector representing audio samples in simulation.cpp.

Naming convention: mixing_matrix_m_n.
Order: mixing_matrix_1_n for all n, mixing_matrix_2_n for all n...we exclude n=m.

Note: this file exists so that we can do something reasonable when the output object is not the device format.  For most apps, this will not trigger.  It is expected that the user will be asked or the backend will provide the info.
It is not inconceivable that this will be useful for testing, and those apps which make the request will still need reasonable audio until they have input from the user.

Note 2: This file will need revision as practical examples of these configurations are found in the wild.

The matrices here are based on the WebAudio spec, save for those to do with 7.1 surround sound for which no specification is given.*/

//this is the registry. The matrices are below.
MixingMatrixInfo mixing_matrix_list[] = {
	{1, 2, mixing_matrix_1_2},
	{1, 4, mixing_matrix_1_4},
	{1, 6, mixing_matrix_1_6},
	{1, 8, mixing_matrix_1_8},
	{2, 1, mixing_matrix_2_1},
	{2, 4, mixing_matrix_2_4},
	{2, 6, mixing_matrix_2_6},
	{2, 8, mixing_matrix_2_8},
	{4, 1, mixing_matrix_4_1},
	{4, 2, mixing_matrix_4_2},
	{4, 6, mixing_matrix_4_6},
	{4, 8, mixing_matrix_4_8},
	{6, 1, mixing_matrix_6_1},
	{6, 2, mixing_matrix_6_2},
	{6, 4, mixing_matrix_6_4},
	{6, 8, mixing_matrix_6_8},
	{8, 1, mixing_matrix_8_1},
	{8, 2, mixing_matrix_8_2},
	{8, 4, mixing_matrix_8_4},
	{8, 6, mixing_matrix_8_6},
{0, 0, nullptr},
};

float mixing_matrix_1_2[] = {
1.0f,
1.0f,
};

//Mono goes to the two front channels.
float mixing_matrix_1_4[] = {
	1.0,
	1.0,
	0.0,
	0.0
};

//fl, fr, fc, lfe, bl, br
//Send mono to center channel only.
float mixing_matrix_1_6[] = {
0.0,
0.0,
1.0,
0.0,
0.0,
0.0,
};

//fl, fr, fc, lfe, bl, br, sl, sr
//Mono goes to center only.
float mixing_matrix_1_8[] = {
0.0f,
0.0f,
1.0f,
0.0f,
0.0f,
0.0f,
0.0f,
0.0f,
};

float mixing_matrix_2_1[] = {
0.5, 0.5
};

//Copy stereo channels to front left and right.
float mixing_matrix_2_4[] = {
	1.0f, 0.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 0.0f,
};

//fl, fr, fc, lfe, bl, br
float mixing_matrix_2_6[] = {
1.0f, 0.0f, //left to front left
0.0f, 1.0f, //right to front right
0.0f, 0.0f, //don't use FC. FC needs filters of one sort or another to work right.
0.0f, 0.0f, //don't use lfe.
0.0f, 0.0f, //No back left.
0.0f, 0.0f, //No back right.
};

float mixing_matrix_2_8[] = {
1.0f, 0.0f, //left to fl.
0.0f, 1.0f, //right to fr.
0.0f, 0.0f, //dont' use fc.
0.0f, 0.0f, //don't use LFE.
0.0f, 0.0f, //No back left.
0.0f, 0.0f, //No back right.
0.0f, 0.0f,
0.0f, 0.0f,
};

float mixing_matrix_4_1[] {
	0.25f, 0.25f, 0.25f, 0.25f,
};

float mixing_matrix_4_2[] {
	0.5f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.5f,
};

float mixing_matrix_4_6[] {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

float mixing_matrix_4_8[] {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.7071f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.7071f,
	0.0f, 0.0f, 0.7071f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.7071f,
};

//These numbers taken from webaudio spec.
//output = 0.7071 * (input.L + input.R) + input.C + 0.5 * (input.SL + input.SR)
float mixing_matrix_6_1[] = {
0.707, 0.707, 1.0, 0.0, 0.5, 0.5,
};

/*Again, from webaudio:
output.L = L + 0.7071 * (input.C + input.SL)
output.R = R + 0.7071 * (input.C + input.SR)
*/
float mixing_matrix_6_2[] = {
1.0, 0.0, 0.7071, 0.0, 0.7071, 0.0,
0.0, 1.0, 0.7071, 0.0, 0.0, 0.7071,
};

float mixing_matrix_6_4[] {
	1.0f, 0.0f, 0.7071f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.7071f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

float mixing_matrix_6_8[] = {
//copy everything from 6 channels, but mirror bl and br to sl and sr.
//in addition, halve bl and br.
1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //fl
0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //fr
0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //fc
0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //lfe.
//these next ones are not identity.
0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, //bl.
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, //br
0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, //sl.
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, //sr.
};

//Based off 6_1, but assuming that the sl and bl contribute equally.
float mixing_matrix_8_1[] = {
0.7071, 0.7071, 1.0, 0.0, 0.25, 0.25, 0.25, 0.25,
};

//Based off 6_2 in the same wasy as 8_1 is based off 6_1.
float mixing_matrix_8_2[] = {
1.0, 0.0, 0.7071, 0.0, 0.7071/2, 0.0, 0.7071/2, 0.0,
0.0, 1.0, 0.7071, 0.0, 0.0, 0.7071/2, 0.0, 0.7071/2,
};

float mixing_matrix_8_4[] {
	1.0f, 0.0f, 0.7071f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.7071f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f,
};

float mixing_matrix_8_6[] = {
//identity, but combine sr and br as well as sl and bl.
1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //fl
0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //fr.
0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //fc
0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //lfe.
//end identity:
0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, //bl.
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, //br.
};

}
}