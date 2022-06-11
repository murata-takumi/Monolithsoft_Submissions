#include "PlayScene.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"

/// <summary>
/// コンストラクタ
/// 最大HPを設定する
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
/// <param name="renderer">Rendererインスタンス</param>
/// <param name="sprite">SpriteManagerインスタンス</param>
/// <param name="input">InputManagerインスタンス</param>
/// <param name="sound">SoundManagerインスタンス</param>
PlayScene::PlayScene(
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
	_maxHP = 300;	//最大HPを設定
}

/// <summary>
/// デコンストラクタ
/// 特に処理はしない
/// </summary>
PlayScene::~PlayScene()
{

}

/// <summary>
/// 更新処理
/// </summary>
void
PlayScene::Update()
{
	_input.UpdateInput();	//入力を更新

	_sprite.SetMousePosition(_input.MouseState().x, _input.MouseState().y);	//マウス座標を更新

	//ロード中でない場合入力を反映する
	if (!_isLoading && !_isNotPlay)
	{
		//右クリックされたらHPを減らす
		if (_input.MouseTracker().rightButton == Mouse::ButtonStateTracker::PRESSED)
		{
			//HPが0になったらリザルトシーンへ遷移
			if ((_currentHP -= 100.0f) <= 0.0f)
			{
				auto& app = Application::Instance();
				app.ChangeScene(SceneNames::Result);
				return;
			}
			//HPが100になったら画面に歪みを加える
			else if (_currentHP == 100.0f)
			{
				_dx12.SetDist(1.0f);
			}

			_sprite.UpdateHP(_currentHP);	//HPを更新させる
		}

		//WASDキーの入力に応じて視点を移動させる
		if (_input.KeyState().A)
		{
			_dx12.RotateSphericalCoordinates(Degree::Azimth, 1);
		}
		if (_input.KeyState().D)
		{
			_dx12.RotateSphericalCoordinates(Degree::Azimth, -1);
		}
		if (_input.KeyState().W)
		{
			_dx12.RotateSphericalCoordinates(Degree::Elevation, 1);
		}
		if (_input.KeyState().S)
		{
			_dx12.RotateSphericalCoordinates(Degree::Elevation, -1);
		}
		if (_input.KeyState().Q)
		{
			_dx12.TranslateSphericalCoordinates(30);
		}
		if (_input.KeyState().E)
		{
			_dx12.TranslateSphericalCoordinates(-30);
		}
	}

	_pera.BeginPeraDraw();									//ペラポリゴン描画準備

	if (!_isLoading) _sprite.BackGroundDraw();				//背景を描画
	else _sprite.LoadingDraw();								//ロード中の画面を描画

	//以下ゲーム画面描画処理
	_dx12.SetPipelines(_renderer.GetRootSignature(), _renderer.GetPipelineState(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_dx12.SetScene();					//ビュー・プロジェクション行列を適用

	if (!_isLoading) _actor->Update();	//FBXモデルの毎フレーム処理

	_pera.EndPeraDraw();				//ペラポリゴン描画後始末

	_dx12.BeginGameDraw();				//ゲーム画面描画準備

	//以下ペラポリゴン用コマンドリスト処理
	_pera.SetPeraPipelines();

	_dx12.CommandList()->IASetVertexBuffers(0, 1, &_pera.PeraVBView());		//ペラポリゴン用VBVをセット
	_dx12.CommandList()->DrawInstanced(4, 1, 0, 0);							//ペラポリゴンを構成する頂点を描画

	if (!_isLoading && !_isNotPlay)_sprite.UIDraw();				//UIを描画

	_sprite.CursorDraw();							//マウスカーソルを描画

	_dx12.EndGameDraw();							//ゲーム画面描画後始末

	_dx12.Swapchain()->Present((UINT)_interval, 0);	//スワップチェーンのフリップ処理

	_sprite.Commit();								//グラフィックスメモリを設定

	/// <summary>
	/// FBX読み込み・その他初期処理
	/// </summary>
	if (_isLoading && !_isNotPlay)LoadingLoop();
}

/// <summary>
/// シーン開始時に実行する関数
/// </summary>
void
PlayScene::SceneStart()
{
	_currentHP = _maxHP;				//現在HPを最大HPで初期化

	_isLoading = true;					//読み込み開始
	_state = LoadingStates::NotStart;	//ロード状態を未開始に設定

	_dx12.Fade(func, 1.0f, 0.0f);		//フェードアウト

	_isNotPlay = false;					//操作可能
}

/// <summary>
/// シーン遷移時に実行する関数
/// </summary>
void
PlayScene::SceneEnd()
{
	_sound.StopBGM();						//BGMを止める

	_isNotPlay = true;						//操作不能にする

	_dx12.Fade(func, 0.0f, 1.0f);			//フェードイン

	_dx12.SetDist(0.0f);					//画面歪みを初期化

	_actor.reset();							//プレイヤーキャラクターを削除
}

/// <summary>
/// 3Dモデルを読み込む関数
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
/// <param name="modelPath">3Dモデルのパス</param>
void
PlayScene::LoadFBXModel(Dx12Wrapper& dx12, const wchar_t* modelPath)
{
	_actor.reset(new FBXActor(dx12, modelPath));		//FBXActorインスタンスを初期化

	_dx12.ResetSphericalCoordinates();					//カメラ位置をリセット
	_sprite.SetMaxHP(_maxHP, _currentHP);				//HPゲージに現在HPを反映

	_state = LoadingStates::Ended;						//ロード状態を終了に設定
}

/// <summary>
/// モデル読み込み用ループ
/// ループ終了時にカメラ位置のリセット・BGM処理などを実行する
/// </summary>
void
PlayScene::LoadingLoop()
{
	/// <summary>
	/// ロード状態に応じて処理を変える
	/// </summary>
	switch (_state)
	{
	case LoadingStates::NotStart:
		//FBXモデルの読み込みを開始
		_th = thread(&PlayScene::LoadFBXModel, this, ref(_dx12), L"6_model/Alicia/FBX/Alicia_solid_Unity.FBX");
		_th.detach();
		_state = LoadingStates::Started;			//ロード状態を開始に設定
		break;
	case LoadingStates::Started:
		break;
	case LoadingStates::Ended:
		_sound.PlayBGM();							//BGM開始
		_isLoading = false;							//読み込みが終了したことを示す
		break;
	default:
		break;
	}
}