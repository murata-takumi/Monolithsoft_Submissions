#pragma once
#include "Application.h"
#include "Scene.h"

/// <summary>
/// �Q�[���V�[�����Ǘ�����N���X
/// </summary>
class PlayScene :public Scene
{
public:
	//�R���X�g���N�^
	PlayScene(
		Dx12Wrapper& dx12,
		PeraRenderer& pera,
		Renderer& renderer,
		SpriteManager& sprite,
		InputManager& input,
		SoundManager& sound
	);
	~PlayScene();							//�f�R���X�g���N�^

	void Update() override;				//�X�V����

	void SceneStart() override;			//�V�[���J�n���̏���
	void SceneEnd() override;			//�V�[���I�����̏���

protected:
private:
	shared_ptr<FBXActor> _actor;	//�v���C���[�L�����N�^�[

	float _maxHP;					//�ő�HP
	float _currentHP;				//����HP

	void LoadFBXModel(Dx12Wrapper& dx12, const wchar_t* modelPath);		//3D���f���ǂݍ���
	void LoadingLoop() override;										//���f���ǂݍ��ݗp���[�v
};