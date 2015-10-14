#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"
#include "Server.h"
#include "KinectThread.h"
#include <vector>
#include <iostream>

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

void connectedEvent(Client *C) {
	
}

//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
int main(int nargs, char **args){
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
	MultiClientTCPServer jointMapServer(10005, BodyServer);
	

	while (true) {
		Sleep(10);
		
	}
	Gdiplus::GdiplusShutdown(gplus);
}