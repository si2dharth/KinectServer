#pragma once
#include "KinectProvider.h"
#include <set>
using namespace std;

class BodyFrameProvider {
	KinectProvider *_kinect;
	bool trackState[BODY_COUNT];
	Joint joints[BODY_COUNT][JointType_Count];
	HandState handStates[BODY_COUNT][2];
	int lastJoint = BODY_COUNT;
	bool lastLeftState[BODY_COUNT], lastRightState[BODY_COUNT];
public:
	BodyFrameProvider(KinectProvider *KP);
	~BodyFrameProvider();
	void updateFrame();
	int getNumberOfBodies();
	set<int> getBodyIndices();
	Joint getJoint(int bodyIndex, JointType jointType);
	bool getLeftHandClosed(int bodyIndex);
	bool getRightHandClosed(int bodyIndex);
};