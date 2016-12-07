/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/constants.hpp>

namespace libaudioverse_implementation {

float standard_panning_map_stereo[] = {-90.0f, 90.0f};
float standard_panning_map_surround40[] = {-45.0, 45.0, -135.0, 135.0};
float standard_panning_map_surround51[] = {-22.5f, 22.5f, INFINITY, INFINITY, -110.0f, 110.0f};
float standard_panning_map_surround71[] = {-22.5f, 22.5f, INFINITY, INFINITY, -150.0f, 150.0f, -110.0f, 110.0f};

}