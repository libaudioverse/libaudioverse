#include <audio_io/private/com.hpp>
#include <powercores/utilities.hpp>
#include <thread>
#include <atomic>
#include <windows.h>

namespace audio_io {
namespace implementation {

const int sta_task_message = WM_USER;
const int sta_quit_message = WM_USER+1;

SingleThreadedApartment::SingleThreadedApartment() {
	ready.store(0);
	current_task.store(nullptr);
	apartment_thread = powercores::safeStartThread(&SingleThreadedApartment::apartmentThreadFunction, this);
	//Wait for the background thread to get a message queue.
	while(ready.load() == 0) std::this_thread::yield();
	apartment_thread_id = GetThreadId((HANDLE)apartment_thread.native_handle());
}

SingleThreadedApartment::~SingleThreadedApartment() {
	sendMessage(sta_quit_message, 0, 0);
	apartment_thread.join();
}

void SingleThreadedApartment::submitTask(std::function<void(void)> &task) {
	std::function<void(void)> *expected = nullptr;
	while(current_task.compare_exchange_weak(expected, &task) == false) {
		expected = nullptr; //because compare_exchange_weak might update it.
		std::this_thread::yield();
	}
	sendMessage(sta_task_message, 0, 0);
}

void SingleThreadedApartment::sendMessage(UINT msg, WPARAM wparam, LPARAM lparam) {
	PostThreadMessage(apartment_thread_id, msg, wparam, lparam);
}

void SingleThreadedApartment::apartmentThreadFunction() {
	MSG message;
	//MSDN says this call creates the queue.
	PeekMessage(&message, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	auto res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
	ready.store(1);
	while(true) {
		GetMessage(&message, nullptr, 0, 0);
		if(message.hwnd == NULL) { //It's a thread message. See if it's a command for us.
			if(message.message == sta_quit_message) break;
			if(message.message == sta_task_message) {
				auto task = current_task.load();
				(*task)();
				//All other threads wait until this goes to nullptr.
				current_task.store(nullptr);
			}
		}
		else DispatchMessage(&message);
	}
	//If we're destructing, we won't see any more calls into the object.
	//But it's possible that we're going to see other messages behind us, still.  Let's try to process them here.
	while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) DispatchMessage(&message);
	CoUninitialize();
}


}
}