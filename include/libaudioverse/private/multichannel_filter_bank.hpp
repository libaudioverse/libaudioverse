/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <functional>

namespace libaudioverse_implementation {

/**The following configures and manages filters using similar tricks to smart pointers.

You can do mcfb->anythingOnTheFilters, as well as . to use the functions here.
*/

template<typename filter_type>
class MultichannelFilterBank {
	public:
	template<typename... constructor_args>
	MultichannelFilterBank(constructor_args... args);
	~MultichannelFilterBank();
	int getChannelCount();
	void setChannelCount(int newCount);
	void reset();
	filter_type& operator*();	
	filter_type* operator->();
	
	//Ticking and block processing.
	void tick(float* inputFrame, float* outputFrame);
	void process(int blockSize, float** inputs, float** outputs);
	//Like the above, but calls a callable for every sample.
	//Callable gets a reference to the filter, the sample index and any additional arguments passed here.
	//The filter is temporarily unhooked from the chain, allowing individual implementation.
	//This is for implementing a-rate automation on nodes where all filters are always the same anyway.
	template<typename CallableT, typename... ArgsT>
	void process(int blockSize, float** inputs, float** outputs, CallableT callable, ArgsT... args);
	private:
	std::function<filter_type*(void)> filter_creator; //used for type erasure.
	filter_type* first = nullptr;
	int channel_count = 0;
};

//Constructor is tricky.  We use a lambda to wrap up the parameter pack.
template<typename filter_type>
template<typename... constructor_args>
MultichannelFilterBank<filter_type>::MultichannelFilterBank(constructor_args... args) {
	filter_creator = [args...]() {
		return new filter_type(args...);
	};
	first = filter_creator();
	channel_count = 1;
}

template<typename filter_type>
MultichannelFilterBank<filter_type>::~MultichannelFilterBank() {
	while(first) {
		auto t = first;
		first = first->getSlave();
		delete t;
	}
}

template<typename filter_type>
int MultichannelFilterBank<filter_type>::getChannelCount() {
	return channel_count;
}

template<typename filter_type>
void MultichannelFilterBank<filter_type>::setChannelCount(int newCount) {
	//Two choices, either it's less or it's greater.
	if(newCount < channel_count) {
		int drop = channel_count-newCount;
		while(drop) {
			auto t = first;
			first = first->getSlave();
			delete t;
		}
	}
	else {
		auto last = first;
		while(last && last->getSlave()) last = last->getSlave();
		int add = newCount-channel_count;
		while(add) {
			last->setSlave(filter_creator());
			last = last->getSlave();
			add--;
		}
	}
	channel_count = newCount;
}

template<typename filter_type>
void MultichannelFilterBank<filter_type>::reset() {
	auto f = first;
	while(f) {
		f->reset();
		f = f->getSlave();
	}
}

template<typename filter_type>
filter_type& MultichannelFilterBank<filter_type>::operator*() {
	return *first;
}

template<typename filter_type>
filter_type* MultichannelFilterBank<filter_type>::operator->() {
	return first;
}

template<typename filter_type>
void MultichannelFilterBank<filter_type>::tick(float* inputFrame, float* outputFrame) {
	auto t = first;
	int i = 0;
	while(t) {
		outputFrame[i] = t->tick(inputFrame[i]);
		i++;
		t = t->getSlave();
	}
}

template<typename filter_type>
void MultichannelFilterBank<filter_type>::process(int blockSize, float** inputs, float** outputs) {
	auto f = first;
	int i = 0;
	while(f) {
		for(int j = 0; j < blockSize; j++) outputs[i][j] = f->tick(inputs[i][j]);
		f = f->getSlave();
		i++;
	}
}

template<typename filter_type>
template<typename CallableT, typename... ArgsT>
void MultichannelFilterBank<filter_type>::process(int blockSize, float** inputs, float** outputs, CallableT callable, ArgsT... args) {
	auto f = first;
	int i = 0;
	while(f) {
		//We temporarily unhook the slave.
		auto slave = f->getSlave();
		f->setSlave(nullptr);
		for(int j = 0; j < blockSize; j++) {
			callable(*f, j, args...);
			outputs[i][j] = f->tick(inputs[i][j]);
		}
		i++;
		//Put the slave back and advance.
		f->setSlave(slave);
		f = f->getSlave();
	}
}

}