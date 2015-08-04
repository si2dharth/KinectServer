#include "BodyFrameProvider.h"

BodyFrameProvider::BodyFrameProvider(KinectProvider *KP) {
	_kinect = KP;
	_kinect->startBodyDataCapture();
}

BodyFrameProvider::~BodyFrameProvider() {
	_kinect->stopBodyDataCapture();
}

void BodyFrameProvider::updateFrame() {
	while (_kinect->getBodyData(bodies) != KinectProvider::result::OK);
}

int BodyFrameProvider::getNumberOfBodies() {
	int res = 0;
	for (int i = 0; i < BODY_COUNT; i++) {
		BOOLEAN tracked = false;
		if (bodies[i]->get_IsTracked(&tracked) == 0)
			if (tracked) res++;
	}
	return res;
}

set<int> BodyFrameProvider::getBodyIndices() {
	set<int> res;
	for (int i = 0; i < BODY_COUNT; i++) {
		BOOLEAN tracked = false;
		if (bodies[i]->get_IsTracked(&tracked) == 0)
			if (tracked) res.insert(i);
	}
	return res;
}

Joint BodyFrameProvider::getJoint(int bodyIndex, JointType jointType) {
	if (lastJoint != bodyIndex) bodies[bodyIndex]->GetJoints(JointType_Count, joints);
	return joints[jointType];
}
