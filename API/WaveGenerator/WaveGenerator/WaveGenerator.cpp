#include <windows.h>		//Windows環境
#include <windowsx.h>		//Windows環境
#include <stdio.h>			//入出力用
#include <process.h>		//スレッド用
#include <stdlib.h>
#include <cmath>

#pragma warning(disable: 4996)
#pragma comment(lib,"winmm.lib")//高精度タイマ

#include "resource.h"		//リソースファイル
#include "header.h"		//リソースファイル

//構造体
typedef struct {
	HWND	hwnd;
	HWND	hPict1;
	HWND	hPict2;
	HWND	hPict3;
	HWND	hPict4;
}SEND_POINTER_STRUCT;

//メイン関数(ダイアログバージョン)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HANDLE hMutex;

	//多重起動判定
	hMutex = CreateMutex(NULL, TRUE, DEF_MUTEX_NAME);		//ミューテックスオブジェクトの生成
	if (GetLastError() == ERROR_ALREADY_EXISTS) {				//2重起動の有無を確認
		MessageBox(NULL, TEXT("既に起動されています．"), NULL, MB_OK | MB_ICONERROR);
		return 0;											//終了
	}

	//ダイアログ起動
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MainDlgProc);

	return FALSE;			//終了
}

//メインプロシージャ（ダイアログ）
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	static HWND hPict1;
	static HWND hPict2;
	static HWND hPict3;
	static HWND hPict4;
	static HFONT hFont;				//フォント
	static HANDLE hThread;
	static UINT thID;
	static SEND_POINTER_STRUCT Sps;
	static HWND hEdit;

	switch (uMsg) {
	case WM_INITDIALOG:		//ダイアログ初期化
		Sps.hwnd = hDlg;
		hPict1 = GetDlgItem(hDlg, IDC_STATIC_LT);
		hPict2 = GetDlgItem(hDlg, IDC_STATIC_LB);
		hPict3 = GetDlgItem(hDlg, IDC_STATIC_RT);
		hPict4 = GetDlgItem(hDlg, IDC_STATIC_RB);
		Sps.hPict1 = hPict1;
		Sps.hPict2 = hPict2;
		Sps.hPict3 = hPict3;
		Sps.hPict4 = hPict4;
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);						//ストップボタン無効化　
		return TRUE;

	case WM_COMMAND:		//ボタンが押された時
		switch (LOWORD(wParam)) {
		case ID_START:			//開始ボタン

			//初期化(背景等描画)
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS1", Sps.hPict1, WndProc, &hPict1); 
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS2", Sps.hPict2, WndProc, &hPict2);
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS3", Sps.hPict3, WndProc, &hPict3);
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS4", Sps.hPict4, WndProc, &hPict4);
			
			//データ読み込みスレッド起動
			hThread = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)&Sps, 0, &thID);   //_beginthreadex→スレッドを立ち上げる関数	
			EnableWindow(GetDlgItem(hDlg, ID_START), FALSE);						//開始ボタン無効化　　　　//EnableWindowで入力を無効または有効にする。
			EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);						//ストップボタン有効化　

			return TRUE;

		case IDCANCEL:	//停止・再開ボタン

						/*　サスペンドカウンタ　**************************
						　　　実行を許可するまでスレッドを動かさない。
						   　　ResumeThread：　サスペンドカウンタを1減らす
							 　SuspendThread：　サスペンドカウンタを1増やす

							  0のときは実行。それ以外は待機する。
							  **************************************************/


			if (ResumeThread(hThread) == 0) {					//停止中かを調べる(サスペンドカウントを１減らす)
				SetDlgItemText(hDlg, IDCANCEL, TEXT("再開"));	//再開に変更　　　　　　　　　　　　　　　　　　　//SetDlgItemTextでダイアログ内のテキストなどを変更することができる
				SuspendThread(hThread);						//スレッドの実行を停止(サスペンドカウントを１増やす)
			}
			else
				SetDlgItemText(hDlg, IDCANCEL, TEXT("停止"));	//停止に変更

			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);			//ダイアログ終了
		return TRUE;
	}

	return FALSE;
}

