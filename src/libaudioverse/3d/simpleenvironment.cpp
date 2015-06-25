/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/3d/sources.hpp>
#include <libaudioverse/3d/simpleenvironment.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/creators.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <vector>

namespace libaudioverse_implementation {

SimpleEnvironmentNode::SimpleEnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): EnvironmentBase(Lav_OBJTYPE_SIMPLE_ENVIRONMENT_NODE, simulation)  {
	this->hrtf = hrtf;
	int channels = getProperty(Lav_ENVIRONMENT_OUTPUT_CHANNELS).getIntValue();
	output = createGainNode(simulation);
	output->resize(channels, channels);
	output->appendInputConnection(0, channels);
	output->appendOutputConnection(0, channels);
	appendOutputConnection(0, channels);
	setOutputNode(output);
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

std::shared_ptr<SimpleEnvironmentNode> createSimpleEnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf) {
	auto retval = std::shared_ptr<SimpleEnvironmentNode>(new SimpleEnvironmentNode(simulation, hrtf), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void SimpleEnvironmentNode::willProcessParents() {
	if(werePropertiesModified(this, Lav_ENVIRONMENT_OUTPUT_CHANNELS)) outputChannelsChanged();
	//update the matrix.
	const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
	const float* atup = getProperty(Lav_3D_ORIENTATION).getFloat6Value();
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(pos[0], pos[1], pos[2]),
		//this is a point in 3d space, not a unit vector indicating direction.
		glm::vec3(pos[0]+atup[0], pos[1]+atup[1], pos[2]+atup[2]),
		glm::vec3(atup[3], atup[4], atup[5]));

	//give the new environment to the sources.
	//this is a set of weak pointers.
	decltype(sources) needsRemoval; //for source cleanup below.
	for(auto i: sources) {
		auto tmp = i.lock();
		if(tmp == nullptr) {
			needsRemoval.insert(i);
			continue;
		}
		tmp->update(environment);
	}
	//do cleanup of dead sources.
	for(auto i: needsRemoval) sources.erase(i);
}

std::shared_ptr<Node> SimpleEnvironmentNode::createPannerNode() {
	auto pan = createMultipannerNode(simulation, hrtf);
	pan->connect(0, output, 0);
	return pan;
}

void SimpleEnvironmentNode::registerSourceForUpdates(std::shared_ptr<SourceNode> source) {
	sources.insert(source);
}

void SimpleEnvironmentNode::outputChannelsChanged() {
	int channels = getProperty(Lav_ENVIRONMENT_OUTPUT_CHANNELS).getIntValue();
	output->resize(channels, channels);
	getOutputConnection(0)->reconfigure(0, channels);
	output->getOutputConnection(0)->reconfigure(0, channels);
	output->getInputConnection(0)->reconfigure(0, channels);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSimpleEnvironmentNode(LavHandle simulationHandle, const char*hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = std::make_shared<HrtfData>();
	if(std::string(hrtfPath) != "default") {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	} else {
		hrtf->loadFromDefault(simulation->getSr());
	}
	auto retval = createSimpleEnvironmentNode(simulation, hrtf);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}