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
	
	while (true) {
		out.open("OUTPUT.bmp");
		IColorFrame *ICF = nullptr;
		hr = ICFR->AcquireLatestFrame(&ICF);
		if (hr != 0) return 4;
		IFrameDescription *IFD = nullptr;
		hr = ICF->get_FrameDescription(&IFD);
		int height, width;
		unsigned int bytesPerPixel;
		IFD->get_BytesPerPixel(&bytesPerPixel);
		IFD->get_Height(&height);
		IFD->get_Width(&width);
		BYTE *frameData = new BYTE[bytesPerPixel * height * width];
		ICF->CopyConvertedFrameDataToArray(bytesPerPixel * height * width, frameData, ColorImageFormat::ColorImageFormat_Rgba);
		for (int i = 0; i < bytesPerPixel * height * width; i++)
			out << frameData[i];
		out.close();
		delete[] frameData;
	}
}