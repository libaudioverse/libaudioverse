# logger_singleton#
A logging singleton intended primarily for actiing as a bridge to client code in my other projects. Runs anywhere C++11 does.
I wrote this because I don't need a framework.
I only need the ability to send log messages to a user-supplied callback.
if you are writing an application, this code may be helpful.
The real intent is library development.
In such a case, it is common to need to allow users to integrate your logging with their framework, a task for which this code is ideally suitable.

You use this code by calling `createLogger`, keeping the shared pointer around, and setting a callback to receive log messages.
When the last shared pointer dies, so too does the logger.

The purpose is to act as a bridge to client code while running anywhere.
For example, [Libaudioverse](http://github.com/camlorn/libaudioverse.git)  uses it to centralize all logging into one callback.

All calls to the callback happen in the same thread, provided they are to the same logger.

If your library is C++, then you can just expose the logger directly.
Otherwise, you'll need to translate the logging levels before sending them to your callback.
In the former case, exposing the logger directly will allow other projects using this code to integrate with it without converting from your logging levels again.
This consideration may matter if you are implementing many closely-related C++ libraries, i.e Libaudioverse and its dependencies.