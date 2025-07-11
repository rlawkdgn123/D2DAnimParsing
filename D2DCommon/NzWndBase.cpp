#include "pch.h"
#include <iostream>

LRESULT CALLBACK NzWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	NzWndBase* pNzWnd = reinterpret_cast<NzWndBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	
	if (pNzWnd != nullptr)
	{
		if (pNzWnd->OnWndProc(hwnd, msg, wparam, lparam))
			return 0; 
    }
	switch (msg)
	{		
	case WM_SIZE:
		if (pNzWnd) pNzWnd->OnResize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	
	case WM_CLOSE:
		if (pNzWnd) pNzWnd->OnClose();

		pNzWnd->Destroy();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}//switch

	return 0;
}


bool NzWndBase::Create(const wchar_t* className, const wchar_t* windowName, int width, int height)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = className;
	wc.lpfnWndProc = NzWndProc; // //[CHECK] #5. 윈도우 프로시저(함수)의 포인터 등록

	
	ATOM classId = 0;
	if (!GetClassInfoEx(HINSTANCE(), className, &wc))
	{
		classId = RegisterClassEx(&wc);

		if (0 == classId) return false;
	}
	
	m_width = width;
	m_height = height;

	RECT rc = { 0, 0, width, height };
	//AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	//[2025-04-22] 리사이즈/최대화 막음
	AdjustWindowRect(&rc,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

	//[CHECK] AdjustWindowRect()의 의미는?
	m_hWnd = CreateWindowEx(NULL, MAKEINTATOM(classId), L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, HWND(), HMENU(), HINSTANCE(), NULL);

	

	if (NULL == m_hWnd) return false;

	::SetWindowText((HWND)m_hWnd, windowName);

	
	SetWindowLongPtr((HWND)m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow((HWND)m_hWnd, SW_SHOW);
	UpdateWindow((HWND)m_hWnd);

	return true;
}

void NzWndBase::Destroy()
{
	if (NULL != m_hWnd)
	{
        DestroyWindow((HWND)m_hWnd);
        m_hWnd = NULL;
    }
}

void NzWndBase::OnResize(int width, int height)
{
	m_width = width;
	m_height = height;
}
