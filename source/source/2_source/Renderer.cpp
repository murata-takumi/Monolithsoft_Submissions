#include "Renderer.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

/// <summary>
/// �R���X�g���N�^
/// :_dx12(dx12)��_dx12�ϐ��Ɉ������i�[���Ă���̂�����
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
Renderer::Renderer(Dx12Wrapper& dx12):_dx12(dx12)
{
	CreateRootSignature();						//���[�g�V�O�l�`��������
	CreateGraphicsPipelineForPMD();				//�p�C�v���C���X�e�[�g������
}

/// <summary>
/// ���[�g�V�O�l�`��������������֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Renderer::CreateRootSignature()
{
	//�V�F�[�_�[�̃X���b�g�ƕR�t���f�B�X�N���v�^�����W�̍쐬
	CD3DX12_DESCRIPTOR_RANGE descTblRange[3] = {};
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	//�萔[b0]
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	//�萔[b1]
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	//�e�N�X�`��[t0]


	//�o�b�t�@�[�ƃf�B�X�N���v�^�����W��R�Â��郋�[�g�p�����[�^�̍쐬
	CD3DX12_ROOT_PARAMETER rootparam[3] = {};
	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);
	rootparam[1].InitAsDescriptorTable(1, &descTblRange[1]);
	rootparam[2].InitAsDescriptorTable(1, &descTblRange[2], D3D12_SHADER_VISIBILITY_PIXEL);

	//�T���v���[(�e�N�X�`����XY���W�̎擾���@�����߂�I�u�W�F�N�g)�̍쐬
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs = {};
	samplerDescs.Init(0);

	//�ݒ�p�\���̂̍쐬�E�ݒ�
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(3,rootparam,1,&samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//���[�g�V�O�l�`���̏�����
	ID3DBlob* rootSigBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�쐬
	result = _dx12.Device()->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(_rootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	rootSigBlob->Release();		//�s�v�ɂȂ����f�[�^���J��

	return result;
}

/// <summary>
/// �O���t�B�b�N�X�p�C�v���C�����쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Renderer::CreateGraphicsPipelineForPMD()
{
	ID3DBlob* vsBlob = nullptr;		//���_�V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob* psBlob = nullptr;		//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob* errorBlob = nullptr;	//�G���[���ʗp�I�u�W�F�N�g

	auto result = S_OK;

	//���̓��C�A�E�g(GPU�ɒ��_�f�[�^���ǂ����߂��邩��������f�[�^)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeline = {};			//�p�C�v���C���X�e�[�g�ݒ�p�\����
	gPipeline.pRootSignature = _rootSignature.Get();			//���[�g�V�O�l�`���Z�b�g

#ifdef _DEBUG
		//���_�V�F�[�_�[�ǂݍ���
	result = D3DCompileFromFile(
		L"3_shader/FBXVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"FBXVS", "vs_5_0",
		0,
		0,
		&vsBlob, &errorBlob);
	if (!CheckCompilerResult(result, errorBlob))
	{
		assert(0);
		return result;
	}

	//�s�N�Z���V�F�[�_�[�ǂݍ���
	result = D3DCompileFromFile(
		L"3_shader/FBXPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"FBXPS", "ps_5_0",
		0,
		0,
		&psBlob, &errorBlob);
	if (!CheckCompilerResult(result, errorBlob))
	{
		assert(0);
		return result;
	}

	gPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob);				//���_�V�F�[�_�[�Z�b�g
	gPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob);				//�s�N�Z���V�F�[�_�[�Z�b�g

#else
	gPipeline.VS.BytecodeLength = sizeof(g_FBXVS);				//���_�V�F�[�_�[�Z�b�g
	gPipeline.VS.pShaderBytecode = &g_FBXVS;
	gPipeline.PS.BytecodeLength = sizeof(g_FBXPS);				//���_�V�F�[�_�[�Z�b�g
	gPipeline.PS.pShaderBytecode = &g_FBXPS;
