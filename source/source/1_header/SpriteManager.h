#pragma once
#include "Application.h"
#include <CommonStates.h>
#include <ResourceUploadBatch.h>
#include <SpriteFont.h>
#include <SpriteBatch.h>
#include <sstream>
#include <time.h>

/// <summary>
/// �摜�╶���t�H���g���Ǘ�����N���X
/// </summary>
class Dx12Wrapper;
class SpriteManager
{
	SIZE _winSize;	//�E�B���h�E�T�C�Y
	template<typename T>
	using ComPtr = ComPtr<T>;

private:
	unique_ptr<GraphicsMemory> _gmemory;	//�O���t�B�b�N�X������
	unique_ptr<SpriteFont> _spriteFont;		//�t�H���g�\���p�I�u�W�F�N�g
	unique_ptr<SpriteBatch> _spriteBatch1;	//�X�v���C�g(�摜)�\���p�I�u�W�F�N�g
	unique_ptr<SpriteBatch> _spriteBatch2;	//�X�v���C�g(�摜)�\���p�I�u�W�F�N�g����2

	Dx12Wrapper& _dx12;	//Dx12Wrapper�C���X�^���X

	ID3DBlob* _psBlob = nullptr;	//�s�N�Z���V�F�[�_�[�p�f�[�^

	//��ʂ̕��E����
	LONG _width;
	LONG _height;

	ID3D12DescriptorHeap* _heapForSpriteFont = nullptr;		//�t�H���g�E�摜�p�q�[�v
	D3D12_CPU_DESCRIPTOR_HANDLE _tmpCPUHandle;				//�q�[�v�n���h��(CPU)
	D3D12_GPU_DESCRIPTOR_HANDLE _tmpGPUHandle;				//�q�[�v�n���h��(GPU)
	UINT _incrementSize;										//�n���h���̃A�h���X�̍���

	map<string,D3D12_GPU_DESCRIPTOR_HANDLE> _GPUHandles;		//�n���h��(GPU)�̃x�N�g��

	RECT _backGroundRect;		//�w�i�p��`
	RECT _startButtonRect;		//�^�C�g����ʂł̃X�^�[�g�{�^���p��`
	RECT _endButtonRect;		//�^�C�g����ʂł̏I���{�^���p��`
	RECT _loadingRect;			//���[�h���A�C�R���p��`
	RECT _HPGageRect;			//�̗̓Q�[�W�p��`
	RECT _titleRect;			//�^�C�g���E���U���g��ʂ̕�����p��`

	LONG _buttonLeft,_buttonRight,_titleWidth,_titleHeight;

	ComPtr<ID3D12RootSignature> _spriteRS;					//�X�v���C�g�`��p���[�g�V�O�l�`��

	wstringstream _wss;		//HP�p������

	//HP�̐��l���Q�[�W�ɔ��f�����邽�߂̍\���̂ƃf�[�^
	struct HitPoint
	{
		float hp;
	};
	HitPoint* _mappedHitPoint = nullptr;

	float _maxHp;	//�ő�HP

	//�}�E�X���W
	int _x;
	int _y;

	HRESULT CreateSpriteRS();											//SpriteBatch�p���[�g�V�O�l�`�����쐬����֐�
	HRESULT CreateUIBufferView(const wchar_t* path, string key);		//UI�p�̉摜�̃o�b�t�@�[�E�r���[���쐬����֐�

	XMFLOAT2 GetCenterPos(RECT rect, const wchar_t* wstr, float rectWidth, float rectHeight);	//��`���當�͂𒆉������ɏo������W���擾����

	bool IsInRect(RECT rect);

	void CreateDataBufferView();							//UI���H�p�f�[�^�̃r���[���쐬����֐�
	void InitSpriteDevices();								//�X�v���C�g�E������\���p�I�u�W�F�N�g������������֐�
	void ShiftHandles();									//CPU,GPU�p�n���h�������炷�֐�
public:
	SpriteManager(Dx12Wrapper& dx12,LONG width,LONG height);	//�R���X�g���N�^

	bool TitleIsOnStart();							//�^�C�g����ʂł̃X�^�[�g�{�^���̏�Ƀ}�E�X�����邩�`�F�b�N
	bool TitleIsOnEnd();							//�^�C�g����ʂł̏I���{�^���̏�Ƀ}�E�X�����邩�`�F�b�N

	void AdjustSpriteRect();						//��ʃT�C�Y�̕ύX�����m���ċ�`�𒲐�����
	void SetMaxHP(float maxHP,float currentHP);		//�̗̓Q�[�W�ɔ��f������HP��ݒ�
	void UpdateHP(float currentHP);					//�̗̓Q�[�W���̕������ύX
	void LoadingDraw();								//���[�h��ʂł̕`��
	void TitleDraw();								//�^�C�g����ʂł�UI��`��
	void ResultDraw();								//���U���g��ʂł�UI��`��
	void BackGroundDraw();							//�w�i��`��
	void UIDraw();									//UI��`��
	void CursorDraw();								//�}�E�X�J�[�\����`��
	void Commit();									//�O���t�B�b�N�X���������R�}���h���X�g�ɃZ�b�g
	void SetMousePosition(int x,int y);				//�}�E�X���W���X�V
};