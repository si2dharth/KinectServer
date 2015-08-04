#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	KinectProvider kinect;
	kinect.startColorCapture();
	kinect.startInfraredCapture();

	for (int i = 0; i < 5; i++) {
		BYTE *raw = nullptr;
		UINT16 *infr = nullptr;
		UINT cap, icap;
		while (kinect.getImage(&raw, cap) != kinect.OK);
		while (kinect.getInfraredImage(&infr, icap) != kinect.OK);
	}
	kinect.stopColorCapture();
	kinect.stopInfraredCapture();
}