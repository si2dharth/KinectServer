#include "stdafx.h"
#include "KinectAudioStream.h"
#include "SpeechHandler.h"

// For FORMAT_WaveFormatEx and such
#include <uuids.h>

// For speech APIs
// NOTE: To ensure that application compiles and links against correct SAPI versions (from Microsoft Speech
//       SDK), VC++ include and library paths should be configured to list appropriate paths within Microsoft
//       Speech SDK installation directory before listing the default system include and library directories,
//       which might contain a version of SAPI that is not appropriate for use together with Kinect sensor.
#include <sapi.h>
__pragma(warning(push))
__pragma(warning(disable:6385 6001)) // Suppress warnings in public SDK header
#include <sphelper.h>
__pragma(warning(pop))

#define INITGUID
#include <guiddef.h>

#include <iostream>
using namespace std;

DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);

void run();
void ProcessSpeech(ISpRecoContext *speechContext);

int main() {
	if (CLSID_ExpectedRecognizer != CLSID_SpInprocRecognizer)
		return EXIT_FAILURE;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (SUCCEEDED(hr)) {
		run();
		CoUninitialize();
	}

	return EXIT_SUCCESS;
}

void run() {
	KinectProvider kinect;
	SpeechProvider sp(&kinect);
	sp.addToGrammar("forward");
	sp.addToGrammar("backward");
	sp.addToGrammar("side");
	sp.addToGrammar("Hello world");
	while (true) {
		string sw = sp.getSpokenWord();
		if (sw != "") cout << sw << endl;
	}
}