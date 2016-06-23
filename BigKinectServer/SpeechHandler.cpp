#include "SpeechHandler.h"

SpeechProvider::SpeechProvider(KinectProvider *KP) :
	_kinect(KP),
	kStream(nullptr),
	speechRecognizer(nullptr),
	speechStream(nullptr),
	speechContext(nullptr),
	speechGrammar(nullptr),
	initState(0), speechEvent(INVALID_HANDLE_VALUE),
	initialized(false)
{
	_kinect->startAudioCapture();
	IStream *audioStream = nullptr;
	_kinect->getAudioData(0, audioStream);
	kStream = new KinectAudioStream(audioStream);
	initializeRecognizer();
	initialized = true;
	startRecognition();
}

void SpeechProvider::initializeRecognizer() {
	WORD AudioFormat = WAVE_FORMAT_PCM;
	WORD AudioChannels = 1;
	DWORD AudioSamplesPerSecond = 16000;
	DWORD AudioAverageBytesPerSecond = 32000;
	WORD AudioBlockAlign = 2;
	WORD AudioBitsPerSample = 16;
	HRESULT hr;

	WAVEFORMATEX wfxOut = { AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0 };

	hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&speechStream);
	if (hr < 0) throw 1;

	kStream->SetSpeechState(true);
	hr = speechStream->SetBaseStream(kStream, SPDFID_WaveFormatEx, &wfxOut);
	if (hr < 0) throw 1;

	ISpObjectToken *engineToken = nullptr;
	hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&speechRecognizer);
	if (hr < 0) throw 1;

	speechRecognizer->SetInput(speechStream, TRUE);

	hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"Language=409;Kinect=True", NULL, &engineToken);
	if (hr < 0) throw 1;

	speechRecognizer->SetRecognizer(engineToken);
	hr = speechRecognizer->CreateRecoContext(&speechContext);
	if (hr < 0) throw 1;

	hr = speechRecognizer->SetPropertyNum(L"AdaptationOn", 0);
	if (hr < 0) throw 1;

	engineToken->Release();

	//LoadSpeechGrammar
	hr = speechContext->CreateGrammar(1, &speechGrammar);
	if (hr < 0) throw 1;

	speechGrammar->ResetGrammar(SpGetUserDefaultUILanguage());
	speechGrammar->GetRule(L"ROOT", 0, SPRAF_Active | SPRAF_Root | SPRAF_TopLevel | SPRAF_Dynamic, TRUE, &initState);
	//speechGrammar->AddRuleTransition(initState, NULL, initState, 1, NULL);
	//hr = speechGrammar->LoadCmdFromFile(L"grammarFile.xml", SPLO_DYNAMIC);
	if (hr < 0) throw 1;
}


void SpeechProvider::startRecognition() {
	if (!initialized) return;
	if (commands.size()  == 0) return;
	HRESULT hr = speechGrammar->SetGrammarState(SPGS_ENABLED);

	hr = speechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
	if (hr < 0) throw 3;

	hr = speechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
	if (hr < 0) throw 2;


	hr = speechContext->Resume(0);
	if (hr < 0) throw 1;

	hr = speechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
	if (hr < 0) throw 1;
	speechEvent = speechContext->GetNotifyEventHandle();
}

void SpeechProvider::pauseRecognition() {
	if (!initialized) return;
	speechGrammar->SetRuleState(NULL, NULL, SPRS_INACTIVE);
	speechGrammar->SetGrammarState(SPGS_DISABLED);
	speechRecognizer->SetRecoState(SPRST_INACTIVE);
	speechContext->Pause(0);
}

void SpeechProvider::addToGrammar(string command) {
	while (!initialized) {
		Sleep(10);
	}
	pauseRecognition();
	commands.insert(command);
	speechGrammar->AddWordTransition(initState, NULL, convertToWStr(command).c_str(), NULL, SPWT_LEXICAL, 1.0, NULL);
	speechGrammar->Commit(NULL);
	startRecognition();
}

