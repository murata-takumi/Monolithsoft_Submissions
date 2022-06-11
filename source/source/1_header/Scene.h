#pragma once
#include "Application.h"
#include <thread>

/// <summary>
/// 各シーンオブジェクトの親クラス
/// </summary>
class Dx12Wrapper;
class PeraRenderer;
class Renderer;
class FBXActor;
class SpriteManager;
class InputManager;
class SoundManager;
class Scene
{
public:

	/// <summary>
	/// コンストラクタ
	/// 各インスタンスに格納する
	/// </summary>
	/// <param name="dx12">Dx12Wrapperインスタンス</param>
	/// <param name="pera">PeraRendererインスタンス</param>
	/// <param name="renderer">Rendererインスタンス</param>
	/// <param name="sprite">SpriteManagerインスタンス</param>
	/// <param name="input">InputManagerインスタンス</param>
	/// <param name="sound">SoundManagerインスタンス</param>
	Scene(
		Dx12Wrapper& dx12,
		PeraRenderer& pera,
		Renderer& renderer,
		SpriteManager& sprite,
		InputManager& input,
		SoundManager& sound
	) :_dx12(dx12),_pera(pera), _renderer(renderer), _sprite(sprite),
		_input(input), _sound(sound)
	{
		//フリップ間隔を取得
		auto& app = Application::Instance();
		_interval = app.GetInterval();

		//更新処理のラムダ式を登録
		func = [&]() {return Update(); };	
	}

	/// <summary>
	/// デコンストラクタ
	/// 特に処理しない
	/// </summary>
	virtual ~Scene()
	{

	}

	virtual void Update() = 0;			//更新処理

	virtual void SceneStart() = 0;		//シーン開始時の処理
	virtual void SceneEnd() = 0;		//シーン終了時の処理
protected:
	Dx12Wrapper& _dx12;			//Dx12Wrapperインスタンス
	PeraRenderer& _pera;		//PeraRendererインスタンス
	Renderer& _renderer;		//Rendererインスタンス
	SpriteManager& _sprite;		//SpriteManagerインスタンス
	InputManager& _input;		//InputManagerインスタンス
	SoundManager& _sound;		//SoundManagerインスタンス

	function<void()> func;		//更新処理のラムダ式を入れる関数ラッパー

	thread _th;					//重い処理を行うためのスレッド

	/// <summary>
	/// ロード状態を識別するための列挙値
	/// </summary>
	enum LoadingStates
	{
		NotStart,	//未開始
		Started,	//開始
		Ended,		//終了
	};
	LoadingStates _state;				//ロード状態

	bool _isLoading;					//現在ロード中かどうか
	bool _isNotPlay;					//現在操作可能かどうか

	int _interval;						//フリップ間隔

	virtual void LoadingLoop() = 0;		//重い処理を実行するための関数
};