/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/constants.hpp>

namespace libaudioverse_implementation {

float standard_panning_map_stereo[] = {-90.0f, 90.0f};
float standard_panning_map_surround40[] = {-45.0, 45.0, -135.0, 135.0};
float standard_panning_map_surround51[] = {-22.5f, 22.5f, INFINITY, INFINITY, -110.0f, 110.0f};
float standard_panning_map_surround71[] = {-22.5f, 22.5f, INFINITY, INFINITY, -150.0f, 150.0f, -110.0f, 110.0f};

}