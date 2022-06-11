#pragma once
#include "Application.h"
#include <Audio.h>

/// <summary>
/// �����f�[�^���Ǘ�����N���X
/// </summary>
class SoundManager
{
private:

	unique_ptr<DirectX::AudioEngine> _audioEngine;		//�T�E���h�S�̂��Ǘ�

	unique_ptr<DirectX::SoundEffect> _bgmEffect;		//BGM��ǂݍ���
	unique_ptr<DirectX::SoundEffectInstance> _bgm;		//BGM��炷

	unique_ptr<DirectX::SoundEffect> _buttonEffect;		//�{�^���������̌��ʉ���ǂݍ���

public:
	
	SoundManager();		//�R���X�g���N�^
	~SoundManager();	//�f�R���X�g���N�^

	void PlayBGM();		//BGM��炵�n�߂�
	void StopBGM();		//BGM���~�߂�
	void CallButton();	//�{�^���������̌��ʉ�
};