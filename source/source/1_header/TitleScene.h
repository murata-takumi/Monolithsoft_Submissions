#pragma once
#include "Application.h"
#include "Scene.h"

/// <summary>
/// タイトルシーンを管理するクラス
/// </summary>
class TitleScene : public Scene
{
public:
	//コンストラクタ
	TitleScene(
		Dx12Wrapper& dx12,
		PeraRenderer& pera,
		Renderer& renderer,
		SpriteManager& sprite,
		InputManager& input,
		SoundManager& sound
	);
	~TitleScene();					//デコンストラクタ

	void Update() override;			//更新処理

	void SceneStart() override;		//シーン開始時の処理
	void SceneEnd() override;		//シーン終了時の処理
private:
	void  LoadingLoop()override;	//重い処理を実行するための関数
};