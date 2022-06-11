#include "AssimpLoader.h"
#include "Dx12Wrapper.h"
#include "FBXActor.h"
#include "Functions.h"

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="dx12">Dx12Wrapper�C���X�^���X</param>
/// <param name="modelPath">FBX�t�@�C���̃p�X</param>
FBXActor::FBXActor(Dx12Wrapper& dx12, const wchar_t* modelPath):_dx12(dx12)
{
	result = S_OK;		//�֐��̕Ԃ�l��������

	LoadFBXModel(modelPath);	//FBX���f���ǂݍ���

	CreateVertexBufferView();	//���_�o�b�t�@�[�E�r���[�쐬

	CreateIndexBufferView();	//�C���f�b�N�X�o�b�t�@�[�E�r���[�쐬

	CreateTransformView();		//���W�ϊ��p�o�b�t�@�[�E�r���[�쐬

	CreateShaderResourceView();	//�V�F�[�_�[���\�[�X�E�r���[�쐬
}

/// <summary>
/// FBX���f����ǂݍ��ފ֐�
/// </summary>
/// <param name="modelPath">FBX���f���̃t�@�C���p�X</param>
/// <returns>�����������������ǂ���</returns>
bool
FBXActor::LoadFBXModel(const wchar_t* modelPath)
{
	//���f���ǂݍ��ݗp�ݒ�
	ImportSettings settings =
	{
		modelPath,
		_meshes,
		false,
		true,
	};

	//FBX���f����ǂݍ���
	AssimpLoader loader;
	if (!loader.Load(settings))
	{
		assert(0);
		return false;
	}

	return true;
}

/// <summary>
/// ���b�V�����ɒ��_�o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <returns>�����������������ǂ���</returns>
HRESULT 
FBXActor::CreateVertexBufferView()
{
	result = S_OK;	//�Ԃ�l��������

	//���_�o�b�t�@�[�E�r���[��p�ӂ���
	_vertexBuffers.reserve(_meshes.size());
	_VBViews.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpVertexBuffer = nullptr;							//�i�[�p�o�b�t�@�[
		D3D12_VERTEX_BUFFER_VIEW tmpVBView = {};							//�i�[�p�r���[

		auto size = sizeof(FBXVertex) * _meshes[i].vertices.size();			//���_�S�̂̃f�[�^�T�C�Y
		auto stride = sizeof(FBXVertex);									//���_��̃f�[�^�T�C�Y

		auto vertexResDesc = CD3DX12_RESOURCE_DESC::Buffer(size);					//���\�[�X�ݒ�

		//�o�b�t�@�[�쐬
		result = _dx12.Device()->CreateCommittedResource
		(
			&_uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&vertexResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&tmpVertexBuffer)
		);
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		auto data = _meshes[i].vertices;	//���_�f�[�^�擾

		//�f�[�^���o�b�t�@�[�֏�������
		FBXVertex* mappedVertex = nullptr;
		tmpVertexBuffer->Map(0, nullptr, (void**)&mappedVertex);
		copy(begin(data), end(data), mappedVertex);
		tmpVertexBuffer->Unmap(0, nullptr);

		//�r���[�Ƀo�b�t�@�[������������
		tmpVBView.BufferLocation = tmpVertexBuffer->GetGPUVirtualAddress();
		tmpVBView.SizeInBytes = (UINT)tmpVertexBuffer->GetDesc().Width;
		tmpVBView.StrideInBytes = sizeof(FBXVertex);

		//���_�o�b�t�@�[�E�r���[��z��ɒǉ�
		_vertexBuffers.push_back(tmpVertexBuffer);
		_VBViews.push_back(tmpVBView);

	}

	return result;
}

