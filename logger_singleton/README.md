# logger_singleton#
A logging singleton intended primarily for actiing as a bridge to client code in my other projects. Runs anywhere C++11 does.

You use this code by calling `getLogger`, keeping the shared pointer around, and setting a callback to receive log messages.
Any code statically linked to this library then shares the logger.

The purpose is to act as a bridge to client code while running anywhere.
For example, [Libaudioverse](http://github.com/camlorn/libaudioverse.git)  uses it to centralize all logging into one callback.

All calls to the callback happen in the same thread, provided they are to the same logger.
See below for best practices.

Calling `initialize` creates a reference to the shared pointer `getLogger` returns; you need to call `shutdown` to kill it.
Whoever has the last shared pointer is responsible for cleanup.

Note that calls to `initialize` and `shutdown` nest.
Each call to `initialize` must be matched to a call of `shutdown`, otherwise the singleton will not die properly.
If you fail to meet this guideline, behavior is undefined but will probably result in `std::terminate` being called at application exit.
`initialize` and `shutdown` are also not threadsafe: you should call them from the main thread of the application or, at the least, ensure they cannot be called concurrently.

Use this library as follows, and everything will go smoothly:

- Integrate `initialize` and `shutdown` calls into your library's initialization and shutdown functions.  If you don't have some, make them.

- Every time you want to log a  essage, make a new call to `getLogger`.

- If you need to interface with other things that might log, forward them to the logger here.

- Provide a way for the user to set a callback to receive logging messages via `setCallback`.

And now all your logging messages will go to the same place.