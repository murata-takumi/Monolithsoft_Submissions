#include "PeraRenderer.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

/// <summary>
/// �y���|���S���pRT�ERTV�ESRV���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
PeraRenderer::CreatePeraResourcesAndView()
{
	auto resDesc = _dx12.BackBuffer()->GetDesc();	//���X�g�p���Ă������\�[�X�ݒ�p�\���̂𗘗p

	//�N���A�l�̓����_�����O���Ɠ����l��ݒ肷��K�v������
	float clsClr[4] = { 1.0f,1.0f,1.0f,1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	//���\�[�X(�����_�[�^�[�Q�b�g)�̍쐬
	auto result = _dx12.Device()->CreateCommittedResource(
		&_defHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&_peraResource));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//RTV�p�q�[�v�p�\���̂̐ݒ�
	auto heapDesc = _dx12.RTVHeap()->GetDesc();
	heapDesc.NumDescriptors = 1;

	//RTV�p�q�[�v�̍쐬
	result = _dx12.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//RTV�p�\���̂̍쐬�E�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC peraRTVDesc = {};
	peraRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	peraRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//RTV�̍쐬
	_dx12.Device()->CreateRenderTargetView(
		_peraResource.Get(),
		&peraRTVDesc,
		_peraRTVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//SRV�p�q�[�v�p�\���̂̐ݒ�
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//SRV�p�q�[�v�̍쐬
	result = _dx12.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//SRV�p�\���̂̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC peraSRVDesc = {};
	peraSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	peraSRVDesc.Format = peraRTVDesc.Format;
	peraSRVDesc.Texture2D.MipLevels = 1;
	peraSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//SRV�̍쐬
	_dx12.Device()->CreateShaderResourceView(
		_peraResource.Get(),
		&peraSRVDesc,
		_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return result;
}

/// <summary>
/// �y���|���S���p���_�o�b�t�@�[�EVBV���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
PeraRenderer::CreatePeraVertex()
{
	//�y���|���S�����쐬���邽�߂̒��_�\����
	struct PeraVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	//�y���|���S�����\������4�̒��_
	PeraVertex pv[] =
	{
		{{-1.0f,-1.0f,0.1f},{0.0f,1.0f}},
		{{-1.0f,1.0f,0.1f},{0.0f,0.0f}},
		{{1.0f,-1.0f,0.1f},{1.0f,1.0f}},
		{{1.0f,1.0f,0.1f},{1.0f,0.0f}}
	};

	auto pvResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));	//���\�[�X�̃T�C�Y��ݒ�

	//�y���|���S���p���_�o�b�t�@�[�̍쐬
	auto result = _dx12.Device()->CreateCommittedResource(
		&_uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&pvResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_peraVB));
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//�y���|���S���̃}�b�v
	PeraVertex* mappedPera = nullptr;
	_peraVB->Map(0, nullptr, (void**)&mappedPera);
	copy(begin(pv), end(pv), mappedPera);
	_peraVB->Unmap(0, nullptr);

	//���_�o�b�t�@�[�r���[�̐ݒ�
	_peraVBV.BufferLocation = _peraVB->GetGPUVirtualAddress();
	_peraVBV.SizeInBytes = sizeof(pv);
	_peraVBV.StrideInBytes = sizeof(PeraVertex);

	return result;
}

