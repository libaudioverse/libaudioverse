/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

/**Optionals:
An optional is a class which either holds a value of type tOk or a value of type tError, and provides a method to check which.
tOk may be the same type as tError.  Use the helper functions, not the class itself: because tOk may be the same type as tError, we can't use constructor overloading.*/

template<class tOk, class tError>
class Optional;

template<class tOk, tError>
Optional<tOk, tError> OptionalOk(tOk val);

template<tOk, tError>
Optional<tOk, tError> OptionalError(tError val);

template<class tOk, class tError>
class Optional {
	public:
	bool isError() {return is_error;}
	tOk getValue() {return ok_value;}
	tError getError() {return err_value;}
	private:
	tOk ok_value;
	tError err_value;
	bool is_error;
	
};

