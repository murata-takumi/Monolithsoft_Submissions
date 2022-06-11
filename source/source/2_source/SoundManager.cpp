#include "SoundManager.h"

/// <summary>
/// �R���X�g���N�^
/// �����f�[�^�̓ǂݍ��݂��s��
/// </summary>
SoundManager::SoundManager()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;	//�t���O��ݒ�

#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif // DEBUG

	_audioEngine = make_unique<AudioEngine>(eflags);										//AudioEngine�C���X�^���X��������

	_bgmEffect = make_unique<SoundEffect>(_audioEngine.get(), L"7_sound/neorock.wav");		//BGM�ǂݍ���

	_bgm = _bgmEffect->CreateInstance();													//SoundEffectInstance�C���X�^���X���쐬
	_bgm->SetVolume(0.3f);																	//���ʂ𒲐�

	_buttonEffect = make_unique<SoundEffect>(_audioEngine.get(), L"7_sound/button.wav");	//���ʉ��ǂݍ���
}

/// <summary>
/// �f�R���X�g���N�^
/// AudioEngine�C���X�^���X�̃��Z�b�g���s��
/// </summary>
SoundManager::~SoundManager()
{
	_audioEngine->Reset();
}

/// <summary>
/// BGM�̍Đ�
/// </summary>
void
SoundManager::PlayBGM()
{
	_bgm->Play(true);
}

/// <summary>
/// BGM�̃X�g�b�v
/// </summary>
void
SoundManager::StopBGM()
{
	_bgm->Stop();
}

/// <summary>
/// �{�^���������̌��ʉ��̍Đ�
/// </summary>
void
SoundManager::CallButton()
{
	_buttonEffect->Play();
}