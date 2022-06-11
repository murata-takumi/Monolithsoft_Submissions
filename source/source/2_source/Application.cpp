#include "Application.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "TitleScene.h"
#include "PlayScene.h"
#include "ResultScene.h"

const unsigned int DISPLAY_WIDTH = GetSystemMetrics(SM_CXSCREEN);	//ディスプレイ幅
const unsigned int WINDOW_WIDTH = 1280;								//ウィンドウ幅
const unsigned int DISPLAY_HEIGHT = GetSystemMetrics(SM_CYSCREEN);	//ディスプレイ高さ
const unsigned int WINDOW_HEIGHT = 720;								//ウィンドウ高さ

/// <summary>
/// OSから送られてくるデータ(メッセージ)を処理する関数
/// </summary>
/// <param name="hwnd">   ウィンドウを識別するデータ</param>
/// <param name="msg">    OSから送られてくるデータ</param>
/// <param name="wpraram">1つ目のメッセージの変数</param>
/// <param name="lparam" >2つ目のメッセージの変数</param>
/// <returns></returns>
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);	//OSにアプリの終了を伝える
		return 0;

	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_SYSKEYDOWN:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(msg, wparam, lparam);
		break;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);	//既定の処理を行う
}

/// <summary>
/// ゲーム用ウィンドウを作成する関数
/// </summary>
/// <param name="hwnd">       ウィンドウを識別するデータ</param>
/// <param name="windowClass">ウィンドウ作成用データを格納する構造体</param>
void 
Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	windowClass.cbSize = sizeof(WNDCLASSEX);			//構造体のメモリサイズ指定
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;	//コールバック関数の指定
	windowClass.lpszClassName = _T("DX12Sample");		//アプリケーションクラス名
	windowClass.hInstance = GetModuleHandle(nullptr);	//インスタンス(アプリケーション)ハンドルの取得

	RegisterClassEx(&windowClass);	                    //アプリケーションクラスの指定をOSに伝える

	RECT wrc = { 0,0,(LONG)WINDOW_WIDTH,(LONG)WINDOW_HEIGHT };			//ウィンドウサイズの決定
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);	//ウィンドウサイズを計算

	hwnd = CreateWindow(windowClass.lpszClassName,		//ウィンドウハンドルの登録
		_T("Program"),
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX ,
		(DISPLAY_WIDTH/2) - (WINDOW_WIDTH/2),
		(DISPLAY_HEIGHT / 2) - (WINDOW_HEIGHT / 2),
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		windowClass.hInstance,
		nullptr
	);

	//フレームレートを取得
	auto hdc = GetDC(hwnd);
	_rate = GetDeviceCaps(hdc, VREFRESH);

	//フレームレートに応じてフリップ間隔を設定
	if (_rate <= 60) { _interval = 1; }
	if (_rate >= 120) { _interval = 2; }

	_deltaTime = (float)1 / (float)_rate;	//1フレーム毎の経過秒数を計算
}

/// <summary>
/// コンストラクタ
/// 特に処理は行わない
/// </summary>
Application::Application()
{

}

/// <summary>
/// 静的インスタンスを返す関数
/// </summary>
/// <returns>インスタンスのアドレス</returns>
Application&
Application::Instance()
{
	static Application instance;
	return instance;
}

