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

int cam_enum() {
	// Enumerator
	IMoniker* pMoniker = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;

	HRESULT hr;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// create enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

	// Cam Enumerator
	if (SUCCEEDED(hr)) {
		ULONG cFetched;
		int num = 1;
		while (pEnum->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					//printf("%d : %S\n", num, varName.bstrVal);
					char* name = ConvertWCtoC(varName.bstrVal);
					printf("%d : %s ", num, name);
					delete[] name;
					VariantClear(&varName);
					/*++num;*/
				}
				//To create an instance of the filter, do the following:
				IBaseFilter *pSrc;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pSrc);
			}
			pMoniker->Release();
			pPropBag->Release();
		}
		pEnum->Release();
		pDevEnum->Release();
	}

	return 0;
}