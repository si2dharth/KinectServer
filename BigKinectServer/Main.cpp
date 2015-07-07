#include "stdafx.h"
#include <fstream>
#include "KinectProvider.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	KinectProvider kinect;
	kinect.startColorCapture(true);
	ofstream out;
	
	int i = 0;
	while (i < 5) {
		UINT capacity = 0;
		UINT16 *raw = nullptr;
		if (kinect.getInfraredImage(&raw, capacity) != KinectProvider::result::OK) continue;
		i++;
		out.open(string("OUTPUT") + to_string(i) + ".raw", ios_base::binary);
		out.write((char*)raw, capacity*2);
		//Image is in YUV2 format.
		out.close();
		//delete[] raw;
	}
}