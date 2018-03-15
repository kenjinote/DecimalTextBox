#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <algorithm>
#include <string>

TCHAR szClassName[] = TEXT("Window");

class CDecimalTextBox
{
public:
	HWND m_hWnd;
	HWND Create(int x, int y, int width, int height, HWND hParent)
	{
		m_hWnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", 0, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, 0, 0, 0, 0, hParent, 0, GetModuleHandle(0), 0);
		DefaultWndProc = (WNDPROC)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)MyEditProc);
		return m_hWnd;
	}
	static WNDPROC DefaultWndProc;
	static LRESULT CALLBACK MyEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_PASTE:
			{
				if (IsClipboardFormatAvailable(CF_UNICODETEXT))
				{
					if (OpenClipboard(hWnd))
					{
						HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
						if (hClipboardData != NULL)
						{
							LPWSTR lpszClipboardText = (LPWSTR)GlobalLock(hClipboardData);
							if (lpszClipboardText)
							{
								std::wstring string(lpszClipboardText);
								string.erase(std::remove_if(string.begin(), string.end(), [](wchar_t c) { return !isdigit(c) && c != L'.' && c != L'-'; }), string.end());
								const wchar_t *wcs = string.c_str();
								wchar_t *stopwcs;
								wcstod(wcs, &stopwcs);
								string.resize(stopwcs - wcs);
								SendMessageW(hWnd, EM_REPLACESEL, 0, (LPARAM)string.c_str());
								GlobalUnlock(lpszClipboardText);
							}
						}
						CloseClipboard();
					}
				}
			}
			return 0;
		case WM_CHAR:
			{
				DWORD dwPos = (DWORD)SendMessageW(hWnd, EM_GETSEL, NULL, NULL);
				if (wParam == L'-')
				{
					// マイナスは常に先頭以外には入力できなくする
					if (LOWORD(dwPos) == 0)
					{
						HLOCAL hMem = (HLOCAL)SendMessageW(hWnd, EM_GETHANDLE, 0, 0);
						LPWSTR lpszBuf = (LPWSTR)LocalLock(hMem);
						if (lpszBuf[HIWORD(dwPos)] == L'-')
						{
							// 既にマイナスが入力されていた場合は無視する
							LocalUnlock(hMem);
							return 0;
						}
						LocalUnlock(hMem);
						return CallWindowProc(DefaultWndProc, hWnd, msg, wParam, lParam);
					}
				}
				else if (isdigit((int)wParam) || wParam == L'.')
				{
					// マイナスの手前に . や数字を入力できなくする
					HLOCAL hMem = (HLOCAL)SendMessageW(hWnd, EM_GETHANDLE, 0, 0);
					LPWSTR lpszBuf = (LPWSTR)LocalLock(hMem);
					for (int i = HIWORD(dwPos); lpszBuf[i]; ++i)
					{
						if (lpszBuf[i] == L'-')
						{
							LocalUnlock(hMem);
							return 0;
						}
					}
					if (wParam == L'.')
					{
						// 数字の直後以外 . は入力不可にする
						if (HIWORD(dwPos) == 0 || !isdigit(lpszBuf[HIWORD(dwPos) - 1]))
						{
							LocalUnlock(hMem);
							return 0;
						}
						// 2 つ目の . は入力不可にする
						for (int i = 0; lpszBuf[i]; ++i)
						{
							if (lpszBuf[i] == L'.')
							{
								LocalUnlock(hMem);
								return 0;
							}
						}
					}
					LocalUnlock(hMem);
					return CallWindowProc(DefaultWndProc, hWnd, msg, wParam, lParam);
				}
				else if (wParam == VK_DELETE || wParam == VK_BACK) // 削除
				{
					return CallWindowProc(DefaultWndProc, hWnd, msg, wParam, lParam);
				}
				else if (wParam == 0x18 || wParam == 0x16 || wParam == 0x1A || wParam == 0x03) // コピー、切り取り、貼り付け、元に戻す
				{
					return CallWindowProc(DefaultWndProc, hWnd, msg, wParam, lParam);
				}
			}
			return 0;
		default:
			break;
		}
		return CallWindowProc(DefaultWndProc, hWnd, msg, wParam, lParam);
	}
};

WNDPROC CDecimalTextBox::DefaultWndProc;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CDecimalTextBox *pEdit;
	switch (msg)
	{
	case WM_CREATE:
		pEdit = new CDecimalTextBox;
		pEdit->Create(10, 10, 256, 32, hWnd);
		break;
	case WM_SIZE:
		if (pEdit)
		{
			MoveWindow(pEdit->m_hWnd, 10, 10, 256, 32, TRUE);
		}
		break;
	case WM_DESTROY:
		delete pEdit;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("小数の入力のみを受け付けるテキストボックス"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
