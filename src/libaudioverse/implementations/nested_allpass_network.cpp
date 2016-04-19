/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/nested_allpass_network.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <libaudioverse/implementations/one_pole_filter.hpp>
#include <libaudioverse/implementations/allpass.hpp>
#include <libaudioverse/implementations/delayline.hpp>
//Get the biquad types:
#include <libaudioverse/libaudioverse_properties.h>
#include <algorithm>
#include <functional>
#include <vector>
#include <math.h>

namespace libaudioverse_implementation {

NestedAllpassNetworkASTNode::NestedAllpassNetworkASTNode(NestedAllpassNetwork* source, NestedAllpassNetworkASTTypes type, void* filter) {
	this->source = source;
	this->type = type;
	this->filter = filter;
}

NestedAllpassNetworkASTNode::~NestedAllpassNetworkASTNode() {
	if(type == NestedAllpassNetworkASTTypes::ALLPASS || type == NestedAllpassNetworkASTTypes::NESTED_ALLPASS) delete (AllpassFilter<InterpolatedDelayLine>*)filter;
	else if(type == NestedAllpassNetworkASTTypes::ONE_POLE) delete (OnePoleFilter*)filter;
	else if(type == NestedAllpassNetworkASTTypes::BIQUAD) delete (BiquadFilter*)filter;
	else if(type == NestedAllpassNetworkASTTypes::BIQUAD) delete (BiquadFilter*)filter;
}

//Tick and reset follow a similar pattern.
float NestedAllpassNetworkASTNode::tick(float input) {
	if(type == NestedAllpassNetworkASTTypes::IDENTITY) return input;
	else if(type == NestedAllpassNetworkASTTypes::ALLPASS) return static_cast<AllpassFilter<InterpolatedDelayLine>*>(filter)->tick(input);
	else if(type == NestedAllpassNetworkASTTypes::ONE_POLE) return static_cast<OnePoleFilter*>(filter)->tick(input);
	else if(type == NestedAllpassNetworkASTTypes::BIQUAD) return static_cast<BiquadFilter*>(filter)->tick(input);
	else if(type == NestedAllpassNetworkASTTypes::NESTED_ALLPASS) {
		float tmp = static_cast<AllpassFilter<InterpolatedDelayLine>*>(filter)->beginNestedTick();
		tmp = nested->tickRow(tmp);
		return static_cast<AllpassFilter<InterpolatedDelayLine>*>(filter)->endNestedTick(input, tmp);
	} 
	//Read the output and add it.
	else if(type == NestedAllpassNetworkASTTypes::READER) {
		source->contribute(input*reader_mul);
		return input;
	}
	return 0.0f;
}

float NestedAllpassNetworkASTNode::tickRow(float input) {
	auto current = this;
	while(current) {
		input = current->tick(input);
		current = current->next;
	}
	return input;
}

void NestedAllpassNetworkASTNode::reset() {
	if(type == NestedAllpassNetworkASTTypes::ALLPASS
	|| type == NestedAllpassNetworkASTTypes::NESTED_ALLPASS) static_cast<AllpassFilter<InterpolatedDelayLine>*>(filter)->reset();
	else if(type == NestedAllpassNetworkASTTypes::BIQUAD) static_cast<BiquadFilter*>(filter)->reset();
	else if(type == NestedAllpassNetworkASTTypes::ONE_POLE) static_cast<OnePoleFilter*>(filter)->reset();
	if(nested) nested->reset();
	if(next) next->reset();
}

//Okay, implement the network itself.
NestedAllpassNetwork::NestedAllpassNetwork(float sr) {
	this->sr = sr;
}

//Small recursive helper function for freeing.
static void freeAST(NestedAllpassNetworkASTNode* start) {
	if(start == nullptr) return;
	NestedAllpassNetworkASTNode *next = start->next, *nested = start->nested;
	freeAST(next);
	freeAST(nested);
}

NestedAllpassNetwork::~NestedAllpassNetwork() {
	freeAST(start);
	freeAST(next_start);
}

void NestedAllpassNetwork::beginNesting(int delay, float coefficient) {
	auto filter = new AllpassFilter<InterpolatedDelayLine>((delay+1)/(double)sr, sr);
	filter->line.setDelayInSamples(delay);
	filter->setCoefficient(coefficient);
	auto node = new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::NESTED_ALLPASS, filter);
	hookupAST(node);
	//Now move current to the stack and kill it.
	stack.push_back(current);
	current = nullptr;
	if(slave) slave->beginNesting(delay, coefficient);
}

void NestedAllpassNetwork::endNesting() {
	//To go back, grab and remove the back of the vector.
	current = stack.back();
	stack.pop_back();
	if(slave) slave->endNesting();
}

//The rest of these follow a very simple pattern: create and configure filter, make node, put node in.
void NestedAllpassNetwork::appendAllpass(int delay, float coefficient) {
	auto f = new AllpassFilter<InterpolatedDelayLine>((delay+1)/(double)sr, sr);
	f->setCoefficient(coefficient);
	f->line.setDelayInSamples(delay);
	auto n = new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::ALLPASS, f);
	hookupAST(n);
	if(slave) slave->appendAllpass(delay, coefficient);
}

void NestedAllpassNetwork::appendOnePole(float frequency, bool isHighpass) {
	auto f = new OnePoleFilter(sr);
	f->setPoleFromFrequency(frequency, isHighpass);
	auto n = new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::ONE_POLE, f);
	hookupAST(n);
	if(slave) slave->appendOnePole(frequency, isHighpass);
}

void NestedAllpassNetwork::appendBiquad(int type, double frequency, double dbGain, double q) {
	auto f = new BiquadFilter(sr);
	f->configure(type, frequency, dbGain, q);
	auto n =  new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::BIQUAD, f);
	hookupAST(n);
	if(slave) slave->appendBiquad(type, frequency, dbGain, q);
}	

void NestedAllpassNetwork::appendReader(float mul) {
	auto n = new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::READER, nullptr);
	n->reader_mul = mul;
	hookupAST(n);
	if(slave) slave->appendReader(mul);
}

void NestedAllpassNetwork::compile() {
	//If the stack has things on it and the current node is nullptr, we just began nesting.
	//In this case,we apend an identity node to the node at the back of the stack.
	if(current == nullptr && stack.size()) {
		auto n = new NestedAllpassNetworkASTNode(this, NestedAllpassNetworkASTTypes::IDENTITY, nullptr);
		stack.back()->nested = n;
	}
	//start becomes next_start, clear everything else.
	start = next_start;
	current = nullptr;
	next_start = nullptr;
	stack.clear();
	if(slave) slave->compile();
}

void NestedAllpassNetwork::hookupAST(NestedAllpassNetworkASTNode *node) {
	if(current == nullptr && stack.empty()) {
		//We just began a new network.
		//The next start is this one.
		next_start = node;
		current = node;
	}
	else if(current == nullptr) {
		//We have something on the stack and no current; we just began some nesting.
		stack.back()->nested = node;
		current = node;
	}
	else {
		//Current isn't null, this is the next node in the list.
		current->next = node;
		current = node;
	}
}

void NestedAllpassNetwork::contribute(float amount) {
	next_output += amount;
}

float NestedAllpassNetwork::tick(float input) {
	next_output = 0.0f;
	if(start) start->tickRow(input);
	return next_output;
}

void NestedAllpassNetwork::reset() {
	if(start) start->reset();
}

NestedAllpassNetwork* NestedAllpassNetwork::getSlave() {
	return slave;
}

void NestedAllpassNetwork::setSlave(NestedAllpassNetwork* s) {
	slave = s;
}

}