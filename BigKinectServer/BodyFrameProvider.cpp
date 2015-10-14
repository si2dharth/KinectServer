#include "BodyFrameProvider.h"

BodyFrameProvider::BodyFrameProvider(KinectProvider *KP){
	_kinect = KP;
	_kinect->startBodyDataCapture();
}

BodyFrameProvider::~BodyFrameProvider() {
	_kinect->stopBodyDataCapture();
}

void BodyFrameProvider::updateFrame() {
	while (_kinect->getBodyData(trackState, joints,handStates) != KinectProvider::result::OK);
}

int BodyFrameProvider::getNumberOfBodies() {
	if (joints == nullptr) return 0;
	int res = 0;
	for (int i = 0; i < BODY_COUNT; i++) {
		if (trackState[i])
			 res++;
	}
	return res;
}

set<int> BodyFrameProvider::getBodyIndices() {
	set<int> res;
	for (int i = 0; i < BODY_COUNT; i++) {
		if (trackState[i])
			res.insert(i);
	}
	return res;
}

Joint BodyFrameProvider::getJoint(int bodyIndex, JointType jointType) {
	if (joints[bodyIndex] == 0) {
		Joint J;
		J.TrackingState = TrackingState_NotTracked;
		return J;
	}
	return joints[bodyIndex][jointType];
}

bool BodyFrameProvider::getLeftHandClosed(int bodyIndex) {
	HandState leftState = handStates[bodyIndex][0];
	if (leftState == HandState_Open || leftState == HandState_Closed)
		lastLeftState[bodyIndex] = leftState == HandState_Closed;
	return lastLeftState[bodyIndex];
}

bool BodyFrameProvider::getRightHandClosed(int bodyIndex) {
	HandState rightState = handStates[bodyIndex][1];
	if (rightState == HandState_Open || rightState == HandState_Closed)
		lastRightState[bodyIndex] = rightState == HandState_Closed;
	return lastRightState[bodyIndex];
}