/// <summary>
/// アプリケーション初期化関数
/// </summary>
/// <returns>初期化が成功したかどうか</returns>
bool 
Application::Init()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);						//スレッド処理を行うためCOMライブラリを初期化
	CreateGameWindow(_hwnd, _windowClass);										//ゲーム用ウィンドウ作成

	ShowCursor(true);															//マウスカーソルを非表示にする

	_dx12.reset(new Dx12Wrapper(_hwnd,_deltaTime));								//Dx12Wrapperインスタンスを初期化
	
	_sprite.reset(new SpriteManager(*_dx12,WINDOW_WIDTH,WINDOW_HEIGHT));		//DXTK12Managerインスタンスを初期化
	
	_input.reset(new InputManager());											//InputManagerインスタンスを初期化

	_pera.reset(new PeraRenderer(*_dx12));										//PeraRendererインスタンスを初期化

	_renderer.reset(new Renderer(*_dx12));										//PMDRendererインスタンスを初期化

	_sound.reset(new SoundManager());											//SoundManagerインスタンスを初期化

	//それぞれタイトル・ゲーム・リザルトシーンの初期化
	_title.reset(new TitleScene(*_dx12,*_pera, *_renderer, *_sprite, *_input, *_sound));
	_play.reset(new PlayScene( *_dx12, *_pera, *_renderer,*_sprite,*_input,*_sound));
	_result.reset(new ResultScene( *_dx12, *_pera, *_renderer,*_sprite,*_input,*_sound));

	ShowWindow(_hwnd, SW_SHOW);			//ウィンドウハンドルに対応するウィンドウを表示

	ChangeScene(SceneNames::Title);	//タイトルシーンへ遷移

	return true;	//初期化が完了したことを返す
}

/// <summary>
/// ゲーム画面を描画する関数
/// </summary>
void
Application::Run()
{
	//ゲームループ
	while (true)
	{
 		//スレッド関係の処理
		if (PeekMessage(&_msg, nullptr, 0, 0, PM_REMOVE))	//メッセージを調べる
		{
			TranslateMessage(&_msg);							//メッセージを翻訳
			DispatchMessage(&_msg);								//ウィンドウプロシージャへメッセージを送る

			if (_msg.message == WM_QUIT)						//アプリが終了したらループを終了させる
			{
				break;
			}
		}
		else
		{
			_Scene->Update();	//各シーンの更新処理
		}
	}
}

/// <summary>
/// アプリ終了時、後始末を行う関数
/// </summary>
void 
Application::Terminate()
{
	UnregisterClass(_windowClass.lpszClassName, _windowClass.hInstance);	//構造体の設定を解除
}

/// <summary>
/// ウィンドウサイズを返す関数
/// </summary>
/// <returns>ウィンドウサイズを示す構造体</returns>
SIZE
Application::GetWindowSize()const
{
	SIZE ret;					//構造体を宣言し、幅と高さを設定
	ret.cx = WINDOW_WIDTH;
	ret.cy = WINDOW_HEIGHT;

	return ret;					//構造体を返す
}

/// <summary>
/// シーンを切り替える関数
/// </summary>
/// <param name="scene">切り替えたいシーンオブジェクト</param>
void
Application::SetScene(Scene* scene)
{
	if (_Scene != nullptr)_Scene->SceneEnd();	//シーン終了時の処理

	_Scene = scene;								//シーン切り替え

	_Scene->SceneStart();						//シーン開始時の処理
}

/// <summary>
/// シーンを切り替える関数
/// </summary>
/// <param name="name">切り替えたいシーン</param>
void
Application::ChangeScene(SceneNames name)
{
	//引数に応じて遷移先のシーンを決める
	switch (name)
	{
	case SceneNames::Title:
		SetScene(_title.get());		//タイトルシーンへ遷移
		break;
	case SceneNames::Play:
		SetScene(_play.get());		//ゲームシーンへ遷移
		break;
	case SceneNames::Result:
		SetScene(_result.get());	//リザルトシーンへ遷移
		break;
	}
}

/// <summary>
/// ゲームを終了させる
/// </summary>
void
Application::ExitApp()
{
	SendMessage(_hwnd,WM_DESTROY,0,0);	//終了用メッセージを送る
}

/// <summary>
/// レンダーターゲットのフリップ間隔を返す関数
/// </summary>
/// <returns>フリップ間隔</returns>
int
Application::GetInterval()
{
	return _interval;
}

/// <summary>
/// 1秒間のフレームレートを返す関数
/// </summary>
/// <returns>フレームレート</returns>
int
Application::GetRate()
{
	return _rate;
}

/// <summary>
/// デコンストラクタ
/// 特に処理は行わない
/// </summary>
Application::~Application()
{

}