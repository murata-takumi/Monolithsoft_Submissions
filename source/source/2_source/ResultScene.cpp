#include "ResultScene.h"
#include "Dx12Wrapper.h"
#include "PeraRenderer.h"
#include "FBXActor.h"
#include "Renderer.h"
#include "SpriteManager.h"
#include "InputManager.h"
#include "SoundManager.h"

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
/// <param name="renderer">Renderer�C���X�^���X</param>
/// <param name="sprite">SpriteManager�C���X�^���X</param>
/// <param name="input">InputManager�C���X�^���X</param>
/// <param name="sound">SoundManager�C���X�^���X</param>
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
/// �f�R���X�g���N�^
/// </summary>
ResultScene::~ResultScene()
{

}

/// <summary>
/// �X�V����
/// </summary>
void
ResultScene::Update()
{
	_input.UpdateInput();	//���͂��X�V

	_sprite.SetMousePosition(_input.MouseState().x, _input.MouseState().y);	//�}�E�X���W���i�[

	if (_input.MouseTracker().leftButton == Mouse::ButtonStateTracker::PRESSED)
	{
		//�J�n�{�^���̏�ō��N���b�N������Q�[���V�[���ɑJ��
		if (_sprite.TitleIsOnStart())
		{
			_sound.CallButton();					//���ʉ�

			auto& app = Application::Instance();
			app.ChangeScene(SceneNames::Play);
			return;
		}
		//�^�C�g���{�^���̏ゾ������^�C�g����ʂɖ߂�
		else if (_sprite.TitleIsOnEnd())
		{
			_sound.CallButton();					//���ʉ�

			auto& app = Application::Instance();
			app.ChangeScene(SceneNames::Title);
			return;
		}
	}

	_pera.BeginPeraDraw();	//�y���|���S���`�揀��

	_sprite.ResultDraw();	//�^�C�g����ʂł̃{�^���`��

	_sprite.CursorDraw();	//�}�E�X�J�[�\����`��

	//�ȉ��Q�[����ʕ`�揈��
	_dx12.SetPipelines(_renderer.GetRootSignature(), _renderer.GetPipelineState(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_dx12.SetScene();		//�r���[�E�v���W�F�N�V�����s���K�p

	_pera.EndPeraDraw();	//�y���|���S���`���n��

	_dx12.BeginGameDraw();	//�Q�[����ʕ`�揀��

	//�ȉ��y���|���S���p�R�}���h���X�g����
	_pera.SetPeraPipelines();

	_dx12.CommandList()->IASetVertexBuffers(0, 1, &_pera.PeraVBView());		//�y���|���S���pVBV���Z�b�g
	_dx12.CommandList()->DrawInstanced(4, 1, 0, 0);							//�y���|���S�����\�����钸�_��`��

	_dx12.EndGameDraw();	//�Q�[����ʕ`���n��

	_dx12.Swapchain()->Present((UINT)_interval, 0);	//�X���b�v�`�F�[���̃t���b�v����

	_sprite.Commit();	//�O���t�B�b�N�X��������ݒ�
}

/// <summary>
/// �V�[���J�n���̏���
/// </summary>
void
ResultScene::SceneStart()
{
	_dx12.Fade(func, 1.0f, 0.0f);	//�t�F�[�h�A�E�g
}

/// <summary>
/// �V�[���I�����̏���
/// </summary>
void
ResultScene::SceneEnd()
{
	_dx12.Fade(func, 0.0f, 1.0f);	//�t�F�[�h�C��
}

/// <summary>
/// �d�����������s���邽�߂̊֐�
/// </summary>
void
ResultScene::LoadingLoop()
{

}