void SpeechProvider::addToGrammar(vector<string> commands) {
	while (!initialized) {
		Sleep(10);
	}
	pauseRecognition();
	for (auto &command : commands) {
		this->commands.insert(command);
		speechGrammar->AddWordTransition(initState, NULL, convertToWStr(command).c_str(), NULL, SPWT_LEXICAL, 1.0, NULL);
	}
	speechGrammar->Commit(NULL);
	startRecognition();
}

void SpeechProvider::setGrammar(queue<string> &grammar, int children, SPSTATEHANDLE curState) {
	for (int i = 0; i < children; i++) {
		int gChildren = stoi(grammar.front());
		grammar.pop();
		string command = grammar.front();
		grammar.pop();

		SPSTATEHANDLE newState = NULL;
		if (gChildren > 0) {
			speechGrammar->CreateNewState(curState, &newState);
			setGrammar(grammar, gChildren, newState);
		}
		speechGrammar->AddWordTransition(initState, newState, convertToWStr(command).c_str(), NULL, SPWT_LEXICAL, 1.0, NULL);
	}
}

void SpeechProvider::setGrammar(string grammarStructure) {
	pauseRecognition();
	
	vector<string> structure = split(grammarStructure);
	queue<string> q;
	for (auto s : structure) {
		q.push(s);
	}
	q.pop();	//Remove first number
	setGrammar(q, stoi(structure[0]), initState);

	startRecognition();
}

void SpeechProvider::removeFromGrammar(string command) {
	while (!initialized) {
		Sleep(10);
	}
	///Inefficient and slow, but there's no other way
	pauseRecognition();
	commands.erase(command);
	speechGrammar->ClearRule(initState);
	for (string s : commands) {
		speechGrammar->AddWordTransition(initState, NULL, convertToWStr(s).c_str(), NULL, SPWT_LEXICAL, 1.0, NULL);
	}
	speechGrammar->Commit(NULL);
	startRecognition();
}

#include <iostream>
void SpeechProvider::setNewGrammar(vector<string> commands, int startIndex) {
	pauseRecognition();
	this->commands.clear();
	speechGrammar->ClearRule(initState);
	cout << "HERE " << startIndex << " " << commands.size() << endl;
	for (int i = startIndex; i < commands.size(); i++) {
		cout << "COMMAND" << endl;
		string &command = commands[i];
		this->commands.insert(command);
		cout << command << endl;
		speechGrammar->AddWordTransition(initState, NULL, convertToWStr(command).c_str(), NULL, SPWT_LEXICAL, 1.0, NULL);
	}
	speechGrammar->Commit(NULL);
	startRecognition();
}

string SpeechProvider::getSpokenWord() {
	if (!initialized) return "";
	DWORD waitResult = WaitForSingleObject(speechEvent, 50);

	if (waitResult == WAIT_OBJECT_0) {
		return processSpeech();
	}
	return "";
}

#include <iostream>
using std::wcout;

string SpeechProvider::processSpeech() {
	const float ConfidenceThreshold = 0.3f;

	SPEVENT curEvent = { SPEI_UNDEFINED, SPET_LPARAM_IS_UNDEFINED, 0, 0, 0, 0 };
	ULONG fetched = 0;
	HRESULT hr = S_OK;

	string fresult;

	speechContext->GetEvents(1, &curEvent, &fetched);
	while (fetched > 0)
	{
		switch (curEvent.eEventId)
		{
		case SPEI_RECOGNITION:
			if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
			{
				// this is an ISpRecoResult
				ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
				SPPHRASE* pPhrase = NULL;

				hr = result->GetPhrase(&pPhrase);
				if (SUCCEEDED(hr))
				{
					wstring s = pPhrase->pElements->pszDisplayText;
					wcout << L"Phrase Spoken : " << s << endl;
					fresult = string(s.begin(), s.end());
					::CoTaskMemFree(pPhrase);
				}
			}
			break;
		}

		speechContext->GetEvents(1, &curEvent, &fetched);
	}
	return fresult;
}

SpeechProvider::~SpeechProvider() {
	pauseRecognition();
	kStream->SetSpeechState(false);
	kStream->Release();
	speechGrammar->Release();
	speechContext->Release();
	speechStream->Release();
	speechRecognizer->Release();
	commands.clear();
}