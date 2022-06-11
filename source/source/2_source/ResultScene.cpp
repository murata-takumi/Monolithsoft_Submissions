#include "ResultScene.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
/// <param name="renderer">Rendererインスタンス</param>
/// <param name="sprite">SpriteManagerインスタンス</param>
/// <param name="input">InputManagerインスタンス</param>
/// <param name="sound">SoundManagerインスタンス</param>
ResultScene::ResultScene
(
	Dx12Wrapper& dx12,
	PeraRenderer& pera,
	Renderer& renderer,
	SpriteManager& sprite,
	InputManager& input,
	SoundManager& sound
)
	:Scene
	(
		dx12,
		pera,
		renderer,
		sprite,
		input,
		sound
	)
{

}

/// <summary>
/// デコンストラクタ
/// </summary>
ResultScene::~ResultScene()
{

}

/// <summary>
/// 更新処理
/// </summary>
void
ResultScene::Update()
{
	_input.UpdateInput();	//入力を更新

	_sprite.SetMousePosition(_input.MouseState().x, _input.MouseState().y);	//マウス座標を格納

	if (_input.MouseTracker().leftButton == Mouse::ButtonStateTracker::PRESSED)
	{
		//開始ボタンの上で左クリックしたらゲームシーンに遷移
		if (_sprite.TitleIsOnStart())
		{
			_sound.CallButton();					//効果音

			auto& app = Application::Instance();
			app.ChangeScene(SceneNames::Play);
			return;
		}
		//タイトルボタンの上だったらタイトル画面に戻る
		else if (_sprite.TitleIsOnEnd())
		{
			_sound.CallButton();					//効果音

			auto& app = Application::Instance();
			app.ChangeScene(SceneNames::Title);
			return;
		}
	}

	_pera.BeginPeraDraw();	//ペラポリゴン描画準備

	_sprite.ResultDraw();	//タイトル画面でのボタン描画

	_sprite.CursorDraw();	//マウスカーソルを描画

	//以下ゲーム画面描画処理
	_dx12.SetPipelines(_renderer.GetRootSignature(), _renderer.GetPipelineState(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_dx12.SetScene();		//ビュー・プロジェクション行列を適用

	_pera.EndPeraDraw();	//ペラポリゴン描画後始末

	_dx12.BeginGameDraw();	//ゲーム画面描画準備

	//以下ペラポリゴン用コマンドリスト処理
	_pera.SetPeraPipelines();

	_dx12.CommandList()->IASetVertexBuffers(0, 1, &_pera.PeraVBView());		//ペラポリゴン用VBVをセット
	_dx12.CommandList()->DrawInstanced(4, 1, 0, 0);							//ペラポリゴンを構成する頂点を描画

	_dx12.EndGameDraw();	//ゲーム画面描画後始末

	_dx12.Swapchain()->Present((UINT)_interval, 0);	//スワップチェーンのフリップ処理

	_sprite.Commit();	//グラフィックスメモリを設定
}

/// <summary>
/// シーン開始時の処理
/// </summary>
void
ResultScene::SceneStart()
{
	_dx12.Fade(func, 1.0f, 0.0f);	//フェードアウト
}

/// <summary>
/// シーン終了時の処理
/// </summary>
void
ResultScene::SceneEnd()
{
	_dx12.Fade(func, 0.0f, 1.0f);	//フェードイン
}

/// <summary>
/// 重い処理を実行するための関数
/// </summary>
void
ResultScene::LoadingLoop()
{

}