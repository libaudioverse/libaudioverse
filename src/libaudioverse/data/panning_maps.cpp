/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/constants.hpp>

namespace libaudioverse_implementation {

float standard_panning_map_stereo[] = {-90.0f, 90.0f};
float standard_panning_map_surround40[] = {-45.0, 45.0, -135.0, 135.0};
float standard_panning_map_surround51[] = {-22.5f, 22.5f, INFINITY, INFINITY, -110.0f, 110.0f};
float standard_panning_map_surround71[] = {-22.5f, 22.5f, INFINITY, INFINITY, -150.0f, 150.0f, -110.0f, 110.0f};

}