#pragma once
#include "KinectProvider.h"
#include <set>
using namespace std;

class BodyFrameProvider {
	KinectProvider *_kinect;
	IBody *bodies[BODY_COUNT];
	Joint joints[JointType_Count];
	int lastJoint = BODY_COUNT;
public:
	BodyFrameProvider(KinectProvider *KP);
	~BodyFrameProvider();
	void updateFrame();
	int getNumberOfBodies();
	set<int> getBodyIndices();
	Joint getJoint(int bodyIndex, JointType jointType);
};