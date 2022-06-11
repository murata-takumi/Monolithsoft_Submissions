#pragma once
#include <d3dcompiler.h>
#include <map>
#include <tchar.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXTex.h>
#include <fbxsdk.h>
#include <vector>

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace fbxsdk;

#pragma comment(lib,"DirectXTK12.lib")
#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

/// <summary>
/// FBX���f���̒��_�p�\����
/// </summary>
struct FBXVertex
{
	XMFLOAT3 position;	//���W
	XMFLOAT3 normal;	//�@��
	XMFLOAT2 uv;		//UV���W
	XMFLOAT3 tangent;	//����
	XMFLOAT4 color;		//���_�J���[
};

/// <summary>
/// FBX���f�����\�����郁�b�V���̍\����
/// </summary>
struct Mesh
{
	vector<FBXVertex> vertices;	//���_�x�N�g��
	vector<uint32_t> indices;	//�C���f�b�N�X�x�N�g��
	wstring diffuseMap;			//�e�N�X�`���̃t�@�C���p�X
};

/// <summary>
/// �ǂݍ���Scene�N���X�����ʂ��邽�߂̗񋓌^
/// </summary>
enum SceneNames
{
	Title,		//�^�C�g���V�[��
	Play,		//�Q�[���V�[��
	Result,		//���U���g�V�[��
};

//�ȉ��̃N���X�錾��Application�̃N���X��`�Ŋe�N���X���Ăяo����悤�ɂ��邽�ߋL�q
class Dx12Wrapper;
class PeraRenderer;
class Renderer;
class FBXActor;
class SpriteManager;
class InputManager;
class SoundManager;
class Scene;
class TitleScene;
class PlayScene;
class ResultScene;
/// <summary>
/// �Q�[���̏������E�X�V�E�I�����Ǘ�����N���X
/// </summary>
class Application
{
private:
	WNDCLASSEX _windowClass;					//�E�B���h�E�쐬���ɕK�v�ȏ����i�[
	HWND _hwnd;									//�E�B���h�E�̎��ʂɕK�v�Ȓl

	//shared_ptr<class>�͎g�p�҂����Ȃ��Ȃ����������I�Ƀ��\�[�X���J������X�}�[�g�|�C���^
	shared_ptr<Dx12Wrapper> _dx12;				//Dx12Wrapper�C���X�^���X
	shared_ptr<PeraRenderer> _pera;				//PeraRenderer�C���X�^���X
	shared_ptr<Renderer> _renderer;				//Renderer�C���X�^���X
	shared_ptr<FBXActor> _actor;				//FBXActor�C���X�^���X
	shared_ptr<SpriteManager> _sprite;			//DXTK12Manager�C���X�^���X
	shared_ptr<InputManager> _input;			//InputManager�C���X�^���X	
	shared_ptr<SoundManager> _sound;			//SoundManager�C���X�^���X

	shared_ptr<TitleScene> _title;				//TitleScene�C���X�^���X
	shared_ptr<PlayScene> _play;				//PlayScene�C���X�^���X
	shared_ptr<ResultScene> _result;			//ResultScene�C���X�^���X

	Scene* _Scene;								//�X�V�������s��Scene�I�u�W�F�N�g

	int _rate;			//1�b������̃t���[����
	int _interval;		//�t���b�v�Ԋu
	float _deltaTime;	//�t���[���Ԃ̌o�ߎ���

	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);		//�Q�[���p�E�B���h�E���쐬����֐�

	Application();													//�R���X�g���N�^

	//�R���X�g���N�^���O������Ăяo����Ȃ��悤�ݒ�
	Application(const Application&) = delete;
	void operator = (const Application&) = delete;
	
public:

	MSG _msg = {};								//���b�Z�[�W�p�\����

	static Application& Instance();		//�C���X�^���X��Ԃ��H

	bool Init();						//������

	void Run();							//�Q�[����ʂ̕`��

	void Terminate();					//�Q�[���I�����̌�n��

	SIZE GetWindowSize()const;			//�E�B���h�E�T�C�Y��Ԃ�

	void SetScene(Scene* scene);		//�V�[����؂�ւ���
	void ChangeScene(SceneNames name);	//�V�[���̐؂�ւ�
	void ExitApp();						//�A�v���P�[�V�������I������

	int GetInterval();					//�����_�[�^�[�Q�b�g�̃t���b�v�Ԋu��Ԃ��֐�
	int GetRate();						//1�b�Ԃ̃t���[�����[�g��Ԃ��֐�

	~Application();						//�f�R���X�g���N�^
};
