#pragma once

namespace audio_io {
namespace implementation {

struct MixingMatrixInfo {
unsigned int in_channels, out_channels;
float* pointer;
};

//ends with a sentinal, {0, 0, nulptr}
extern MixingMatrixInfo mixing_matrix_list[];

extern float mixing_matrix_1_2[];
extern float mixing_matrix_1_4[];
extern float mixing_matrix_1_6[];
extern float mixing_matrix_1_8[];

extern float mixing_matrix_2_1[];
extern float mixing_matrix_2_4[];
extern float mixing_matrix_2_6[];
extern float mixing_matrix_2_8[];

extern float mixing_matrix_4_1[];
extern float mixing_matrix_4_2[];
extern float mixing_matrix_4_6[];
extern float mixing_matrix_4_8[];

extern float mixing_matrix_6_1[];
extern float mixing_matrix_6_2[];
extern float mixing_matrix_6_4[];
extern float mixing_matrix_6_8[];

extern float mixing_matrix_8_1[];
extern float mixing_matrix_8_2[];
extern float mixing_matrix_8_4[];
extern float mixing_matrix_8_6[];

}
}