/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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