//データ読み込み用スレッド
UINT WINAPI TFunc(LPVOID thParam)
{
	static SEND_POINTER_STRUCT* FU = (SEND_POINTER_STRUCT*)thParam;        //構造体のポインタ取得


	FILE* fp;
	FILE* fpPara;			//ファイルポインタ
	double max[4] = { 0 }, min[4] = { 0 };
	BOOL Flag = TRUE;		//ループフラグ
	HFONT hFont;		//フォント
	PAINTSTRUCT ps1, ps2, ps3, ps4;					//(構造体)クライアント領域描画するための情報	
	HDC hdc1, hdc2, hdc3, hdc4;

	double data;		//データ
	HPEN hPenWave, hOldPenWave;
	colorWave = RGB(0, 0, 255);
	RECT rect;	//描画領域
	GetClientRect(FU->hPict1, &rect);
	int xMax, xMin;
	double yZero;
	int ifInitial = 1, counter = -1;
	int x, oldX;
	double y1, oldY1, y2, oldY2, y3, oldY3, y4, oldY4;


	/*　wchar_t型　***********************************************
	char型はLPCWS型と互換性がないためそのままだと文字化けする。
	mbstowcs_s関数を用いて、charからwcharに変換する必要がある。
	***************************************************************/
	hdc1 = GetDC(FU->hPict1);//デバイスコンテキストのハンドル取得
	hdc2 = GetDC(FU->hPict2);//デバイスコンテキストのハンドル取得
	hdc3 = GetDC(FU->hPict3);//デバイスコンテキストのハンドル取得
	hdc4 = GetDC(FU->hPict4);//デバイスコンテキストのハンドル取得
	hPenWave = CreatePen(PS_SOLID, 2, colorWave);		//ペン生成
	hOldPenWave = (HPEN)SelectObject(hdc1, hPenWave);		//ペン設定
	SelectObject(hdc2, hPenWave);
	SelectObject(hdc3, hPenWave);
	SelectObject(hdc4, hPenWave);
	xMax= rect.right - 30,	xMin = 30,	yZero = rect.bottom / 2;
	oldX = xMin - 1, x = xMin;
	SetTextColor(hdc1, RGB(255, 255, 255));
	SetBkColor(hdc1, RGB(0, 0, 0));
	SetTextColor(hdc2, RGB(255, 255, 255));
	SetBkColor(hdc2, RGB(0, 0, 0));
	SetTextColor(hdc3, RGB(255, 255, 255));
	SetBkColor(hdc3, RGB(0, 0, 0));
	SetTextColor(hdc4, RGB(255, 255, 255));
	SetBkColor(hdc4, RGB(0, 0, 0));

	DWORD DNum = 0, beforeTime;

	int time = 0;

	//エディタのフォント変更(case WM_INITDIALOG:の中で設定しても良い)
	hFont = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, NULL);
	//SendMessage(FU->hEdit, WM_SETFONT, (int)hFont, TRUE);

	beforeTime = timeGetTime();						//現在の時刻計算（初期時間）

													//ファイルオープン
	if ((fpPara = fopen("13_49_57check.txt", "r")) == NULL) {
		MessageBox(NULL, TEXT("Input File Open ERROR!"), NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	int i;
	while (Flag == TRUE) {
		if (fscanf(fpPara, "%lf", &data) == EOF) {
			Flag = FALSE;												//ループ終了フラグ
		}
		counter++;
		i = counter % 5;
		if (i == 0)
			continue;
		
		max[i - 1] = max[i - 1] < data ? data : max[i - 1];
		min[i - 1] = data < min[i - 1] ? data : min[i - 1];
	}
	
	for (i = 0; i < 4; i++) {
		min[i] = abs(min[i]);
		max[i] = max[i] > min[i] ? max[i] : min[i];
	}

	fclose(fpPara);


	if ((fp = fopen("13_49_57check.txt", "r")) == NULL) {
		MessageBox(NULL, TEXT("Input File Open ERROR!"), NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	counter = 0;
	Flag = TRUE;
	//データ読み込み・表示
	while (Flag == TRUE) {
		DWORD nowTime, progress, idealTime;

		//時間の調整
		nowTime = timeGetTime();					//現在の時刻計算
		progress = nowTime - beforeTime;				//処理時間を計算
		idealTime = (DWORD)(DNum * (1000.0 / (double)DEF_DATAPERS));	//理想時間を計算
		if (idealTime > progress) {
			Sleep(idealTime - progress);			//理想時間になるまで待機
		}
		
		
		//データの読み込み
		if (fscanf(fp, "%lf", &data) == EOF) {
			MessageBox(NULL, TEXT("終了"), TEXT("INFORMATION"), MB_OK | MB_ICONEXCLAMATION);
			EnableWindow(GetDlgItem(FU->hwnd, ID_START), TRUE);		//開始ボタン有効
			EnableWindow(GetDlgItem(FU->hwnd, IDCANCEL), FALSE);	//ストップボタン無効
			Flag = FALSE;												//ループ終了フラグ
			return FALSE;
		}

		counter++;

		
		//表示
		switch (counter % 5) {
		case 1:
			break;
		case 2:
			data = data / max[0];
			if (ifInitial == 1) {
				TextOut(hdc1, 10, 10, TEXT("Ch1"), 3);		//テキスト描画
				y1 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
				break;
			}
			oldY1 = y1;
			y1 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero	;
			//軸描画
			MoveToEx(hdc1, oldX, (int)oldY1, NULL);
			LineTo(hdc1, x, (int)y1);
			break;

		case 3:
			data = data / max[1];
			if (ifInitial == 1) {
				TextOut(hdc2, 10, 10, TEXT("Ch2"), 3);		//テキスト描画
				y2 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
				break;
			}
			oldY2 = y2;
			y2 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
			//軸描画
			MoveToEx(hdc2, oldX, (int)oldY2, NULL);
			LineTo(hdc2, x, (int)y2);
			break;
		case 4:
			data = (data ) / max[2];
			if (ifInitial == 1) {
				TextOut(hdc3, 10, 10, TEXT("Ch3"), 3);		//テキスト描画
				y3 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
				break;
			}
			oldY3 = y3;
			y3 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
			//軸描画
			MoveToEx(hdc3, oldX, (int)oldY3, NULL);
			LineTo(hdc3, x, (int)y3);
			break;

		case 0:
			data = data / max[3];
			if (ifInitial == 1) {
				TextOut(hdc4, 10, 10, TEXT("Ch4"), 3);		//テキスト描画
				ifInitial = 0;
				x++;	 oldX++;
				y4 = -1.0 * data * (rect.bottom/2-30.0) + yZero;
				break; 
			}
			oldY4 = y4;
			y4 = -1.0 * data * (rect.bottom / 2 - 30.0) + yZero;
			//軸描画
			MoveToEx(hdc4, oldX, (int)oldY4, NULL);
			LineTo(hdc4, x, (int)y4);
			x++;	oldX++;
			if (x > xMax) {
				oldX = xMin - 1, x = xMin;
				ifInitial = 1;
				InvalidateRect(FU->hwnd, NULL, TRUE);
				counter = 0;
			}
			break;
		}

		
		DNum++;

		//一秒経過時
		if (progress >= 1000.0) {
			beforeTime = nowTime;
			DNum = 0;
		}
	}

	//ペン，ブラシ廃棄
	SelectObject(hdc1, hOldPenWave);
	DeleteObject(hPenWave);
	
	//デバイスコンテキストのハンドル破棄
	EndPaint(FU->hPict1, &ps1);
	EndPaint(FU->hPict2, &ps2);
	EndPaint(FU->hPict3, &ps3);
	EndPaint(FU->hPict4, &ps4);

	return 0;
}

//
//UINT WINAPI Wave(LPVOID thParam) {
//	static SEND_POINTER_STRUCT* FU = (SEND_POINTER_STRUCT*)thParam;        //構造体のポインタ取得
//
//
//	return 0;
//}




//子ウィンドウプロシージャ1
HRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	HDC			hdc;				//デバイスコンテキストのハンドル
	PAINTSTRUCT ps;					//(構造体)クライアント領域描画するための情報	
	HBRUSH		hBrushBack, hOldBrushBack;	//ブラシ
	HPEN		hPenAxis, hOldPenAxis;		//ペン
	RECT		rect;	//描画領域

	switch (uMsg) {
	case WM_CREATE:
		//色指定
		colorBackground = RGB(0, 0, 0);
		colorAxis = RGB(255, 255, 255);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);//デバイスコンテキストのハンドル取得

		//ペン，ブラシ生成
		hBrushBack = CreateSolidBrush(colorBackground);				//ブラシ生成
		hOldBrushBack = (HBRUSH)SelectObject(hdc, hBrushBack);	//ブラシ
		GetClientRect(hWnd, &rect);	//領域の大きさ取得
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);	//背景描画

		hPenAxis = CreatePen(PS_SOLID, 2, colorAxis);		//ペン生成
		hOldPenAxis = (HPEN)SelectObject(hdc, hPenAxis);		//ペン設定
		//軸描画
		MoveToEx(hdc, 30, 15, NULL);
		LineTo(hdc, 30, rect.bottom - 15);
		MoveToEx(hdc, 30, rect.bottom/2, NULL);
		LineTo(hdc, rect.right - 30, rect.bottom/2);

		SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, colorAxis);
		SetBkColor(hdc, colorBackground);
		TextOut(hdc, rect.right/2, rect.bottom - 30, TEXT("Time(s)"), 7);		//テキスト描画

		//ペン，ブラシ廃棄
		SelectObject(hdc, hOldBrushBack);
		DeleteObject(hBrushBack);
		SelectObject(hdc, hOldPenAxis);
		DeleteObject(hPenAxis);

		//デバイスコンテキストのハンドル破棄
		EndPaint(hWnd, &ps);
		break;
	}

	return TRUE;
}