/// <summary>
/// �y���|���S���p�p�C�v���C�����쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
PeraRenderer::CreatePeraPipeline()
{
	//�y���|���S���p���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		//���_���W
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		//UV���W
		{
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
	};

	//�y���|���S���p�p�C�v���C���ݒ�\���̂̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);	//���̓��C�A�E�g��
	gpsDesc.InputLayout.pInputElementDescs = layout;	//���̓��C�A�E�g�ݒ�

	ID3DBlob* vs = nullptr;			//���_�V�F�[�_�[�p�f�[�^
	ID3DBlob* ps = nullptr;			//�s�N�Z���V�F�[�_�[�p�f�[�^
	ID3DBlob* errorBlob = nullptr;	//�G���[�m�F�p�f�[�^
	auto result = S_OK;

#ifdef _DEBUG
	//���_�V�F�[�_�[�ǂݍ���
	result = D3DCompileFromFile(L"3_shader/peraVertex.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_5_0", 0, 0,
		&vs,
		&errorBlob);
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//�s�N�Z���V�F�[�_�[�ǂݍ���
	result = D3DCompileFromFile(L"3_shader/peraPixel.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_5_0", 0, 0,
		&ps,
		&errorBlob);
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//�\���̂ɒ��_�E�s�N�Z���V�F�[�_�[��ݒ�
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
#else
	//�\���̂ɒ��_�E�s�N�Z���V�F�[�_�[��ݒ�
	gpsDesc.VS.BytecodeLength = sizeof(g_vs);
	gpsDesc.VS.pShaderBytecode = &g_vs;
	gpsDesc.PS.BytecodeLength = sizeof(g_ps);
	gpsDesc.PS.pShaderBytecode = &g_ps;
#endif // _DEBUG

	//�ȉ��p�C�v���C���X�e�[�g�̐ݒ�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	//�f�B�X�N���v�^�����W
	D3D12_DESCRIPTOR_RANGE range[3] = {};
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;

	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[1].BaseShaderRegister = 1;
	range[1].NumDescriptors = 1;

	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[2].BaseShaderRegister = 0;
	range[2].NumDescriptors = 1;

	//���[�g�p�����[�^
	D3D12_ROOT_PARAMETER rp[3] = {};
	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rp[0].DescriptorTable.pDescriptorRanges = &range[0];
	rp[0].DescriptorTable.NumDescriptorRanges = 1;

	rp[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rp[1].DescriptorTable.pDescriptorRanges = &range[1];
	rp[1].DescriptorTable.NumDescriptorRanges = 1;

	rp[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[2].DescriptorTable.pDescriptorRanges = &range[2];
	rp[2].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);	//�T���v���[

	//���[�g�V�O�l�`���ݒ�\���̂̍쐬
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 3;
	rsDesc.pParameters = rp;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rsBlob = nullptr;	//���[�g�V�O�l�`���p�f�[�^

	//���[�g�V�O�l�`���p�f�[�^�̏�����
	result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rsBlob,
		&errorBlob);
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//���[�g�V�O�l�`���̍쐬
	result = _dx12.Device()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(&_peraRS));
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	rsBlob->Release();	//�s�v�ɂȂ����̂ŊJ��

	gpsDesc.pRootSignature = _peraRS.Get();	//�p�C�v���C���փ��[�g�V�O�l�`����o�^

	result = _dx12.Device()->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_peraPipeline));	//�p�C�v���C���̍쐬
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	return result;
}

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
PeraRenderer::PeraRenderer(Dx12Wrapper& dx12):_dx12(dx12)
{
	CreatePeraResourcesAndView();			//�y���|���S���pRT�ERTV�ESRV���쐬
	 
	CreatePeraVertex();						//�y���|���S���p���_�o�b�t�@�[�EVBV���쐬
	
	CreatePeraPipeline();					//�y���|���S���p�p�C�v���C�����쐬
}

/// <summary>
/// �f�R���X�g���N�^
/// ���ɏ����͂��Ȃ�
/// </summary>
PeraRenderer::~PeraRenderer()
{

}

