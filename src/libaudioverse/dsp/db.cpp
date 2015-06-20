/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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