#include "stdafx.h"
#include <fstream>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	IKinectSensor *KS = nullptr;
	int hr = GetDefaultKinectSensor(&KS);
	if (hr != 0) return 1;
	KS->Open();
	
	IColorFrameSource *ICFS = nullptr;
	hr = KS->get_ColorFrameSource(&ICFS);
	if (hr != 0) return 2;

	IColorFrameReader *ICFR = nullptr;
	hr = ICFS->OpenReader(&ICFR);
	if (hr != 0) return 3;

	ofstream out;
	int i = 0;
	while (i < 5) {
		IColorFrame *ICF = nullptr;
		hr = ICFR->AcquireLatestFrame(&ICF);
		if (hr != 0) {
			Sleep(1);
			continue;
		}
		i++;
		out.open(string("OUTPUT") + to_string(i) + ".raw", ios_base::binary);
		IFrameDescription *IFD = nullptr;
		hr = ICF->get_FrameDescription(&IFD);
		int height, width;
		unsigned int bytesPerPixel;
		IFD->get_BytesPerPixel(&bytesPerPixel);
		IFD->get_Height(&height);
		IFD->get_Width(&width);
		ColorImageFormat CIF;
		hr = ICF->get_RawColorImageFormat(&CIF);
		UINT size = bytesPerPixel * height * width;
		UINT capacity = 0;
		BYTE *raw = nullptr;
		ICF->AccessRawUnderlyingBuffer(&capacity, &raw);
		out.write((char*)raw, capacity);
		//Image is in YUV2 format.
		out.close();
		//delete[] raw;
		ICF->Release();
	}
}