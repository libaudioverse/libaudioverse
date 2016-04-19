/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <vector>

namespace libaudioverse_implementation {

class NestedAllpassNetwork;

/**Types of ASST node.*/
enum class NestedAllpassNetworkASTTypes {
	IDENTITY, READER, ALLPASS, ONE_POLE, NESTED_ALLPASS, BIQUAD,
};

/**Uses type erasure to run filters.
The actual class is constructed and destructed by the NestedAllpassNetwork directly.*/
class NestedAllpassNetworkASTNode {
	public:
	NestedAllpassNetworkASTNode(NestedAllpassNetwork* source, NestedAllpassNetworkASTTypes type, void* filter);
	//Destructor only deletes our filter, not child nodes.
	~NestedAllpassNetworkASTNode();
	float tick(float input);
	//Ticks us and all our nexts.
	//Returns the rightmost tick in the row.
	float tickRow(float input);
	void reset();
	NestedAllpassNetwork* source = nullptr;
	NestedAllpassNetworkASTTypes type;
	void* filter = nullptr; //Depends on the type.
	//Nested is the one that's nested inside, next is the next one on this level.
	NestedAllpassNetworkASTNode *nested = nullptr, *next = nullptr;
	//Used for the reader.
	float reader_mul = 1.0f;
};

/**This class builds networks of nested allpasses and lowpasses, most commonly used in Schroeder reverb designs.

To use this class, you call vaerious functions that introduce elements or change the nesting level.  When done, you call compile to produce the network.*/
class NestedAllpassNetwork {
	public:
	NestedAllpassNetwork(float sr);
	~NestedAllpassNetwork();
	//Create an allpass, append it to the current level, and begin nesting inside it.
	//delay is in samples and must be at least one.
	//Further appended filters will be positioned at the end of the allpass's delay line.
	void beginNesting(int delay, float coefficient);
	//End the current nesting level.
	void endNesting();
	//Append an allpass with specified delay and coefficient to the current level.
	//The delay is in samples and must be at least one.  The internal delay lines can't handle delays of 0.
	void appendAllpass(int delay, float coefficient);
	//Append a one-pole lowpass or highpass (rolloff of 6 db) to the current level.
	//Specify -3db frequency.
	void appendOnePole(float frequency, bool isHighpass = false);
	//Append a biquad to the current level.  Type is a Lav_BIQUAD_TYPES value.
	void appendBiquad(int type, double frequency, double dbGain, double q);
	//Append a reader.
	//Readers sum the output of whatever is before/above them into the final output.
	//If there are no readers, the output is silent.
	void appendReader(float mul);
	//Compile the network and begin a new one.
	void compile();
	//Standard filter stuff.
	float tick(float input);
	void reset();
	NestedAllpassNetwork* getSlave();
	void setSlave(NestedAllpassNetwork* s);
	
	//called by readers.
	void contribute(float amount);
	private:
	void hookupAST(NestedAllpassNetworkASTNode* node);
	float sr, next_output;
	//Used to push and pop nesting levels, etc.
	std::vector<NestedAllpassNetworkASTNode*> stack;
	//The one we're currently manipulating and the one at the root of the tree we're preparing.
	//Note: current goes to nullptr when we ahve just begun a nesting level.
	//In that case, we have to hook it up via the top of trhe stack.
	NestedAllpassNetworkASTNode* current = nullptr, *next_start = nullptr;
	//The first node in our tree.
	NestedAllpassNetworkASTNode* start = nullptr;
	NestedAllpassNetwork* slave = nullptr;
};

}