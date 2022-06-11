#pragma once
#include "Application.h"
#include "Scene.h"

/// <summary>
/// ���U���g�V�[�����Ǘ�����N���X
/// </summary>
class ResultScene : public Scene
{
public:
	//�R���X�g���N�^
	ResultScene(
		Dx12Wrapper& dx12,
		PeraRenderer& pera,
		Renderer& renderer,
		SpriteManager& sprite,
		InputManager& input,
		SoundManager& sound
	);
	~ResultScene();					//�f�R���X�g���N�^

	void Update() override;			//�X�V����

	void SceneStart() override;		//�V�[���J�n���̏���
	void SceneEnd() override;			//�V�[���I�����̏���
private:
	void  LoadingLoop()override;	//�d�����������s���邽�߂̊֐�
};