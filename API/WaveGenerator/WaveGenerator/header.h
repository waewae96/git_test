#pragma once
//定数宣言
#define DEF_APP_NAME	TEXT("Waveform Test")
#define DEF_MUTEX_NAME	DEF_APP_NAME			//ミューテックス名
#define DEF_DATAPERS 500	//1秒間に何データ入出力するか

//関数宣言
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);	//メインダイアログプロシージャ
BOOL WinInitialize(HINSTANCE hInst, HWND hPaWnd, HMENU chID, const char* cWinName, HWND PaintArea, WNDPROC WndProc, HWND* hDC);//子ウィンドウを生成
UINT WINAPI TFunc(LPVOID thParam);												//データ読み込み用スレッド
HRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);		//子ウィンドウプロシージャ

static COLORREF	colorBackground, colorAxis, colorWave;	//色