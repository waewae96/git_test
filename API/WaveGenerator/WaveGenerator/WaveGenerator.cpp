#include <windows.h>		//Windows環境
#include <windowsx.h>		//Windows環境
#include <stdio.h>			//入出力用
#include <process.h>		//スレッド用
#include <stdlib.h>

#pragma warning(disable: 4996)
#pragma comment(lib,"winmm.lib")//高精度タイマ

#include "resource.h"		//リソースファイル
#include "header.h"		//リソースファイル

//構造体
typedef struct {
	HWND	hwnd;
	HWND	hPict1;
	HWND	hPict2;
	//HWND	hEdit;
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
	static HFONT hFont;				//フォント
	static HWND hWnd1;
	static HWND hWnd2;
	static HANDLE hThread;
	static UINT thID;
	static SEND_POINTER_STRUCT Sps;
	static HWND hEdit;

	switch (uMsg) {
	case WM_INITDIALOG:		//ダイアログ初期化
		Sps.hwnd = hDlg;
		hPict1 = GetDlgItem(hDlg, IDC_STATIC_TOP);
		hPict2 = GetDlgItem(hDlg, IDC_STATIC_BOTTOM);
		Sps.hPict1 = hPict1;
		Sps.hPict2 = hPict2;
		//Sps.hEdit = hEdit;
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);						//ストップボタン無効化　
		return TRUE;

	case WM_COMMAND:		//ボタンが押された時
		switch (LOWORD(wParam)) {
		case ID_START:			//開始ボタン

			/*とりあえず1つめWindowだけおわらす*/
			//初期化(背景等描画)
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS1", Sps.hPict1, WndProc, &hWnd1); 
			WinInitialize(NULL, hDlg, (HMENU)110, "CLS2", Sps.hPict2, WndProc, &hWnd2);
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

	FILE* fp;			//ファイルポインタ
	BOOL Flag = TRUE;		//ループフラグ
	HFONT hFont;		//フォント

	double data;		//データ
	char str[256];		//データ
	char str2[256];     //分数用
	wchar_t wcs1[256];
	wchar_t wcs2[256];
	size_t ret;

	/*　wchar_t型　***********************************************
	char型はLPCWS型と互換性がないためそのままだと文字化けする。
	mbstowcs_s関数を用いて、charからwcharに変換する必要がある。
	***************************************************************/


	DWORD DNum = 0, beforeTime;

	int time = 0, min = 0;

	//エディタのフォント変更(case WM_INITDIALOG:の中で設定しても良い)
	hFont = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, NULL);
	//SendMessage(FU->hEdit, WM_SETFONT, (int)hFont, TRUE);

	beforeTime = timeGetTime();						//現在の時刻計算（初期時間）

													//ファイルオープン
	if ((fp = fopen("data.txt", "r")) == NULL) {
		MessageBox(NULL, TEXT("Input File Open ERROR!"), NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

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
			//背景の再描画

			//読み込みを最初からにして再びループに入る

			MessageBox(NULL, TEXT("終了"), TEXT("INFORMATION"), MB_OK | MB_ICONEXCLAMATION);
			EnableWindow(GetDlgItem(FU->hwnd, ID_START), TRUE);		//開始ボタン有効
			Flag = FALSE;												//ループ終了フラグ
			return FALSE;
		}

		//表示
		_gcvt(data, 8, str);								//double型を文字列に変換
		//_gcvt(idealTime, 8, str);								//double型を文字列に変換
		mbstowcs_s(&ret, wcs1, 256, str, _TRUNCATE);        //charからwcharに変換する
		//SetDlgItemText(FU->hwnd, IDC_EDIT1, wcs1);				//エディタに表示

		DNum++;

		//一秒経過時
		if (progress >= 1000.0) {
			beforeTime = nowTime;
			DNum = 0;
		}

		//秒数表示
		time++;
		_itoa((int)(time / (int)DEF_DATAPERS), str, 10);	//秒数(int型)を文字列に変換
		mbstowcs_s(&ret, wcs1, 256, str, _TRUNCATE);        //charからwcharに変換する
		//SetDlgItemText(FU->hwnd, IDC_STATIC3, wcs1);			//表示


																//60秒経過したとき
		if (time == 60000) {
			min++;                                          //分表示に切り替える
			time = 0;
			itoa((int)(time / (int)DEF_DATAPERS), str, 10);	//秒数(int型)を文字列に変換
			mbstowcs_s(&ret, wcs1, 256, str, _TRUNCATE);    //charからwcharに変換する
			//SetDlgItemText(FU->hwnd, IDC_STATIC3, wcs1);			//0と表示
			itoa((int)(min), str2, 10);	                 //分数(int型)を文字列に変換
			mbstowcs_s(&ret, wcs2, 256, str2, _TRUNCATE);    //charからwcharに変換する
			//SetDlgItemText(FU->hwnd, IDC_STATIC1, wcs2);       //分数を画面表示

		}
	}

	return 0;
}

//子ウィンドウプロシージャ1
HRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	HDC			hdc;				//デバイスコンテキストのハンドル
	PAINTSTRUCT ps;					//(構造体)クライアント領域描画するための情報	
	HBRUSH		hBrushBack, hOldBrushBack;	//ブラシ
	HPEN		hPenAxis, hPenWave, hOldPenAxis, hOldPenWave;		//ペン
	RECT		rect;	//描画領域

	switch (uMsg) {
	case WM_CREATE:
		//色指定
		colorBackground = RGB(0, 0, 0);
		colorAxis = RGB(255, 255, 255);
		colorWave = RGB(0, 0, 255);
		break;

	case WM_PAINT:

		/********************************

		PictureControlに描画するためには，HDC型のハンドルを別に取得する
		必要があります．

		例：hdc = BeginPaint(hWnd, &ps);
		hdc:デバイスコンテキストのハンドル
		hWnd:PictureControlのハンドル
		ps：(構造体)クライアント領域描画するための情報

		********************************/

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

		//hPenWave = CreatePen(PS_SOLID, 2, colorWave);		//ペン生成
		//hOldPenWave = (HPEN)SelectObject(hdc, hPenWave);		//ペン設定


		//描画
		/********************************

		図形を描画するためには以下の関数を用います．
		長方形：Rectangle(HDC hdc ,int nLeftRect , int nTopRect ,int nRightRect , int nBottomRect);
		円：Ellipse(HDC hdc ,int nLeftRect , int nTopRect ,int nRightRect , int nBottomRect);

		 nLiftRect：長方形の左上X座標
		  nTopRect：左上Y座標
		  nRightRect：右下X座標
		  nBottomRect：右下のY座標

		線を引くには以下の関数を用います．

		線の始点設定：MoveToEx(HDC hdc , int X , int Y , NULL);
		  X,Y：線の始点の座標
		線；LineTo(HDC hdc , int nXEnd , int nYEnd);
		  nXEnd, nYEnd：線の終点の設定


		  以上を参考に図形を描画する関数を以下に記述しましょう
		********************************/
		SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, colorAxis);
		SetBkColor(hdc, colorBackground);
		TextOut(hdc, rect.right/2, rect.bottom - 30, TEXT("Time(s)"), 7);		//テキスト描画

		//ペン，ブラシ廃棄
		/********************************

		使い終わったペンとブラシは破棄する必要があります．

		********************************/
		SelectObject(hdc, hOldBrushBack);
		DeleteObject(hBrushBack);
		SelectObject(hdc, hOldPenAxis);
		DeleteObject(hPenAxis);
		//SelectObject(hdc, hOldPenWave);
		//DeleteObject(hPenWave);

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