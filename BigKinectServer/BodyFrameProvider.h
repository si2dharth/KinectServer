#pragma once
#include "KinectProvider.h"
#include <mutex>
#include <set>
#include <queue>
#define UPDATE_COUNT 10
using namespace std;

class BodyFrameProvider {
	KinectProvider *_kinect;
	bool trackState[BODY_COUNT];
	Joint joints[BODY_COUNT][JointType_Count];
	HandState handStates[BODY_COUNT][2];

	bool tmp_trackState[BODY_COUNT];
	Joint tmp_joints[BODY_COUNT][JointType_Count];
	HandState tmp_handStates[BODY_COUNT][2];

	int lastJoint = BODY_COUNT;
	bool lastLeftState[BODY_COUNT], lastRightState[BODY_COUNT];

	mutex dataMutex;
	bool storingData = false;
	void updateData();

	int numUsers = 0;
	mutex collectLock;
public:
	BodyFrameProvider(KinectProvider *KP);
	~BodyFrameProvider();
	void updateFrame();

	void startDataCollection();
	void stopDataCollection();
	
	int getNumberOfBodies();
	set<int> getBodyIndices();
	Joint getJoint(int bodyIndex, JointType jointType);
	bool getLeftHandClosed(int bodyIndex);
	bool getRightHandClosed(int bodyIndex);
};