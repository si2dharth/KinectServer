#pragma once
#include "KinectProvider.h"
#include "KinectAudioStream.h"

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

#include <set>
using namespace std;

class SpeechProvider {
	KinectProvider *_kinect;
	KinectAudioStream *kStream;

	ISpRecognizer *speechRecognizer;
	ISpStream *speechStream;
	ISpRecoContext *speechContext;
	ISpRecoGrammar *speechGrammar;
	SPSTATEHANDLE initState;

	bool initialized;

	HANDLE speechEvent;

	set<string> commands;

	void initializeRecognizer();
	void startRecognition();
	void pauseRecognition();

	string processSpeech();
public:
	SpeechProvider(KinectProvider *KP);
	~SpeechProvider();
	void addToGrammar(string command);
	void removeFromGrammar(string command);

	string getSpokenWord();
};