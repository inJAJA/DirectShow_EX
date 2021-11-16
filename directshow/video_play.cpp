#include <stdio.h>
#include <iostream>

#include <dshow.h>

#include <atlconv.h>

#include <windows.h>
#pragma comment(lib, "strmiids")

using namespace std;

int video_play()
{
	BSTR filename = SysAllocString(L"..\\data\\NY.avi"); // const char ro BSTR

	IGraphBuilder *pGraphBuilder;
	IMediaControl *pMediaControl;


	// initialize COM
	CoInitialize(NULL);

	// create FilterGraph
	CoCreateInstance(CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC,
		IID_IGraphBuilder,
		(LPVOID *)&pGraphBuilder);


	// get MediaControl
	pGraphBuilder->QueryInterface(IID_IMediaControl,
		(LPVOID *)&pMediaControl);

	// create Graph.
	// Graph that contains SampleGrabber
	// will be created automatically.
	HRESULT hr = pGraphBuilder->RenderFile(filename, 0);	// supported : ASF, WMA, WMV, AIFF, AU, AVI, MIDI, SND, WAV, ACC
	if (FAILED(hr)) {
		cout << "Error : " << hr << endl;
	}

	// Start playing
	pMediaControl->Run();

	// Block execution
	MessageBox(NULL,
		"Video Run",
		"Message",
		MB_OK);

	// Release
	pMediaControl->Release();
	pGraphBuilder->Release();

	// finish COM
	CoUninitialize();

	return 0;
}