/// <summary>
/// �y���|���S���p���\�[�X�̑J��(SHADER_RESOURCE��RENDER_TARGET)�ERTV�̃Z�b�g�����s����֐�
/// </summary>
void
PeraRenderer::BeginPeraDraw()
{
	//�y���|���S���p���\�[�X��RENDER_TARGET�ɑJ��
	_dx12.BarrierTransition(_peraResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtvH = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();	//�y���|���S���pRTV/SRV�q�[�v�̃n���h�����擾
	auto dsvH = _dx12.DSVHeap()->GetCPUDescriptorHandleForHeapStart();		//�[�x�X�e���V���q�[�v�̃n���h�����擾

	//�����_�[�^�[�Q�b�g�E�[�x�X�e���V���q�[�v�̃n���h�����R�}���h���X�g�ɐݒ�
	_dx12.CommandList()->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };	//�w�i�F��ݒ�

	//�����_�[�^�[�Q�b�g�E�[�x�X�e���V���r���[���N���A
	_dx12.CommandList()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_dx12.CommandList()->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	_dx12.CommandList()->RSSetViewports(1, _dx12.ViewPort());		//�r���[�|�[�g���R�}���h���X�g�ɐݒ�
	_dx12.CommandList()->RSSetScissorRects(1, _dx12.Rect());		//�V�U�[��`���R�}���h���X�g�ɐݒ�
}

/// <summary>
/// �y���|���S���p���[�g�V�O�l�`���E�p�C�v���C�����Z�b�g����֐�
/// </summary>
void
PeraRenderer::SetPeraPipelines()
{
	//�p�C�v���C���X�e�[�g�A���[�g�V�O�l�`���A�v���~�e�B�u�g�|���W�[��ݒ肷��֐�
	_dx12.SetPipelines(_peraRS.Get(), _peraPipeline.Get(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�y���|���S���pSRV�A�y�уn���h����ݒ�
	ID3D12DescriptorHeap* peraSRVHeaps[] = { _peraSRVHeap.Get() };
	_dx12.CommandList()->SetDescriptorHeaps(1, peraSRVHeaps);
	auto handle = _peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(0, handle);

	//�G�t�F�N�g�pSRV�A�y�уn���h����ݒ�
	ID3D12DescriptorHeap* effectSRVHeap[] = { _dx12.EffectSRVHeap()};
	_dx12.CommandList()->SetDescriptorHeaps(1, effectSRVHeap);
	handle = _dx12.EffectSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(1, handle);

	//�G�t�F�N�g�K�p�f�[�^�pCBV�A�y�уn���h����ݒ�
	ID3D12DescriptorHeap* factorCBVHeap[] = { _dx12.FactorCBVHeap() };
	_dx12.CommandList()->SetDescriptorHeaps(1, factorCBVHeap);
	handle = _dx12.FactorCBVHeap()->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, handle);

	//�f�[�^���X�V
	_dx12.UpdateFactor();
}

/// <summary>
/// �y���|���S���p���\�[�X�̑J��(RENDER_TARGET��SHADER_RESOURCE)�����s����֐�
/// </summary>
void
PeraRenderer::EndPeraDraw()
{
	//�y���|���S���p���\�[�X��SHADER_RESOURCE�ɑJ��
	_dx12.BarrierTransition(_peraResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

/// <summary>
/// �y���|���S���p���_�o�b�t�@�[�r���[��Ԃ��֐�
/// </summary>
/// <returns>�y���|���S���p���_�o�b�t�@�[�r���[</returns>
D3D12_VERTEX_BUFFER_VIEW
PeraRenderer::PeraVBView()
{
	return _peraVBV;
}

/// <summary>
/// �y���|���S���p�p�C�v���C����Ԃ��֐�
/// </summary>
/// <returns>�y���|���S���p�p�C�v���C���̃|�C���^</returns>
ID3D12PipelineState*
PeraRenderer::PeraPipeline()
{
	return _peraPipeline.Get();
}

/// <summary>
/// �y���|���S���p���[�g�V�O�l�`����Ԃ��֐�
/// </summary>
/// <returns>�y���|���S���p���[�g�V�O�l�`���̃|�C���^</returns>
ID3D12RootSignature*
PeraRenderer::PeraRootSignature()
{
	return _peraRS.Get();
}