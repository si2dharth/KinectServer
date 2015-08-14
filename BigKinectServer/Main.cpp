#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"
#include "Server.h"
#include "KinectThread.h"

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	ULONG_PTR gplus;
	calConv();
	Gdiplus::GdiplusStartupInput gInp;
	gInp.GdiplusVersion = 1;
	gInp.SuppressBackgroundThread = FALSE;
	gInp.SuppressExternalCodecs = FALSE;
	Gdiplus::GdiplusStartup(&gplus,&gInp,0);
	KinectProvider kinect;
	initAll(&kinect);
	MultiClientTCPServer colorServer(10001, ColorImageServer);
	MultiClientTCPServer infraredServer(10002, InfraredImageServer);
	MultiClientTCPServer depthMapServer(10003, DepthMapServer);
	MultiClientTCPServer bodyMapServer(10004, BodyMapServer);

	

	while (true);
	Gdiplus::GdiplusShutdown(gplus);
}