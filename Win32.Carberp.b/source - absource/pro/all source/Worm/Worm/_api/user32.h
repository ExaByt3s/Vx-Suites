
//@S
HANDLE _USER32=0;

int     (WINAPI *_MessageBox)(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
LRESULT (WINAPI *_CallWindowProc)(WNDPROC lpPrevWndFunc,HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);
LRESULT (WINAPI *_DefWindowProc)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);
HWND    (WINAPI *_CreateWindowEx)(DWORD dwExStyle,LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
BOOL    (WINAPI *_DestroyWindow)(HWND hWnd);	
LONG    (WINAPI *_DispatchMessage)(CONST MSG *lpmsg);	
int     (WINAPI *_GetClassName)(HWND hWnd,LPTSTR lpClassName,int nMaxCount);
HWND    (WINAPI *_GetForegroundWindow)(VOID);
BOOL    (WINAPI *_GetMessage)(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax);	
HWND    (WINAPI *_GetWindow)(HWND hWnd,UINT uCmd);
LONG    (WINAPI *_GetWindowLong)(HWND hWnd,int nIndex); 
BOOL    (WINAPI *_GetWindowRect)(HWND hWnd,LPRECT lpRect);
int     (WINAPI *_GetWindowText)(HWND hWnd,LPTSTR lpString,int nMaxCount);	
BOOL    (WINAPI *_MoveWindow)(HWND hWnd,int X,int Y,int nWidth,int nHeight,BOOL bRepaint);
ATOM    (WINAPI *_RegisterClass)(CONST WNDCLASS *lpWndClass); 
LRESULT (WINAPI *_SendMessage)(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);
HWND    (WINAPI *_SetFocus)(HWND hWnd);
LONG    (WINAPI *_SetWindowLong)(HWND hWnd,int nIndex,LONG dwNewLong);
BOOL    (WINAPI *_SetWindowText)(HWND hWnd,LPCTSTR lpString);
BOOL    (WINAPI *_ShowWindow)(HWND hWnd,int nCmdShow);
BOOL    (WINAPI *_TranslateMessage)(CONST MSG *lpMsg);
HWND    (WINAPI *_FindWindowEx)(HWND hwndParent,HWND hwndChildAfter,LPCTSTR lpszClass,LPCTSTR lpszWindow);
BOOL    (WINAPI *_EnumDesktopWindows)(HDESK hDesktop,WNDENUMPROC lpfn,LPARAM lParam);
BOOL    (WINAPI *_OemToChar)(LPCSTR lpszSrc,LPTSTR lpszDst);
DWORD   (WINAPI *_CharUpperBuff)(LPTSTR lpsz,DWORD cchLength);
//@E

//UINT    WINAPI (*_SetTimer)(HWND hWnd,UINT nIDEvent,UINT uElapse,TIMERPROC lpTimerFunc);
//HCURSOR WINAPI (*_LoadCursor)(HINSTANCE hInstance,LPCTSTR lpCursorName);
//HICON   WINAPI (*_LoadIcon)(HINSTANCE hInstance,LPCTSTR lpIconName);

/*DWORD _GPAecoU32(char *fname) // Economy Code
{
 DWORD a=(DWORD)_GetProcAddress(_USER32,fname);
 return a;
}*/

void  *padrU32[] =                  {&_CallWindowProc, &_CreateWindowEx, &_DefWindowProc, &_DestroyWindow,&_DispatchMessage, &_GetClassName, &_GetForegroundWindow,&_GetMessage, &_GetWindow,&_GetWindowLong, &_GetWindowRect,&_GetWindowText, &_MessageBox, &_MoveWindow,&_RegisterClass, &_SendMessage, &_SetFocus,&_SetWindowLong, &_SetWindowText, &_ShowWindow,&_TranslateMessage,&_FindWindowEx, &_EnumDesktopWindows,&_OemToChar, &_CharUpperBuff};

void InitUser32()
{
 _USER32 = _GetModuleHandle("user32.dll"); //fix strange "POP ECX"
 if (/*@S==*/_USER32 == NULL/*@E*/) _USER32 = _LoadLibrary("user32.dll");

 InitAPIbyStr(padrU32,_USER32,"/*@S|*/00CallWindowProcA|01CreateWindowExA|02DefWindowProcA|03DestroyWindow|04DispatchMessageA|05GetClassNameA|06GetForegroundWindow|07GetMessageA|08GetWindow|09GetWindowLongA|10GetWindowRect|11GetWindowTextA|12MessageBoxA|13MoveWindow|14RegisterClassA|15SendMessageA|16SetFocus|17SetWindowLongA|18SetWindowTextA|19ShowWindow|20TranslateMessage|21FindWindowExA|22EnumDesktopWindows|23OemToCharA|24CharUpperBuffA/*@E*/|");


/*
 _CallWindowProc   = (DWORD *)_GPAecoU32("CallWindowProcA");//@S
 _CreateWindowEx   = (DWORD *)_GPAecoU32("CreateWindowExA");
 _DefWindowProc    = (DWORD *)_GPAecoU32("DefWindowProcA");
 _DestroyWindow    = (DWORD *)_GPAecoU32("DestroyWindow");
 _DispatchMessage  = (DWORD *)_GPAecoU32("DispatchMessageA");
 _GetClassName     = (DWORD *)_GPAecoU32("GetClassNameA");
 _GetForegroundWindow = (DWORD *)_GPAecoU32("GetForegroundWindow");
 _GetMessage       = (DWORD *)_GPAecoU32("GetMessageA");
 _GetWindow        = (DWORD *)_GPAecoU32("GetWindow");
 _GetWindowLong    = (DWORD *)_GPAecoU32("GetWindowLongA");
 _GetWindowRect    = (DWORD *)_GPAecoU32("GetWindowRect");
 _GetWindowText    = (DWORD *)_GPAecoU32("GetWindowTextA");
 _MessageBox       = (DWORD *)_GPAecoU32("MessageBoxA");
 _MoveWindow       = (DWORD *)_GPAecoU32("MoveWindow");
 _RegisterClass    = (DWORD *)_GPAecoU32("RegisterClassA");
 _SendMessage      = (DWORD *)_GPAecoU32("SendMessageA");
 _SetFocus         = (DWORD *)_GPAecoU32("SetFocus");
 _SetWindowLong    = (DWORD *)_GPAecoU32("SetWindowLongA");
 _SetWindowText    = (DWORD *)_GPAecoU32("SetWindowTextA");
 _ShowWindow       = (DWORD *)_GPAecoU32("ShowWindow");
 _TranslateMessage = (DWORD *)_GPAecoU32("TranslateMessage");
 _FindWindowEx     = (DWORD *)_GPAecoU32("FindWindowExA");
 _EnumDesktopWindows = (DWORD *)_GPAecoU32("EnumDesktopWindows");//@E
*/
// _SetTimer         = (DWORD *)_GPAecoU32("SetTimer");
// _LoadCursor       = (DWORD *)_GPAecoU32("LoadCursorA");
// _LoadIcon         = (DWORD *)_GPAecoU32("LoadIconA");
}
