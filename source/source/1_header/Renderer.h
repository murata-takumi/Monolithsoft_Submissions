#pragma once
#include"Application.h"

/// <summary>
/// �p�C�v���C���X�e�[�g�⃋�[�g�V�O�l�`�����̕`�揈���ɕK�v�ȃI�u�W�F�N�g���Ǘ�����N���X
/// </summary>
class Dx12Wrapper;
class Renderer
{
	SIZE _winSize;	//�E�B���h�E�T�C�Y
	template<typename T>
	using ComPtr = ComPtr<T>;

private:
	Dx12Wrapper& _dx12;	//Dx12Wrapper�C���X�^���X

	ComPtr<ID3D12PipelineState> _pipeline;			//�O���t�B�b�N�X�p�C�v���C���X�e�[�g(�p�C�v���C���ݒ���`����I�u�W�F�N�g)
	ComPtr<ID3D12RootSignature> _rootSignature;		//���[�g�V�O�l�`��(�X���b�g�Ɗe�r���[���Ǘ�����o�b�t�@�[���֘A�t����)

	HRESULT CreateRootSignature();								//���[�g�V�O�l�`���������֐�
	HRESULT CreateGraphicsPipelineForPMD();						//�p�C�v���C���X�e�[�g�������֐�
	bool CheckCompilerResult(HRESULT result, ID3DBlob* error);	//�V�F�[�_�[�ǂݍ��݂̐��ۂ��m�F����֐�

public:
	Renderer(Dx12Wrapper& dx12);				//�R���X�g���N�^
	ID3D12PipelineState* GetPipelineState();	//�p�C�v���C���X�e�[�g��Ԃ��֐�
	ID3D12RootSignature* GetRootSignature();	//���[�g�V�O�l�`����Ԃ��֐�
};