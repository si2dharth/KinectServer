#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"
#include "Server.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	KinectProvider kinect;
	initAll(&kinect);
	MultiClientTCPServer colorServer(10001, ColorImageServer);
	MultiClientTCPServer infraredServer(10002, InfraredImageServer);
	MultiClientTCPServer depthMapServer(10003, DepthMapServer);
	MultiClientTCPServer bodyMapServer(10004, BodyMapServer);

	while (true);
}