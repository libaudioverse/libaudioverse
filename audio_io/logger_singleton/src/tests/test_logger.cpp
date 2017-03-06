#include <logger_singleton/logger_singleton.hpp>
#include <string>
#include <stdio.h>

int main() {
	int count = 0;
	int expected = 10;
	//Use a scope so that the logger finishes before we check.
	//Lifetime of the logger is until we let go of the shared_ptr.
	{
		auto l = logger_singleton::createLogger();
		l->setLoggingCallback([&] (logger_singleton::LogMessage m) {
			printf("%s\n", m.message.c_str());
			count++;
		});
		for(int i = 0; i < expected; i++) {
			l->logCritical("test", "Message %i", i);
		}
	}
	if(count == expected) printf("Passed.\n");
	else printf("Failed.  Expected %i but got %i.\n", expected, count);
	return 0;
}
