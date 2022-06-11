#pragma once
#include"Application.h"

/// <summary>
/// 3D���f���y�т��̃f�[�^���Ǘ�����N���X
/// </summary>
class Dx12Wrapper;
class FBXActor
{
	SIZE _winSize;	//�E�B���h�E�T�C�Y
	template<typename T>
	using ComPtr = ComPtr<T>;

private:
	HRESULT result;								//�֐��̕Ԃ�l

	Dx12Wrapper& _dx12;							//Dx12Wrapper�C���X�^���X

	vector<Mesh> _meshes;						//���f���ǂݍ��ݗp���b�V���z��

	vector<ID3D12Resource*> _vertexBuffers;		//���b�V���p���_�o�b�t�@�[�z��
	vector<D3D12_VERTEX_BUFFER_VIEW> _VBViews;	//���b�V���p���_�o�b�t�@�[�r���[�z��

	vector<ID3D12Resource*> _indexBuffers;		//���b�V���p�C���f�b�N�X�o�b�t�@�[�z��
	vector<D3D12_INDEX_BUFFER_VIEW> _IBViews;	//���b�V���p�C���f�b�N�X�o�b�t�@�[�r���[�z��

	ComPtr<ID3D12Resource> _transformBuffer;			//���[���h�s��p�o�b�t�@�[
	ComPtr<ID3D12DescriptorHeap> _transformHeap;		//���[���h�s��p�q�[�v

	ComPtr<ID3D12DescriptorHeap> _texHeap;				//�e�N�X�`���p�q�[�v
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> _GPUHandles;		//�e�N�X�`���o�b�t�@�[�r���[��GPU�n���h���z��

	bool LoadFBXModel(const wchar_t* modelPath);	//FBX���f���ǂݍ��݊֐�

	HRESULT CreateVertexBufferView();				//���_�o�b�t�@�[�E�r���[�쐬�֐�

	HRESULT CreateIndexBufferView();				//�C���f�b�N�X�o�b�t�@�[�E�r���[�쐬�֐�

	HRESULT CreateTransformView();					//���W�ϊ��p�o�b�t�@�[�E�r���[�쐬�֐�

	HRESULT CreateShaderResourceView();				//�V�F�[�_�[���\�[�X�E�r���[�쐬�֐�
public:
	FBXActor(Dx12Wrapper& dx12, const wchar_t* modelPath);	//�R���X�g���N�^

	void Update();	//���t���[���̏���
};