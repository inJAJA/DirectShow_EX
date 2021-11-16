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

int cam_img_cap() {
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

	// Added for the SampleGrabber
	ISampleGrabber *pSampleGrabber = NULL;
	IBaseFilter *pSgrabberF = NULL;
	IPin *pSrcPin = NULL;
	IPin *pGrabPin = NULL;
	AM_MEDIA_TYPE am_media_type;

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

	//Create SampleGrabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (LPVOID *)&pSgrabberF);

	// Get interfaces
	hr = pSgrabberF->QueryInterface(IID_ISampleGrabber, (LPVOID *)&pSampleGrabber);
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pME);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVW);

	// create enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

	// Cam Bind ( use first cam ) 
	hr = pEnum->Next(1, &pMoniker, NULL);
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pSrc);

	// Add Filter
	hr = pGraph->AddFilter(pSrc, L"Video Capture");
	hr = pGraph->AddFilter(pSgrabberF, L"Sample Grabber");
	hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer");

	hr = pGraph->Connect(pSrcPin, pGrabPin);

	// RenderStream
	hr = pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSrc, NULL, NULL);

	//SampleGrabber
	ZeroMemory(&am_media_type, sizeof(am_media_type));
	am_media_type.majortype = MEDIATYPE_Video;
	am_media_type.subtype = MEDIASUBTYPE_RGB24;
	am_media_type.formattype = FORMAT_VideoInfo;
	pSampleGrabber->SetMediaType(&am_media_type);

	// Get connection information.
	// This must be done after the Graph is created
	// by RenderFile.
	hr = pSampleGrabber->GetConnectedMediaType(&am_media_type);
	VIDEOINFOHEADER *pVideoInfoHeader =
		(VIDEOINFOHEADER *)am_media_type.pbFormat;

	//// Print the width and height of the image.
	//// This is just to make the sample understandable.
	//// This is not a required feature.
	printf("size = %dx%d\n",
		pVideoInfoHeader->bmiHeader.biWidth,
		pVideoInfoHeader->bmiHeader.biHeight);

	//// Print the data size.
	//// This is just for understanding too.
	printf("sample size = %d\n",
		am_media_type.lSampleSize);

	//// Configure SampleGrabber to do grabbing.
	//// Buffer data can not be obtained if you
	//// do not use SetBufferSamples.
	//// You can use SetBufferSamples after Run() too.
	hr = pSampleGrabber->SetBufferSamples(TRUE);

	// VideoWindow Setting
	// https://docs.microsoft.com/ko-kr/windows/win32/api/control/nf-control-ivideowindow-put_autoshow
	//
	pVW->put_Height(1024);
	pVW->put_Width(1280);
	pVW->put_AutoShow(OATRUE); //OAFALSE

	// Cam Run
	hr = pMC->Run();

	// Block execution
	MessageBox(NULL,
		"Cam Capture",
		"Message",
		MB_OK);

	// BITMAP will be saved after OK is pressed

	// prepare buffer
	long nBufferSize = am_media_type.lSampleSize;
	long *pBuffer = (long *)malloc(nBufferSize);

	// grab image data.
	pSampleGrabber->GetCurrentBuffer(&nBufferSize,
		pBuffer);

	// Save image data as JPG.
	// This is just to make this sample easily understandable.
	//
	HANDLE fh;
	DWORD nWritten;

	string filename = "result.jpg";
	/*char filename[15];
	sprintf(filename, "result", "%d", cam_num, ".jpg");*/

	// save Format
	fh = CreateFile(filename.c_str(),
		GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// JPG
	WriteFile(fh, pBuffer, nBufferSize, &nWritten, NULL); // use malloc

	// Memory Free
	free(pBuffer);

	// Release
	pMC->Release();
	pVW->Release();
	pME->Release();
	pGraph->Release();
	pSampleGrabber->Release();
	pSgrabberF->Release();

	return 0;
}