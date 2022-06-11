#pragma once
#include "Application.h"
#include <thread>

/// <summary>
/// �e�V�[���I�u�W�F�N�g�̐e�N���X
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
	/// �R���X�g���N�^
	/// �e�C���X�^���X�Ɋi�[����
	/// </summary>
	/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
	/// <param name="pera">PeraRenderer�C���X�^���X</param>
	/// <param name="renderer">Renderer�C���X�^���X</param>
	/// <param name="sprite">SpriteManager�C���X�^���X</param>
	/// <param name="input">InputManager�C���X�^���X</param>
	/// <param name="sound">SoundManager�C���X�^���X</param>
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
		//�t���b�v�Ԋu���擾
		auto& app = Application::Instance();
		_interval = app.GetInterval();

		//�X�V�����̃����_����o�^
		func = [&]() {return Update(); };	
	}

	/// <summary>
	/// �f�R���X�g���N�^
	/// ���ɏ������Ȃ�
	/// </summary>
	virtual ~Scene()
	{

	}

	virtual void Update() = 0;			//�X�V����

	virtual void SceneStart() = 0;		//�V�[���J�n���̏���
	virtual void SceneEnd() = 0;		//�V�[���I�����̏���
protected:
	Dx12Wrapper& _dx12;			//Dx12Wrapper�C���X�^���X
	PeraRenderer& _pera;		//PeraRenderer�C���X�^���X
	Renderer& _renderer;		//Renderer�C���X�^���X
	SpriteManager& _sprite;		//SpriteManager�C���X�^���X
	InputManager& _input;		//InputManager�C���X�^���X
	SoundManager& _sound;		//SoundManager�C���X�^���X

	function<void()> func;		//�X�V�����̃����_��������֐����b�p�[

	thread _th;					//�d���������s�����߂̃X���b�h

	/// <summary>
	/// ���[�h��Ԃ����ʂ��邽�߂̗񋓒l
	/// </summary>
	enum LoadingStates
	{
		NotStart,	//���J�n
		Started,	//�J�n
		Ended,		//�I��
	};
	LoadingStates _state;				//���[�h���

	bool _isLoading;					//���݃��[�h�����ǂ���
	bool _isNotPlay;					//���ݑ���\���ǂ���

	int _interval;						//�t���b�v�Ԋu

	virtual void LoadingLoop() = 0;		//�d�����������s���邽�߂̊֐�
};