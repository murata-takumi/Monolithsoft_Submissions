#pragma once
#include "Application.h"
#include "Scene.h"

/// <summary>
/// ゲームシーンを管理するクラス
/// </summary>
class PlayScene :public Scene
{
public:
	//コンストラクタ
	PlayScene(
		Dx12Wrapper& dx12,
		PeraRenderer& pera,
		Renderer& renderer,
		SpriteManager& sprite,
		InputManager& input,
		SoundManager& sound
	);
	~PlayScene();							//デコンストラクタ

	void Update() override;				//更新処理

	void SceneStart() override;			//シーン開始時の処理
	void SceneEnd() override;			//シーン終了時の処理

protected:
private:
	shared_ptr<FBXActor> _actor;	//プレイヤーキャラクター

	float _maxHP;					//最大HP
	float _currentHP;				//現在HP

	void LoadFBXModel(Dx12Wrapper& dx12, const wchar_t* modelPath);		//3Dモデル読み込み
	void LoadingLoop() override;										//モデル読み込み用ループ
};