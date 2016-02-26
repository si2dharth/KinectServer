#include "BodyFrameProvider.h"

BodyFrameProvider::BodyFrameProvider(KinectProvider *KP){
	_kinect = KP;
	bool started = false;
	while (!started) {
		try {
			_kinect->startBodyDataCapture();
			started = true;
		}
		catch (...) {

		}
	}
}

BodyFrameProvider::~BodyFrameProvider() {
	_kinect->stopBodyDataCapture();
}

void BodyFrameProvider::updateFrame() {
	while (_kinect->getBodyData(tmp_trackState, tmp_joints, tmp_handStates) != KinectProvider::result::OK) {
		Sleep(10);
	}
}

void BodyFrameProvider::updateData() {
	dataMutex.lock();
	memcpy(trackState, tmp_trackState, sizeof tmp_trackState);
	memcpy(joints, tmp_joints, sizeof tmp_joints);
	memcpy(handStates, tmp_handStates, sizeof tmp_handStates);
	dataMutex.unlock();
}

void BodyFrameProvider::startDataCollection() {
	numUsers++;
	if (numUsers == UPDATE_COUNT)
		collectLock.lock();
	else
		collectLock.try_lock();
}

void BodyFrameProvider::stopDataCollection() {
	numUsers--;
	if (numUsers == 0) {
		updateData();
		collectLock.unlock();
	}
}

int BodyFrameProvider::getNumberOfBodies() {
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