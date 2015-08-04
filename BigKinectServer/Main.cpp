#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	KinectProvider kinect;
	BodyFrameProvider bodies(&kinect);

	while (1) {
		bodies.updateFrame();
		set<int> bds = bodies.getBodyIndices();
		if (bds.size() == 0) continue;
		Joint right = bodies.getJoint(*bds.begin(),JointType_HandRight);
		SetCursorPos(right.Position.X * 1200 - 600, -right.Position.Y * 800 + 400);
	}
}