/// <summary>
/// ���b�V�����ɃC���f�b�N�X�o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <returns></returns>
HRESULT
FBXActor::CreateIndexBufferView()
{
	result = S_OK;	//�Ԃ�l��������

	//�C���f�b�N�X�o�b�t�@�[�E�r���[��p�ӂ���
	_indexBuffers.reserve(_meshes.size());
	_IBViews.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpIndexBuffer = nullptr;	//�i�[�p�o�b�t�@�[
		D3D12_INDEX_BUFFER_VIEW tmpIBView = {};		//�i�[�p�r���[

		auto size = sizeof(uint32_t) * _meshes[i].indices.size();	//�C���f�b�N�X�S�̂̃f�[�^�T�C�Y

		auto indexResDesc = CD3DX12_RESOURCE_DESC::Buffer(size);					//���\�[�X�ݒ�

		//�o�b�t�@�[�쐬
		result = _dx12.Device()->CreateCommittedResource
		(
			&_uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&indexResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&tmpIndexBuffer)
		);
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		auto data = _meshes[i].indices;	//�C���f�b�N�X�f�[�^

		//�f�[�^���o�b�t�@�[�ɏ�������
		uint32_t* mappedIndex = nullptr;
		tmpIndexBuffer->Map(0, nullptr, (void**)&mappedIndex);
		copy(begin(data), end(data), mappedIndex);
		tmpIndexBuffer->Unmap(0, nullptr);

		//�r���[�Ƀo�b�t�@�[������������
		tmpIBView.BufferLocation = tmpIndexBuffer->GetGPUVirtualAddress();
		tmpIBView.Format = DXGI_FORMAT_R32_UINT;
		tmpIBView.SizeInBytes = static_cast<UINT>(size);

		//�C���f�b�N�X�o�b�t�@�[�E�r���[��z��ɒǉ�
		_indexBuffers.push_back(tmpIndexBuffer);
		_IBViews.push_back(tmpIBView);
	}

	return result;
}

/// <summary>
/// ���f���p�g�����X�t�H�[���o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
FBXActor::CreateTransformView()
{
	result = S_OK;

	//���[���h�s��p�o�b�t�@�[�̍쐬
	auto transformResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + _constSize) & ~_constSize);
	result = _dx12.Device()->CreateCommittedResource
	(
		&_uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&transformResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_transformBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	//�f�[�^�̏�������
	XMMATRIX* mappedMat = nullptr;
	_transformBuffer->Map(0, nullptr, (void**)&mappedMat);
	*mappedMat = XMMatrixIdentity();

	//�f�B�X�N���v�^�q�[�v�ݒ�p�\���̂̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;									//�Ƃ肠�������[���h�ЂƂ�
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;
	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���

	//�q�[�v�̍쐬
	result = _dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf()));//����
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	//�r���[�ݒ�p�\���̂̍쐬
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_transformBuffer->GetDesc().Width;

	//�r���[�̍쐬
	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// ���f���p�e�N�X�`���o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
