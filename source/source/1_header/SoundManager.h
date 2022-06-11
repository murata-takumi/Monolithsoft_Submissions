#pragma once
#include "Application.h"
#include <Audio.h>

/// <summary>
/// 音声データを管理するクラス
/// </summary>
class SoundManager
{
private:

	unique_ptr<DirectX::AudioEngine> _audioEngine;		//サウンド全体を管理

	unique_ptr<DirectX::SoundEffect> _bgmEffect;		//BGMを読み込む
	unique_ptr<DirectX::SoundEffectInstance> _bgm;		//BGMを鳴らす

	unique_ptr<DirectX::SoundEffect> _buttonEffect;		//ボタン押下時の効果音を読み込む

public:
	
	SoundManager();		//コンストラクタ
	~SoundManager();	//デコンストラクタ

	void PlayBGM();		//BGMを鳴らし始める
	void StopBGM();		//BGMを止める
	void CallButton();	//ボタン押下時の効果音
};