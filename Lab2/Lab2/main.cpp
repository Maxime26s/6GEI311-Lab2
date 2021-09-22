// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include <new>
#include <windows.h>
#include <dshow.h>
#include "playback.h"

#include <Python.h>

#include<conio.h>

DShowPlayer* g_pPlayer = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK OnGraphEvent(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);
void OnChar(HWND hwnd, wchar_t c);
void OnFileOpen(HWND hwnd);
void OnPaint(HWND hwnd);
void OnSize(HWND hwnd);
void NotifyError(HWND hwnd, PCWSTR pszMessage);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		NotifyError(NULL, L"CoInitializeEx failed.");
		return 0;
	}

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"DirectShow Playback",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		NotifyError(NULL, L"CreateWindowEx failed.");
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CHAR:
		OnChar(hwnd, (wchar_t)wParam);
		return 0;

	case WM_CREATE:
		g_pPlayer = new (std::nothrow) DShowPlayer(hwnd);
		if (g_pPlayer == NULL)
		{
			return -1;
		}
		return 0;

	case WM_DESTROY:
		delete g_pPlayer;
		PostQuitMessage(0);
		return 0;

	case WM_DISPLAYCHANGE:
		g_pPlayer->DisplayModeChanged();
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		OnPaint(hwnd);
		return 0;

	case WM_SIZE:
		OnSize(hwnd);
		return 0;

	case WM_GRAPH_EVENT:
		g_pPlayer->HandleGraphEvent(OnGraphEvent);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hwnd, &ps);

	if (g_pPlayer->State() != STATE_NO_GRAPH && g_pPlayer->HasVideo())
	{
		// The player has video, so ask the player to repaint. 
		g_pPlayer->Repaint(hdc);
	}
	else
	{
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
	}

	EndPaint(hwnd, &ps);
}

void OnSize(HWND hwnd)
{
	if (g_pPlayer)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		g_pPlayer->UpdateVideoWindow(&rc);
	}
}

void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
	switch (evCode)
	{
	case EC_COMPLETE:
	case EC_USERABORT:
		g_pPlayer->Stop();
		break;

	case EC_ERRORABORT:
		NotifyError(hwnd, L"Playback error.");
		g_pPlayer->Stop();
		break;
	}
}

void OnChar(HWND hwnd, wchar_t c)
{
	switch (c)
	{
	case L'o':
	case L'O':
		OnFileOpen(hwnd);
		break;

	case L'p':
	case L'P':
		if (g_pPlayer->State() == STATE_RUNNING)
		{
			g_pPlayer->Pause();
		}
		else
		{
			g_pPlayer->Play();
		}
		break;
	case L'q':
	case L'Q':
		exit(0);
		break;
	case L'a':
	case L'A':
		g_pPlayer->FastForward();
		break;
	case L'r':
	case L'R':
		g_pPlayer->Back();
		break;
	}

}

void OnFileOpen(HWND hwnd)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	WCHAR szFileName[MAX_PATH];
	szFileName[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = L"All (*.*)/0*.*/0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;

	HRESULT hr;

	if (GetOpenFileName(&ofn))
	{
		hr = g_pPlayer->OpenFile(szFileName);

		InvalidateRect(hwnd, NULL, FALSE);

		if (SUCCEEDED(hr))
		{
			// If this file has a video stream, notify the video renderer 
			// about the size of the destination rectangle.
			OnSize(hwnd);
		}
		else
		{
			NotifyError(hwnd, TEXT("Cannot open this file."));
		}
	}
}

void NotifyError(HWND hwnd, PCWSTR pszMessage)
{
	MessageBox(hwnd, pszMessage, TEXT("Error"), MB_OK | MB_ICONERROR);
}

/* The keyboard capturing function */
void keyboard_capture(void)
{
	char ch = '\0';
	do
	{
		ch = _getch();
		switch (ch)
		{
		case 'H':
		case 'h':
			printf("%c - Help Menu\n", ch);
			printf("%c - Press Q to Quit\n", ch);
			printf("Press O to open media file, write file name with extention"
				"or write the whole path to the file with the extension\n");
			printf("Press R to run the media\n");
			printf("Press P to pause the media\n");
			printf("Press S to stop the media, you will then need to press O and reneter the media path\n");
			printf("Press U for volume up\n");
			printf("Press D for volume down\n");
			printf("Press M for volume mute/unmute up\n");
			printf("Press {,[ for rewind or },] for fast forward, works only in  \"Paused mode\" \n");
			break;
			break;
		case L'o':
		case L'O':
			//OnFileOpen(hwnd);
			break;

		case L'p':
		case L'P':
			if (g_pPlayer->State() == STATE_RUNNING)
			{
				g_pPlayer->Pause();
			}
			else
			{
				g_pPlayer->Play();
			}
			break;
		case L'q':
		case L'Q':
			exit(0);
			break;
		case L'a':
		case L'A':
			g_pPlayer->FastForward();
			break;
		case L'r':
		case L'R':
			g_pPlayer->Back();
			break;
		}
	} while (ch != 'q');



	if (ch == 'q')// if we are qutting by pressing 'q'
	{
		delete g_pPlayer;

		// UnInitialize the COM Library
		CoUninitialize();
	}

}

static PyObject* viewPlayer(PyObject* self, PyObject* args)
{
	int value = wWinMain(GetModuleHandle(NULL), GetModuleHandle(NULL), PWSTR(), 1);

	HANDLE hThread[2];
	DWORD dwID[2];
	DWORD dwRetVal = 0;

	hThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)keyboard_capture,
		NULL, 0, &dwID[0]);
	//Create second thread for playback
	hThread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wWinMain,
		NULL, 0, &dwID[1]);
	//Wait for threads to complete
	dwRetVal = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	PyObject* python_val = Py_BuildValue("");

	return python_val;
}

static PyMethodDef VideoPlayerMethods[] = {
	{"viewPlayer", (PyCFunction)viewPlayer, METH_NOARGS, "Shows the video player."},
	{nullptr, nullptr, 0, nullptr}        /* Sentinel */
};

static struct PyModuleDef videoplayermodule = {
	PyModuleDef_HEAD_INIT,
	"Lab2",   /* name of module */
	nullptr, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
	VideoPlayerMethods
};

PyMODINIT_FUNC PyInit_Lab2(void)
{
	return PyModule_Create(&videoplayermodule);
}

//sys.path.append("C:\\Users\\msimard104\\Desktop\\Lab2\\6GEI311-Lab2\\Lab2\\x64\\Debug")