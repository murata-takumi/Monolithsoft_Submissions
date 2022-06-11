#pragma once
#include <d3dcompiler.h>
#include <map>
#include <tchar.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXTex.h>
#include <fbxsdk.h>
#include <vector>

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace fbxsdk;

#pragma comment(lib,"DirectXTK12.lib")
#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

/// <summary>
/// FBXモデルの頂点用構造体
/// </summary>
struct FBXVertex
{
	XMFLOAT3 position;	//座標
	XMFLOAT3 normal;	//法線
	XMFLOAT2 uv;		//UV座標
	XMFLOAT3 tangent;	//正接
	XMFLOAT4 color;		//頂点カラー
};

/// <summary>
/// FBXモデルを構成するメッシュの構造体
/// </summary>
struct Mesh
{
	vector<FBXVertex> vertices;	//頂点ベクトル
	vector<uint32_t> indices;	//インデックスベクトル
	wstring diffuseMap;			//テクスチャのファイルパス
};

/// <summary>
/// 読み込むSceneクラスを識別するための列挙型
/// </summary>
enum SceneNames
{
	Title,		//タイトルシーン
	Play,		//ゲームシーン
	Result,		//リザルトシーン
};

//以下のクラス宣言はApplicationのクラス定義で各クラスを呼び出せるようにするため記述
class Dx12Wrapper;
class PeraRenderer;
class Renderer;
class FBXActor;
class SpriteManager;
class InputManager;
class SoundManager;
class Scene;
class TitleScene;
class PlayScene;
class ResultScene;
/// <summary>
/// ゲームの初期化・更新・終了を管理するクラス
/// </summary>
class Application
{
private:
	WNDCLASSEX _windowClass;					//ウィンドウ作成時に必要な情報を格納
	HWND _hwnd;									//ウィンドウの識別に必要な値

	//shared_ptr<class>は使用者がいなくなった時自動的にリソースを開放するスマートポインタ
	shared_ptr<Dx12Wrapper> _dx12;				//Dx12Wrapperインスタンス
	shared_ptr<PeraRenderer> _pera;				//PeraRendererインスタンス
	shared_ptr<Renderer> _renderer;				//Rendererインスタンス
	shared_ptr<FBXActor> _actor;				//FBXActorインスタンス
	shared_ptr<SpriteManager> _sprite;			//DXTK12Managerインスタンス
	shared_ptr<InputManager> _input;			//InputManagerインスタンス	
	shared_ptr<SoundManager> _sound;			//SoundManagerインスタンス

	shared_ptr<TitleScene> _title;				//TitleSceneインスタンス
	shared_ptr<PlayScene> _play;				//PlaySceneインスタンス
	shared_ptr<ResultScene> _result;			//ResultSceneインスタンス

	Scene* _Scene;								//更新処理を行うSceneオブジェクト

	int _rate;			//1秒あたりのフレーム数
	int _interval;		//フリップ間隔
	float _deltaTime;	//フレーム間の経過時間

	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);		//ゲーム用ウィンドウを作成する関数

	Application();													//コンストラクタ

	//コンストラクタを外部から呼び出されないよう設定
	Application(const Application&) = delete;
	void operator = (const Application&) = delete;
	
public:

	MSG _msg = {};								//メッセージ用構造体

	static Application& Instance();		//インスタンスを返す？

	bool Init();						//初期化

	void Run();							//ゲーム画面の描画

	void Terminate();					//ゲーム終了時の後始末

	SIZE GetWindowSize()const;			//ウィンドウサイズを返す

	void SetScene(Scene* scene);		//シーンを切り替える
	void ChangeScene(SceneNames name);	//シーンの切り替え
	void ExitApp();						//アプリケーションを終了する

	int GetInterval();					//レンダーターゲットのフリップ間隔を返す関数
	int GetRate();						//1秒間のフレームレートを返す関数

	~Application();						//デコンストラクタ
};