#endif // DEBUG

	gPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;			//�T���v���}�X�N

	gPipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);	//�u�����h�X�e�[�g
	
	gPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);	//���X�^���C�U�[�X�e�[�g(�|���S�����s�N�Z���ɕϊ�����d�g��)
	gPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;			//�J�����O���[�h(�`��s�v�ȃ|���S����`�悵�Ȃ��悤�d�g��)

	//�[�x�l����̐ݒ�
	gPipeline.DepthStencilState.DepthEnable = true;								//�[�x�l��L����
	gPipeline.DepthStencilState.StencilEnable = false;							//�X�e���V���l�͖����̂܂�
	gPipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�s�N�Z���`�掞�ɐ[�x�o�b�t�@�[�ɐ[�x�l���������ނ悤�ݒ�
	gPipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;			//�s�N�Z�����m�̐[�x���r���A����������`��

	gPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//32bit�l��[�x�l�Ƃ��Ďg�p����

	gPipeline.InputLayout.pInputElementDescs = inputLayout;		//���̓��C�A�E�g�ݒ�
	gPipeline.InputLayout.NumElements = _countof(inputLayout);	//���C�A�E�g�z��̗v�f��

	gPipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	//�|���S���̐؂����
	gPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//�|���S�����O�p�`�ŕ\������

	gPipeline.NumRenderTargets = 1;								//�����_�[�^�[�Q�b�g��
	gPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//�����_�[�^�[�Q�b�g�̃t�H�[�}�b�g

	gPipeline.SampleDesc.Count = 1;		//�s�N�Z��������T���v�����O��
	gPipeline.SampleDesc.Quality = 0;	//�N�I���e�B

	result = _dx12.Device()->CreateGraphicsPipelineState(&gPipeline, IID_PPV_ARGS(_pipeline.ReleaseAndGetAddressOf()));	//�p�C�v���C���X�e�[�g�쐬

	return result;	//�֐������������̂�Ԃ�
}

/// <summary>
/// �V�F�[�_�[�ǂݍ��݂������������ǂ����𔻕ʂ���֐�
/// </summary>
/// <param name="result">�V�F�[�_�[�ǂݍ��݊֐��̕Ԃ�l</param>
/// <param name="error">�G���[�f�[�^�i�[�p�I�u�W�F�N�g</param>
/// <returns>�����������ǂ���</returns>
bool
Renderer::CheckCompilerResult(HRESULT result, ID3DBlob* error)
{
	//�֐������s������
	if (FAILED(result))
	{
		//�G���[��ނ��ȉ��̂��̂�������t�@�C����������Ȃ��|���L�q
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("�t�@�C����������܂���");
		}
		else
		{
			string errStr;							//�G���[���L�q�p������
			errStr.resize(error->GetBufferSize());	//������̃T�C�Y��ύX

			//�I�u�W�F�N�g�̃A�h���X����o�b�t�@�[�T�C�Y���f�[�^���擾���A������ɃR�s�[����
			copy_n((char*)error->GetBufferPointer(),error->GetBufferSize(),errStr.begin());

			errStr += "\n";							//�����ɉ��s������������
			OutputDebugStringA(errStr.c_str());		//�o��
		}
		return false;	//�֐������s������U��Ԃ�
	}
	else
	{
		return true;	//�֐�������������^��Ԃ�
	}
}

/// <summary>
///	�p�C�v���C���X�e�[�g��Ԃ��֐�
/// </summary>
/// <returns>�p�C�v���C���X�e�[�g</returns>
ID3D12PipelineState*
Renderer::GetPipelineState()
{
	return _pipeline.Get();
}

/// <summary>
/// ���[�g�V�O�l�`����Ԃ��֐�
/// </summary>
/// <returns>���[�g�V�O�l�`��</returns>
ID3D12RootSignature*
Renderer::GetRootSignature()
{
	return _rootSignature.Get();
}