//ここまで

//-----------------------------------------------------------------------------
//子ウィンドウ初期化＆生成
//指定したウィンドウハンドルの領域に子ウィンドウを生成する．
//----------------------------------------------------------
// hInst	: 生成用インスタンスハンドル
// hPaWnd	: 親ウィンドウのハンドル
// chID		: 子ウィンドウのID
// cWinName	: 子ウィンドウ名
// PaintArea: 子ウィンドウを生成する領域のデバイスハンドル
// WndProc	: ウィンドウプロシージャ
// *hWnd	: 子ウィンドウのハンドル（ポインタ）
// 戻り値	: 成功時=true
//-----------------------------------------------------------------------------
BOOL WinInitialize(HINSTANCE hInst, HWND hPaWnd, HMENU chID, const char* cWinName, HWND PaintArea, WNDPROC WndProc, HWND* hWnd)
{
	WNDCLASS wc;			//ウィンドウクラス
	WINDOWPLACEMENT	wplace;	//子ウィンドウ生成領域計算用（画面上のウィンドウの配置情報を格納する構造体）
	RECT WinRect;			//子ウィンドウ生成領域
	ATOM atom;				//アトム

	//ウィンドウクラス初期化
	wc.style = CS_HREDRAW ^ WS_MAXIMIZEBOX | CS_VREDRAW;	//ウインドウスタイル
	wc.lpfnWndProc = WndProc;									//ウインドウのメッセージを処理するコールバック関数へのポインタ
	wc.cbClsExtra = 0;											//
	wc.cbWndExtra = 0;
	wc.hCursor = NULL;										//プログラムのハンドル
	wc.hIcon = NULL;										//アイコンのハンドル
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);		//ウインドウ背景色
	wc.hInstance = hInst;										//ウインドウプロシージャがあるインスタンスハンドル
	wc.lpszMenuName = NULL;										//メニュー名
	wc.lpszClassName = (LPCTSTR)cWinName;									//ウインドウクラス名

	if (!(atom = RegisterClass(&wc))) {
		MessageBox(hPaWnd, TEXT("ウィンドウクラスの生成に失敗しました．"), NULL, MB_OK | MB_ICONERROR);
		return false;
	}

	GetWindowPlacement(PaintArea, &wplace);	//描画領域ハンドルの情報を取得(ウィンドウの表示状態を取得)
	WinRect = wplace.rcNormalPosition;		//描画領域の設定

	//ウィンドウ生成
	*hWnd = CreateWindow(
		(LPCTSTR)atom,
		(LPCTSTR)cWinName,
		WS_CHILD | WS_VISIBLE,//| WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME |WS_VISIBLE, 
		WinRect.left, WinRect.top,
		WinRect.right - WinRect.left, WinRect.bottom - WinRect.top,
		hPaWnd, chID, hInst, NULL
	);

	if (*hWnd == NULL) {
		MessageBox(hPaWnd, TEXT("ウィンドウの生成に失敗しました．"), NULL, MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}