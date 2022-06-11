#pragma once
#include "Application.h"

/// <summary>
/// �y���|���S���Ɋւ���I�u�W�F�N�g�A�������Ǘ�����N���X
/// </summary>
class Dx12Wrapper;
class PeraRenderer
{
private:
	Dx12Wrapper& _dx12;		//Dx12Wrapper�C���X�^���X

	//�y���|���S������
	ComPtr<ID3D12Resource> _peraVB;					//�y���|���S���p�̒��_�o�b�t�@�[
	ComPtr<ID3D12Resource> _peraResource;			//RTV/SRV�������������ނ��߂̃��\�[�X
	ComPtr<ID3D12DescriptorHeap> _peraRTVHeap;		//RTV�p�q�[�v
	ComPtr<ID3D12DescriptorHeap> _peraSRVHeap;		//SRV�p�q�[�v
	ComPtr<ID3D12PipelineState> _peraPipeline;		//�y���|���S���p�̃p�C�v���C���X�e�[�g
	ComPtr<ID3D12RootSignature> _peraRS;			//�y���|���S���p�̃��[�g�V�O�l�`��

	D3D12_VERTEX_BUFFER_VIEW _peraVBV = {};		//�y���|���S���p�̒��_�o�b�t�@�[�r���[

	HRESULT CreatePeraResourcesAndView();		//�y���|���S���pRT�ESR�ERTV�ESRV���쐬����֐�

	HRESULT CreatePeraVertex();					//�y���|���S���p���_�o�b�t�@�[�EVBV���쐬����֐�

	HRESULT CreatePeraPipeline();				//�y���|���S���p�p�C�v���C�����쐬����֐�

public:

	PeraRenderer(Dx12Wrapper& dx12);	//�R���X�g���N�^
	~PeraRenderer();					//�f�R���X�g���N�^

	void BeginPeraDraw();		//�y���|���S���p���\�[�X�̑J��(SHADER_RESOURCE��RENDER_TARGET)�ERTV�̃Z�b�g�����s����֐�
	void SetPeraPipelines();	//�y���|���S���p���[�g�V�O�l�`���E�p�C�v���C�����Z�b�g����֐�
	void EndPeraDraw();			//�y���|���S���p���\�[�X�̑J��(RENDER_TARGET��SHADER_RESOURCE)�����s����֐�

	D3D12_VERTEX_BUFFER_VIEW PeraVBView();		//�y���|���S���p���_�o�b�t�@�[�r���[��Ԃ��֐�
	ID3D12PipelineState* PeraPipeline();		//�y���|���S���p�p�C�v���C����Ԃ��֐�
	ID3D12RootSignature* PeraRootSignature();	//�y���|���S���p���[�g�V�O�l�`����Ԃ��֐�
};