#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <dshow.h>
#include "qedit.h" // for SampleGrabber
#include <Windows.h>
#include <atlconv.h>

#include <Wincodec.h>
#include <stdio.h>
#include <fstream>

#include <vector>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Windowscodecs.lib")

using namespace std;

static char * ConvertWCtoC(wchar_t* str)
{
	//반환할 char* 변수 선언
	char* pStr;

	//입력받은 wchar_t 변수의 길이를 구함
	int strSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	//char* 메모리 할당
	pStr = new char[strSize];

	//형 변환 
	WideCharToMultiByte(CP_ACP, 0, str, -1, pStr, strSize, 0, 0);
	return pStr;
}

int cam_play() {
	// NullRender
	IBaseFilter *pNullRenderer = NULL;

	// Added for the ICaptureGraphBuilder2
	IGraphBuilder  *pGraph;
	ICaptureGraphBuilder2 *pCapture;

	IBaseFilter *pSrc = NULL;
	IVideoWindow  * pVW = NULL;
	IMediaControl * pMC = NULL;
	IMediaEventEx * pME = NULL;

	// Enumerator
	IMoniker* pMoniker = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;

	HRESULT hr;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);


	// NullRender
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void **)&pNullRenderer);

	// Create Filtergraph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);

	// Create CaptureGraphBuilder2
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
		(void **)&pCapture);

	// Initialize the Capture Graph Builder
	hr = pCapture->SetFiltergraph(pGraph);

	// Get interfaces
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pME);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVW);

	// create enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

	// Cam Bind
	hr = pEnum->Next(1, &pMoniker, NULL);
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pSrc);

	// Add Filter
	hr = pGraph->AddFilter(pSrc, L"Video Capture");
	hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer");

	// RenderStream
	hr = pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSrc, NULL, NULL);

	// Cam Run
	hr = pMC->Run();

	// Block execution
	MessageBox(NULL,
		"Cam Run",
		"Message",
		MB_OK);

	// Release
	pMC->Release();
	pVW->Release();
	pME->Release();
	pGraph->Release();

	return 0;
}