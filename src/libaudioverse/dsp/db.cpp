/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/dspmath.hpp>
#include <math.h>

namespace libaudioverse_implementation {

/**As this is an audio application, we use the field quantity definition of DB, namely:
20 log_10 (scalar/reference) db
*/


double gainToDb(double gain) {
	return scalarToDb(gain, 1.0);
}

double dbToGain(double db) {
	return dbToScalar(db, 1.0);
}

double scalarToDb(double scalar, double reference) {
	return 20*log10(scalar/reference);
}

double dbToScalar(double db, double reference) {
	/*db=20log10(scalar/reference)
	db/20=log10(scalar/reference)
	db/20=log10 scalar-log10 reference
	db/20+log10 reference= log10 scalar
	10^(db/20+log10 reference)=scalar
	*/
	return pow(10, (db/20.0)+log10(reference));
}

}