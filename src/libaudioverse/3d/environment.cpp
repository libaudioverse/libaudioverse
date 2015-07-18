/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/3d/source.hpp>
#include <libaudioverse/3d/environment.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/creators.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <vector>

namespace libaudioverse_implementation {

EnvironmentNode::EnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): SubgraphNode(Lav_OBJTYPE_ENVIRONMENT_NODE, simulation)  {
	this->hrtf = hrtf;
	int channels = getProperty(Lav_ENVIRONMENT_OUTPUT_CHANNELS).getIntValue();
	output = createGainNode(simulation);
	output->resize(channels, channels);
	output->appendInputConnection(0, channels);
	output->appendOutputConnection(0, channels);
	appendOutputConnection(0, channels);
	setOutputNode(output);
	environment_info.world_to_listener_transform = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

std::shared_ptr<EnvironmentNode> createEnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf) {
	auto retval = std::shared_ptr<EnvironmentNode>(new EnvironmentNode(simulation, hrtf), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void EnvironmentNode::willProcessParents() {
	if(werePropertiesModified(this, Lav_3D_POSITION, Lav_3D_ORIENTATION)) {
		//update the matrix.
		//Important: look at the glsl constructors. Glm copies them, and there is nonintuitive stuff here.
		const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
		const float* atup = getProperty(Lav_3D_ORIENTATION).getFloat6Value();
		auto at = glm::vec3(atup[0], atup[1], atup[2]);
		auto up = glm::vec3(atup[3], atup[4], atup[5]);
		auto right = glm::cross(at, up);
		auto m = glm::mat4(
		right.x, up.x, -at.x, 0,
		right.y, up.y, -at.y, 0,
		right.z, up.z, -at.z, 0,
		0, 0, 0, 1);
		//Above is a rotation matrix, which works presuming the player is at (0, 0).
		//Pass the translation through it, so that we can bake the translation in.
		auto posvec = m*glm::vec4(pos[0], pos[1], pos[2], 1.0f);
		//[column][row] because GLSL.
		m[3][0] = -posvec.x;
		m[3][1] = -posvec.y;
		m[3][2] = -posvec.z;
		environment_info.world_to_listener_transform = m;
		//this debug code left in case this is still all broken.
		/*printf("\n%f %f %f %f\n", m[0][0], m[1][0], m[2][0], m[3][0]);
		printf("%f %f %f %f\n", m[0][1], m[1][1], m[2][1], m[3][1]);
		printf("%f %f %f %f\n", m[0][2], m[1][2], m[2][2], m[3][2]);
		printf("%f %f %f %f\n\n", m[0][3], m[1][3], m[2][3], m[3][3]);*/
	}
	//give the new environment to the sources.
	//this is a set of weak pointers.
	decltype(sources) needsRemoval; //for source cleanup below.
	for(auto i: sources) {
		auto tmp = i.lock();
		if(tmp == nullptr) {
			needsRemoval.insert(i);
			continue;
		}
		tmp->update(environment_info);
	}
	//do cleanup of dead sources.
	for(auto i: needsRemoval) sources.erase(i);
}

std::shared_ptr<Node> EnvironmentNode::createPannerNode() {
	auto pan = createMultipannerNode(simulation, hrtf);
	pan->connect(0, output, 0);
	return pan;
}

void EnvironmentNode::registerSourceForUpdates(std::shared_ptr<SourceNode> source) {
	sources.insert(source);
	simulation->invalidatePlan();
}

void EnvironmentNode::willTick() {
	if(werePropertiesModified(this, Lav_ENVIRONMENT_OUTPUT_CHANNELS)) {
		int channels = getProperty(Lav_ENVIRONMENT_OUTPUT_CHANNELS).getIntValue();
		output->resize(channels, channels);
		getOutputConnection(0)->reconfigure(0, channels);
		output->getOutputConnection(0)->reconfigure(0, channels);
		output->getInputConnection(0)->reconfigure(0, channels);
	}
}

void EnvironmentNode::visitDependencies(std::function<void(std::shared_ptr<Job>&)> &pred) {
	SubgraphNode::visitDependencies(pred);
	//Other dependencies: all our sources.
	for(auto w: sources) {
		auto n = w.lock();
		if(n) {
			auto j = std::static_pointer_cast<Job>(n);
			pred(j);
		}
	}
}

void EnvironmentNode::playAsync(std::shared_ptr<Buffer> buffer, float x, float y, float z) {
	auto e = std::static_pointer_cast<EnvironmentNode>(shared_from_this());
	auto s = createSourceNode(simulation, e);
	auto b = createBufferNode(simulation);
	b->getProperty(Lav_BUFFER_BUFFER).setBufferValue(buffer);
	b->connect(0, s, 0);
	s->getProperty(Lav_3D_POSITION).setFloat3Value(x, y, z);
	//The key here is that we capture the shared pointers, holding them until the event fires.
	//When the event fires, we null the pointers we captured, and then everything schedules for deletion.
	b->getEvent(Lav_BUFFER_END_EVENT).setHandler([b, e, s] (Node* unused1, void* unused2) mutable {
		if(b) b->disconnect(0);
		b.reset();
		s.reset();
		e.reset();
	});
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createEnvironmentNode(LavHandle simulationHandle, const char*hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = std::make_shared<HrtfData>();
	if(std::string(hrtfPath) != "default") {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	} else {
		hrtf->loadFromDefault(simulation->getSr());
	}
	auto retval = createEnvironmentNode(simulation, hrtf);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_environmentNodePlayAsync(LavHandle nodeHandle, LavHandle bufferHandle, float x, float y, float z) {
	PUB_BEGIN
	auto e = incomingObject<EnvironmentNode>(nodeHandle);
	auto b = incomingObject<Buffer>(bufferHandle);
	LOCK(*e);
	e->playAsync(b, x, y, z);
	PUB_END
}

}