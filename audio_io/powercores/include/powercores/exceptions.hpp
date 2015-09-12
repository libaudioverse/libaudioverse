/**This file is part of Lambdatask, released under the terms of the Unlicense.
See LICENSE in the root of the Lambdatask repository for details.*/
#pragma once
#include <exception>

namespace powercores {

/**Thrown when an operation with a timeout times out.*/
class TimeoutException: public std::exception {
};

}