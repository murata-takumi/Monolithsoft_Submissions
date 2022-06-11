#include "SoundManager.h"

/// <summary>
/// コンストラクタ
/// 音声データの読み込みを行う
/// </summary>
SoundManager::SoundManager()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;	//フラグを設定

#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif // DEBUG

	_audioEngine = make_unique<AudioEngine>(eflags);										//AudioEngineインスタンスを初期化

	_bgmEffect = make_unique<SoundEffect>(_audioEngine.get(), L"7_sound/neorock.wav");		//BGM読み込み

	_bgm = _bgmEffect->CreateInstance();													//SoundEffectInstanceインスタンスを作成
	_bgm->SetVolume(0.3f);																	//音量を調節

	_buttonEffect = make_unique<SoundEffect>(_audioEngine.get(), L"7_sound/button.wav");	//効果音読み込み
}

/// <summary>
/// デコンストラクタ
/// AudioEngineインスタンスのリセットを行う
/// </summary>
SoundManager::~SoundManager()
{
	_audioEngine->Reset();
}

/// <summary>
/// BGMの再生
/// </summary>
void
SoundManager::PlayBGM()
{
	_bgm->Play(true);
}

/// <summary>
/// BGMのストップ
/// </summary>
void
SoundManager::StopBGM()
{
	_bgm->Stop();
}

/// <summary>
/// ボタン押下時の効果音の再生
/// </summary>
void
SoundManager::CallButton()
{
	_buttonEffect->Play();
}