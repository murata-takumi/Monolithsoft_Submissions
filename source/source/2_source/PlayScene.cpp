#include "PlayScene.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"

/// <summary>
/// �R���X�g���N�^
/// �ő�HP��ݒ肷��
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
/// <param name="renderer">Renderer�C���X�^���X</param>
/// <param name="sprite">SpriteManager�C���X�^���X</param>
/// <param name="input">InputManager�C���X�^���X</param>
/// <param name="sound">SoundManager�C���X�^���X</param>
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
	_maxHP = 300;	//�ő�HP��ݒ�
}

/// <summary>
/// �f�R���X�g���N�^
/// ���ɏ����͂��Ȃ�
/// </summary>
PlayScene::~PlayScene()
{

}

/// <summary>
/// �X�V����
/// </summary>
void
PlayScene::Update()
{
	_input.UpdateInput();	//���͂��X�V

	_sprite.SetMousePosition(_input.MouseState().x, _input.MouseState().y);	//�}�E�X���W���X�V

	//���[�h���łȂ��ꍇ���͂𔽉f����
	if (!_isLoading && !_isNotPlay)
	{
		//�E�N���b�N���ꂽ��HP�����炷
		if (_input.MouseTracker().rightButton == Mouse::ButtonStateTracker::PRESSED)
		{
			//HP��0�ɂȂ����烊�U���g�V�[���֑J��
			if ((_currentHP -= 100.0f) <= 0.0f)
			{
				auto& app = Application::Instance();
				app.ChangeScene(SceneNames::Result);
				return;
			}
			//HP��100�ɂȂ������ʂɘc�݂�������
			else if (_currentHP == 100.0f)
			{
				_dx12.SetDist(1.0f);
			}

			_sprite.UpdateHP(_currentHP);	//HP���X�V������
		}

		//WASD�L�[�̓��͂ɉ����Ď��_���ړ�������
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

	_pera.BeginPeraDraw();									//�y���|���S���`�揀��

	if (!_isLoading) _sprite.BackGroundDraw();				//�w�i��`��
	else _sprite.LoadingDraw();								//���[�h���̉�ʂ�`��

	//�ȉ��Q�[����ʕ`�揈��
	_dx12.SetPipelines(_renderer.GetRootSignature(), _renderer.GetPipelineState(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_dx12.SetScene();					//�r���[�E�v���W�F�N�V�����s���K�p

	if (!_isLoading) _actor->Update();	//FBX���f���̖��t���[������

	_pera.EndPeraDraw();				//�y���|���S���`���n��

	_dx12.BeginGameDraw();				//�Q�[����ʕ`�揀��

	//�ȉ��y���|���S���p�R�}���h���X�g����
	_pera.SetPeraPipelines();

	_dx12.CommandList()->IASetVertexBuffers(0, 1, &_pera.PeraVBView());		//�y���|���S���pVBV���Z�b�g
	_dx12.CommandList()->DrawInstanced(4, 1, 0, 0);							//�y���|���S�����\�����钸�_��`��

	if (!_isLoading && !_isNotPlay)_sprite.UIDraw();				//UI��`��

	_sprite.CursorDraw();							//�}�E�X�J�[�\����`��

	_dx12.EndGameDraw();							//�Q�[����ʕ`���n��

	_dx12.Swapchain()->Present((UINT)_interval, 0);	//�X���b�v�`�F�[���̃t���b�v����

	_sprite.Commit();								//�O���t�B�b�N�X��������ݒ�

	/// <summary>
	/// FBX�ǂݍ��݁E���̑���������
	/// </summary>
	if (_isLoading && !_isNotPlay)LoadingLoop();
}

/// <summary>
/// �V�[���J�n���Ɏ��s����֐�
/// </summary>
void
PlayScene::SceneStart()
{
	_currentHP = _maxHP;				//����HP���ő�HP�ŏ�����

	_isLoading = true;					//�ǂݍ��݊J�n
	_state = LoadingStates::NotStart;	//���[�h��Ԃ𖢊J�n�ɐݒ�

	_dx12.Fade(func, 1.0f, 0.0f);		//�t�F�[�h�A�E�g

	_isNotPlay = false;					//����\
}

/// <summary>
/// �V�[���J�ڎ��Ɏ��s����֐�
/// </summary>
void
PlayScene::SceneEnd()
{
	_sound.StopBGM();						//BGM���~�߂�

	_isNotPlay = true;						//����s�\�ɂ���

	_dx12.Fade(func, 0.0f, 1.0f);			//�t�F�[�h�C��

	_dx12.SetDist(0.0f);					//��ʘc�݂�������

	_actor.reset();							//�v���C���[�L�����N�^�[���폜
}

/// <summary>
/// 3D���f����ǂݍ��ފ֐�
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
/// <param name="modelPath">3D���f���̃p�X</param>
void
PlayScene::LoadFBXModel(Dx12Wrapper& dx12, const wchar_t* modelPath)
{
	_actor.reset(new FBXActor(dx12, modelPath));		//FBXActor�C���X�^���X��������

	_dx12.ResetSphericalCoordinates();					//�J�����ʒu�����Z�b�g
	_sprite.SetMaxHP(_maxHP, _currentHP);				//HP�Q�[�W�Ɍ���HP�𔽉f

	_state = LoadingStates::Ended;						//���[�h��Ԃ��I���ɐݒ�
}

/// <summary>
/// ���f���ǂݍ��ݗp���[�v
/// ���[�v�I�����ɃJ�����ʒu�̃��Z�b�g�EBGM�����Ȃǂ����s����
/// </summary>
void
PlayScene::LoadingLoop()
{
	/// <summary>
	/// ���[�h��Ԃɉ����ď�����ς���
	/// </summary>
	switch (_state)
	{
	case LoadingStates::NotStart:
		//FBX���f���̓ǂݍ��݂��J�n
		_th = thread(&PlayScene::LoadFBXModel, this, ref(_dx12), L"6_model/Alicia/FBX/Alicia_solid_Unity.FBX");
		_th.detach();
		_state = LoadingStates::Started;			//���[�h��Ԃ��J�n�ɐݒ�
		break;
	case LoadingStates::Started:
		break;
	case LoadingStates::Ended:
		_sound.PlayBGM();							//BGM�J�n
		_isLoading = false;							//�ǂݍ��݂��I���������Ƃ�����
		break;
	default:
		break;
	}
}