FBXActor::CreateShaderResourceView()
{
	result = S_OK;

	//�e�N�X�`���p�f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NodeMask = 1;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.NumDescriptors = (UINT)_meshes.size();
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	_dx12.Device()->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(_texHeap.ReleaseAndGetAddressOf()));

	//�e�N�X�`���o�b�t�@�[�r���[�p�\���̂̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto CPUHeapHandle = _texHeap->GetCPUDescriptorHandleForHeapStart();							//�q�[�v�̐擪�A�h���X(CPU)
	auto GPUHeapHandle = _texHeap->GetGPUDescriptorHandleForHeapStart();							//�q�[�v�̐擪�A�h���X(GPU)	
	auto incrementSize = _dx12.Device()->GetDescriptorHandleIncrementSize(texHeapDesc.Type);	//�A�h���X�̂��炷��

	//�e�N�X�`���ǂݍ��ݗp�f�[�^
	TexMetadata meta = {};
	ScratchImage scratch = {};

	DXGI_FORMAT format;		//�e�N�X�`���̃t�H�[�}�b�g
	size_t width;			//��
	size_t height;			//����
	UINT16 arraySize;		//�e�N�X�`���T�C�Y
	UINT16 mipLevels;
	void* pixels;
	UINT rowPitch;
	UINT slicePitch;

	//�e�N�X�`���̓ǂݍ��ݏ���
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpTexBuffer = nullptr;

		auto path = _meshes[i].diffuseMap;	//�e�N�X�`���̃p�X

		//���e�N�X�`��
		if (wcscmp(path.c_str(), L"") == 0)
		{
			vector<unsigned char> data(4 * 4 * 4);
			fill(data.begin(), data.end(), 0x00);

			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			width = 4;
			height = 4;
			arraySize = 1;
			mipLevels = 1;

			pixels = data.data();
			rowPitch = 4 * 4;
			slicePitch = (UINT)data.size();
		}
		//�ʏ�e�N�X�`��
		else
		{
			auto ext = FileExtension(path);		//�g���q���擾
			
			//�g���q��"psd"�������ꍇ�A"tga"�ɕϊ�����
			if (wcscmp(ext.c_str(), L"psd") == 0)
			{
				path = ReplaceExtension(path, "tga");	//�p�X�̊g���q��ϊ�
				ext = FileExtension(path);				//�g���q���擾���Ȃ���
			}

			result = _dx12._loadLambdaTable[ToString(ext)](path, &meta, scratch);	//�g���q�ɉ����ēǂݍ��݊֐���ς���
			if (FAILED(result))
			{
				assert(0);
				return result;
			}

			//�e�N�X�`���f�[�^�̗p��
			auto img = scratch.GetImage(0, 0, 0);

			format = meta.format;
			width = meta.width;
			height = meta.height;
			arraySize = static_cast<UINT16>(meta.arraySize);
			mipLevels = static_cast<UINT16>(meta.mipLevels);

			pixels = img->pixels;
			rowPitch = static_cast<UINT>(img->rowPitch);
			slicePitch = static_cast<UINT>(img->slicePitch);
		}
		
		auto shaderResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			format,
			(UINT)width,
			(UINT)height,
			arraySize,
			(UINT)mipLevels);

		//�V�F�[�_�[���\�[�X�̍쐬
		result = _dx12.Device()->CreateCommittedResource
		(
			&_writeHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&shaderResDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&tmpTexBuffer)
		);
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		//�e�N�X�`���̏�������
		result = tmpTexBuffer->WriteToSubresource(0,
			nullptr,
			pixels,
			rowPitch,
			slicePitch
		);
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		srvDesc.Format = tmpTexBuffer->GetDesc().Format;									//�t�H�[�}�b�g�����킹��
		_dx12.Device()->CreateShaderResourceView(tmpTexBuffer, &srvDesc, CPUHeapHandle);	//�e�N�X�`���o�b�t�@�[�r���[���쐬

		_GPUHandles.push_back(GPUHeapHandle);	//GPU�̃A�h���X��ǉ�

		CPUHeapHandle.ptr += incrementSize;	//CPU�̃A�h���X�����炷
		GPUHeapHandle.ptr += incrementSize;	//GPU�̃A�h���X�����炷
	}

	return result;
}

/// <summary>
/// ���t���[���̍X�V����
/// </summary>
void
FBXActor::Update()
{
	//���W�ϊ��p�f�B�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* transformHeaps[] = { _transformHeap.Get() };
	_dx12.CommandList()->SetDescriptorHeaps(1, transformHeaps);	

	//���[�g�p�����[�^�ƃf�B�X�N���v�^�q�[�v�̃n���h�����֘A�t��
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(1, _transformHeap->GetGPUDescriptorHandleForHeapStart());

	//���_�E�C���f�b�N�X�o�b�t�@�[�r���[�̃Z�b�g�A���_�`��
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		_dx12.CommandList()->IASetVertexBuffers(0, 1, &_VBViews[i]);
		_dx12.CommandList()->IASetIndexBuffer(&_IBViews[i]);

		//�e�N�X�`���o�b�t�@�[�r���[�̃Z�b�g

		ID3D12DescriptorHeap* SetTexHeap[] = { _texHeap.Get() };
		_dx12.CommandList()->SetDescriptorHeaps(1, SetTexHeap);
		_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, _GPUHandles[i]);

		_dx12.CommandList()->DrawIndexedInstanced((UINT)_meshes[i].indices.size(), 1, 0, 0, 0);
	}
}