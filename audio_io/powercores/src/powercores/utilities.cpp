#include <powercores/utilities.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>

namespace powercores {

long long getThreadId() {
	//This relies on "magic statics", the threadsafe initialization of statics, ensured by the compiler.
	thread_local long long id = 0; //0 means no id.
	static std::atomic<long long> globallyUniqueId{1};
	if(id == 0) id = globallyUniqueId.fetch_add(1, std::memory_order_relaxed);
	return id;
}

//Helper class for atThreadExitImpl.
class AtThreadExitImplHelper {
	public:
	~AtThreadExitImplHelper();
	void schedule(std::function<void(void)> what);
	std::vector<std::function<void(void)>> at_exit;
};

AtThreadExitImplHelper::~AtThreadExitImplHelper() {
	for(auto &i: at_exit) i();
}

void AtThreadExitImplHelper::schedule(std::function<void(void)> what) {
	at_exit.emplace_back(what);
}

thread_local AtThreadExitImplHelper at_thread_exit;
void atThreadExitImpl(std::function<void(void)> what) {
	at_thread_exit.schedule(what);
}

}