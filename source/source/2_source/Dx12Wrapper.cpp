#include "Dx12Wrapper.h"
#include "Functions.h"
#include "SphericalCoordinates.h"

const int FADE_TIME = 80;								//�t�F�[�h�C���^�A�E�g�ɂ����鎞��
const float FADE_DIFF = (float)1 / (float)FADE_TIME;	//�t�F�[�h���̐؂�ւ��Ԋu

/// <summary>
/// �f�o�b�O�p���C���[������������֐�
/// </summary>
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));
		
	debugLayer->EnableDebugLayer();	//�f�o�b�O���C���[��L��������
	debugLayer->Release();			//�L����������C���^�[�t�F�C�X���J������
}

/// <summary>
/// �f�R���X�g���N�^
/// ���ɏ����͍s��Ȃ�
/// </summary>
Dx12Wrapper::~Dx12Wrapper()
{

}

/// <summary>
/// �f�o�C�X�֘A������������֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Dx12Wrapper::InitializeDXGIDevice()
{
	//Debug�������łȂ����Ńt�@�N�g���[�̍쐬�֐���ς���
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG
	
	//�t�B�[�`���[���x���̔z���������
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	vector<IDXGIAdapter*> adapters;			//�񋓂����A�_�v�^�[���i�[
	IDXGIAdapter* tmpAdapter = nullptr;		//�A�_�v�^�[

	//�t�@�N�g���[���̑S�A�_�v�^�[�ɑ΂����[�v
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);	//�A�_�v�^�[���x�N�g���Ɋi�[
	}

	//�i�[�����S�A�_�v�^�[�ɑ΂����[�v
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);					//�A�_�v�^�[�̐������L�q����\���̂��擾

		wstring strDesc = adesc.Description;	//�A�_�v�^�[�̖��O���擾

		//�A�_�v�^�[�̖��O������̖��O�ƈ�v�����烋�[�v�I��
		if (strDesc.find(L"NVIDIA") != string::npos)
		{
			tmpAdapter = adpt;	//�f�o�C�X�쐬�Ɏg�p����A�_�v�^�[������
			break;				//���[�v�I��
		}
	}

	//�����������t�B�[�`���[���x���ɑ΂����[�v
	for (auto lv : levels)
	{
		//�f�o�C�X���쐬�ł���t�B�[�`���[���x�������������炻�̂܂܍쐬���ă��[�v�I��
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			result = S_OK;		//�쐬�������������Ƃ�Ԃ�
			break;				//�����\�ȃo�[�W���������������烋�[�v�I��
		}
	}
	return result;
}

/// <summary>
/// �R�}���h�֘A������������֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Dx12Wrapper::InitializeCommand()
{
	//�R�}���h�A���P�[�^���쐬
	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));

	//�R�}���h���X�g���쐬
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(), nullptr,
		IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));

	//�R�}���h�L���[�ݒ�\���̂��쐬�E�ݒ�
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	//�R�}���h�L���[���쐬
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));

	return result;	//�����̐�����Ԃ�
}

/// <summary>
/// �X���b�v�`�F�[�����쐬����֐�
/// </summary>
/// <param name="hwnd">�E�B���h�E���ʗp�f�[�^</param>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Dx12Wrapper::CreateSwapChain(const HWND& hwnd)
{
	//�X���b�v�`�F�[���ݒ�p�\���́E�ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = _winSize.cx;
	swapchainDesc.Height = _winSize.cy;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;				
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//�X���b�v�`�F�[���쐬
	auto result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.ReleaseAndGetAddressOf());

	return result;
}

/// <summary>
/// �����_�[�^�[�Q�b�g���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT 
Dx12Wrapper::CreateRenderTargetsView()
{
	//�����_�[�^�[�Q�b�g�p�f�B�X�N���v�^�q�[�v�ݒ�\���̂̍쐬�E�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//�����_�[�^�[�Q�b�g�p�f�B�X�N���v�^�q�[�v�̍쐬
	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_rtvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�X���b�v�`�F�[���̏��擾
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�o�b�N�o�b�t�@�[�̃T�C�Y���X���b�v�`�F�[���̃o�b�t�@�[���ɍ��킹��
	_backBuffers.resize(swcDesc.BufferCount);

	//�f�B�X�N���v�^�q�[�v�̐擪�A�h���X(�n���h��)
	D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	//�����_�[�^�[�Q�b�g�r���[�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//�e�o�b�N�o�b�t�@�[�ɑ΂����[�v����
	for (int idx = 0; idx < (int)swcDesc.BufferCount; ++idx)
	{
		//�X���b�v�`�F�[������f�[�^���擾�A�o�b�N�o�b�t�@�[�Ɋ��蓖�Ă�
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));	
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		//�o�b�N�o�b�t�@�[��ΏۂɃ����_�[�^�[�Q�b�g�r���[���쐬
		_dev->CreateRenderTargetView(
			_backBuffers[idx],
			&rtvDesc,
			handle);

		//�n���h���̈ʒu�����炷
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//�X���b�v�`�F�[���̏����擾(��L��swcDesc�Ƃ͕ʕ�)
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	result = _swapchain->GetDesc1(&desc);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�X���b�v�`�F�[���̏������Ƀr���[�|�[�g�A�V�U�[��`��������
	_viewPort.reset(new CD3DX12_VIEWPORT(_backBuffers[0]));
	_rect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));

	return result;	//�֐������������̂�Ԃ�
}

/// <summary>
/// �r���[�v���W�F�N�V�����p�r���[���쐬
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT 
Dx12Wrapper::CreateSceneView()
{
	auto sceneResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + _constSize) & ~_constSize);

	//�萔�o�b�t�@�[(���\�[�X)���쐬
	_dev->CreateCommittedResource(
		&_uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&sceneResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_sceneConstBuff));

	//�萔�o�b�t�@�[�ɑ΂��������݉\�ȃf�[�^���Z�b�g
	_mappedScene = nullptr;
	auto result = _sceneConstBuff->Map(0,nullptr,(void**)&_mappedScene);

	//���L�̍��W����������
	_mappedScene->view = XMMatrixLookAtLH(XMLoadFloat3(&_eye),XMLoadFloat3(&_target),XMLoadFloat3(&_up));
	_mappedScene->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		static_cast<float>(_winSize.cx)/ static_cast<float>(_winSize.cy),
		0.1f,
		1000.0f
	);
	_mappedScene->eye = _eye;

	//�萔�o�b�t�@�[�p�f�B�X�N���v�^�q�[�v�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC sceneDescHeapDesc = {};
	sceneDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	sceneDescHeapDesc.NodeMask = 0;
	sceneDescHeapDesc.NumDescriptors = 1;
	sceneDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//�f�B�X�N���v�^�q�[�v�쐬
	result = _dev->CreateDescriptorHeap(&sceneDescHeapDesc,IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));

	//�萔�o�b�t�@�[�r���[�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_sceneConstBuff->GetDesc().Width;
	
	//�萔�o�b�t�@�[�r���[�̍쐬
	_dev->CreateConstantBufferView(
		&cbvDesc,
		_sceneDescHeap->GetCPUDescriptorHandleForHeapStart());

	return result;	//�֐������������̂�Ԃ�
}

/// <summary>
/// �[�x�X�e���V���r���[���쐬���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Dx12Wrapper::CreateDepthStencilView()
{
	//�[�x�X�e���V���o�b�t�@�[�p�q�[�v�v���p�e�B���쐬�E�ݒ�
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//�o�b�t�@�[�p���\�[�X�f�B�X�N���쐬�E�ݒ�
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = _winSize.cx;
	depthResDesc.Height = _winSize.cy;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//�N���A�o�����[�H�̍쐬�E�ݒ�
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//�o�b�t�@�[�̍쐬
	auto result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));

	//�o�b�t�@�[�p�f�B�X�N���v�^�q�[�v�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	//�f�B�X�N���v�^�q�[�v�̍쐬
	result = _dev->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&_dsvHeap));

	//�o�b�t�@�[�r���[�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	//�r���[�̍쐬
	_dev->CreateDepthStencilView(_depthBuffer.Get(),&dsvDesc,_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return result;	//�֐������������̂�Ԃ�
}



/// <summary>
/// �y���|���S���ɂ�����G�t�F�N�g�p�̃o�b�t�@�[�E�r���[���쐬����֐�
/// </summary>
/// <returns>�����������������ǂ���</returns>
HRESULT
Dx12Wrapper::CreateEffectBufferAndView()
{
	//�e�N�X�`���ǂݍ��ݗp�f�[�^
	TexMetadata meta = {};
	ScratchImage scratch = {};

	wstring filePath = L"5_image/normalmap.png";	//�G�t�F�N�g�̃p�X

	auto ext = FileExtension(filePath);	//�g���q���擾

	//�G�t�F�N�g�f�[�^��ǂݍ���
	auto result = _loadLambdaTable[ToString(ext)](filePath, &meta, scratch);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	auto img = scratch.GetImage(0, 0, 0);

	DXGI_FORMAT format = meta.format;							//�e�N�X�`���̃t�H�[�}�b�g
	size_t width = meta.width;									//��
	size_t height = meta.height;								//����
	UINT16 arraySize = static_cast<UINT16>(meta.arraySize);		//�e�N�X�`���T�C�Y
	UINT16 mipLevels = static_cast<UINT16>(meta.mipLevels);
	void* pixels = img->pixels;
	UINT rowPitch = static_cast<UINT>(img->rowPitch);
	UINT slicePitch = static_cast<UINT>(img->slicePitch);

	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		(UINT)width,
		(UINT)height,
		arraySize,
		(UINT)mipLevels);

	//�V�F�[�_�[���\�[�X�̍쐬
	result = _dev->CreateCommittedResource
	(
		&_writeHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(_effectTexBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�e�N�X�`���̏�������
	result = _effectTexBuffer->WriteToSubresource(0,
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

	//�e�N�X�`���p�f�B�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NodeMask = 1;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.NumDescriptors = 1;
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	_dev->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(_effectSRVHeap.ReleaseAndGetAddressOf()));

	auto desc = _effectTexBuffer->GetDesc();
	//�e�N�X�`���o�b�t�@�[�r���[�p�\���̂̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_dev->CreateShaderResourceView(_effectTexBuffer.Get(), &srvDesc, _effectSRVHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// �G�t�F�N�g�K�p�����߂�f�[�^�p�̃q�[�v�E�o�b�t�@�[���쐬����֐�
/// </summary>
/// <returns>�֐��������������ǂ���</returns>
HRESULT
Dx12Wrapper::CreateFactorBufferAndView()
{
	//�o�b�t�@�[�쐬
	auto factorResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(Factor) + _constSize) & ~_constSize);

	auto result = _dev->CreateCommittedResource(
		&_writeHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&factorResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_factorConstBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�o�b�t�@�[�ւ̏�������
	result = _factorConstBuff->Map(0, nullptr, (void**)&_mappedFactor);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}
	UpdateFactor();

	//�萔�o�b�t�@�[�p�f�B�X�N���v�^�q�[�v�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC factorHeapDesc = {};
	factorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	factorHeapDesc.NodeMask = 0;
	factorHeapDesc.NumDescriptors = 1;
	factorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//�f�B�X�N���v�^�q�[�v�쐬
	result = _dev->CreateDescriptorHeap(&factorHeapDesc, IID_PPV_ARGS(_factorCBVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//�萔�o�b�t�@�[�r���[�ݒ�p�\���̂̍쐬�E�ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _factorConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_factorConstBuff->GetDesc().Width;

	//�萔�o�b�t�@�[�r���[�̍쐬
	_dev->CreateConstantBufferView(
		&cbvDesc,
		_factorCBVHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// �e�N�X�`�����[�h�p�e�[�u�����쐬����֐�
/// </summary>
void
Dx12Wrapper::CreateTextureLoaderTable()
{
	//�t�@�C�����Ɋ܂܂�Ă���g���q�ɉ����Ď��s����֐���ς���
	//sph,spa,png,jpg�g���q��key�ɁALoadFromWICFile�֐���value�ɂ���
	_loadLambdaTable["sph"]
		= _loadLambdaTable["spa"]
		= _loadLambdaTable["bmp"]
		= _loadLambdaTable["png"]
		= _loadLambdaTable["jpg"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& img)
		->HRESULT
	{
		return LoadFromWICFile(path.c_str(), 0, meta, img);
	};

	//tga�g���q��key�ɁALoadFromTGAFile�֐���value�ɂ���
	_loadLambdaTable["tga"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& img)
		->HRESULT
	{
		return LoadFromTGAFile(path.c_str(), meta, img);
	};

	//dds�g���q��key�ɁALoadFromDDSFile�֐���value�ɂ���
	_loadLambdaTable["dds"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& img)
		->HRESULT
	{
		return LoadFromDDSFile(path.c_str(), 0, meta, img);
	};
}

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="hwnd">�E�B���h�E�n���h��</param>
/// <param name="deltaTime">�t���[���̐؂�ւ��Ԋu</param>
Dx12Wrapper::Dx12Wrapper(HWND hwnd, float deltaTime) :
	_initialPos(0, 100, 180), _target(0, 100, 0), _up(0, 1, 0), _deltaTime(deltaTime), _dist(0.0f),_fade(1.0f)
{
#ifdef _DEBUG
	EnableDebugLayer();		//�f�o�b�O�p���C���[���N��
#endif 
	auto& app = Application::Instance();	//Application�C���X�^���X���擾
	_winSize = app.GetWindowSize();			//�E�B���h�E�̃T�C�Y���擾

	InitializeDXGIDevice();					//�f�o�C�X�֘A��������

	InitializeCommand();					//�R�}���h�֘A��������

	CreateSwapChain(hwnd);					//�X���b�v�`�F�[�����쐬

	CreateRenderTargetsView();				//�����_�[�^�[�Q�b�g���쐬

	CreateSceneView();						//�r���[�v���W�F�N�V�����p�r���[���쐬

	CreateTextureLoaderTable();				//�e�N�X�`�����[�h�p�e�[�u�����쐬

	CreateDepthStencilView();				//�[�x�X�e���V���r���[���쐬

	CreateEffectBufferAndView();			//�c�݉摜�p�o�b�t�@�[�E�r���[���쐬

	CreateFactorBufferAndView();			//�c�݁E�t�F�[�h�C���^�A�E�g�p�o�b�t�@�[�E�r���[���쐬

	_dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));		//�t�F���X���쐬
}


/// <summary>
/// ���\�[�X��J�ڂ�����֐�
/// </summary>
/// <param name="resource">�J�ڂ����������\�[�X</param>
/// <param name="before">�J�ڑO�̃X�e�[�g</param>
/// <param name="after">�J�ڌ�̃X�e�[�g</param>
void
Dx12Wrapper::BarrierTransition(
	ID3D12Resource* resource,
	D3D12_RESOURCE_STATES before,
	D3D12_RESOURCE_STATES after)
{
	_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);	//�ϐ���ݒ�

	//���\�[�X��J�ڂ�����
	_cmdList->ResourceBarrier(1, &_barrier);
}

/// <summary>
/// �J�������ߕt����E��������֐�
/// </summary>
/// <param name="x">�ړ�����</param>
void
Dx12Wrapper::TranslateSphericalCoordinates(int x)
{
	_eye = XMFLOAT3
	(
		_coordinates->TranslateRadius(x * _deltaTime).ToCartesian().x + _target.x,
		_coordinates->TranslateRadius(x * _deltaTime).ToCartesian().y + _target.y,
		_coordinates->TranslateRadius(x * _deltaTime).ToCartesian().z + _target.z
	);
}

/// <summary>
/// SphericalCoordinates��̃J�������W����]������֐�
/// </summary>
/// <param name="rotation">���ʊp���p�����߂�</param>
/// <param name="direction">�ォ�����A�E������</param>
void
Dx12Wrapper::RotateSphericalCoordinates(Degree rotation,int direction)
{
	switch (rotation)
	{
	case Degree::Azimth:	//���ʊp�̕����ɉ�]������
		_eye = XMFLOAT3
		(
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().x + _target.x,
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().y + _target.y,
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().z + _target.z
		);
		break;
	case Degree::Elevation:	//�p�̕����ɉ�]������
		_eye = XMFLOAT3
		(
			_coordinates->Rotate(0.0f, direction * 45.0f * _deltaTime).ToCartesian().x + _target.x,
			_coordinates->Rotate(0.0f, direction * 45.0f * _deltaTime).ToCartesian().y + _target.y,
			_coordinates->Rotate(0.0f, direction * 45.0f * _deltaTime).ToCartesian().z + _target.z
		);
		break;
	default:
		break;
	}
}

/// <summary>
/// �J�����̈ʒu������������֐�
/// </summary>
void
Dx12Wrapper::ResetSphericalCoordinates()
{
	_eye = _initialPos;

	_coordinates.reset(new SphericalCoordinates());	//���ʍ��W������SphericalCoordinates�C���X�^���X���쐬
	_coordinates->SetRadius(180.0f);					//���a��180�ŌŒ�
	_coordinates->SetAzimth(0.0f);					//���ʊp�A�p��0�x�ŏ�����
	_coordinates->SetElevation(0.0f);
}

/// <summary>
/// �r���[�v���W�F�N�V�����p�r���[���Z�b�g����֐�
/// </summary>
void
Dx12Wrapper::SetScene()
{
	//�������݉\�ȃf�[�^�ɏ�L�̍��W����������
	_mappedScene->view = XMMatrixLookAtLH(XMLoadFloat3(&_eye), XMLoadFloat3(&_target), XMLoadFloat3(&_up));
	_mappedScene->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		static_cast<float>(_winSize.cx) / static_cast<float>(_winSize.cy),
		0.1f,
		1000.0f
	);
	_mappedScene->eye = _eye;

	//�f�B�X�N���v�^�q�[�v���R�}���h���X�g�ɃZ�b�g
	ID3D12DescriptorHeap* sceneHeaps[] = { _sceneDescHeap.Get() };
	_cmdList->SetDescriptorHeaps(1, sceneHeaps);

	//�f�B�X�N���v�^�q�[�v�̃n���h�������[�g�p�����[�^�Ɗ֘A�t��
	_cmdList->SetGraphicsRootDescriptorTable(0, _sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// �Q�[����ʗp���\�[�X�̑J��(PRESENT��RENDER_TARGET)�ERTV�̃Z�b�g�����s����֐�
/// </summary>
void
Dx12Wrapper::BeginGameDraw()
{
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();		//���݂̃o�b�N�o�b�t�@�[�̃C���f�b�N�X���擾

	//�Q�[����ʗp���\�[�X��RENDER_TARGET�ɑJ��
	BarrierTransition(_backBuffers[bbIdx], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtvH = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);

	float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };	//�w�i�F��ݒ�

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

/// <summary>
/// �Q�[���p���\�[�X�̑J��(RENDER_TARGET��STATE_PRESENT)�����s����֐�
/// </summary>
void
Dx12Wrapper::EndGameDraw()
{
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();		//���݂̃o�b�N�o�b�t�@�[�̃C���f�b�N�X���擾

	//�Q�[����ʗp���\�[�X��PRESENT�ɑJ��
	BarrierTransition(_backBuffers[bbIdx], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	_cmdList->Close();	//�R�}���h���X�g�̃N���[�Y

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	WaitForCommandQueue();

	_cmdAllocator->Reset();						//�L���[�̃N���A
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);	//�ēx�R�}���h���X�g�𒙂߂鏀��
}
/// <summary>
/// �����̓����҂����s���֐�
/// </summary>
void
Dx12Wrapper::WaitForCommandQueue()
{
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);	//�t�F���X�l���X�V

	//CPU��GPU�̃t�F���X�l���r���A��v����܂ŏ�����҂����킹��
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);	//��̃C�x���g���擾�H

		_fence->SetEventOnCompletion(_fenceVal, event);				//�t�F���X�l��_fenceVal�ɂȂ������C�x���g��ʒm

		WaitForSingleObject(event, INFINITE);						//�C�x���g����������܂ő҂�������

		CloseHandle(event);											//�C�x���g�����
	}
}

/// <summary>
/// �e�N�X�`����ǂݍ��ފ֐�
/// </summary>
/// <param name="texPath">�e�N�X�`���t�@�C����</param>
/// <returns>�e�N�X�`��(�V�F�[�_�[���\�[�X)�o�b�t�@�[</returns>
ID3D12Resource*
Dx12Wrapper::LoadTextureFromFile(const char* texPath)
{
	//���\�[�X�p�A�z�z��̕�����Ƀt�@�C���������݂��Ȃ����m�F���A����ΑΉ����郊�\�[�X��Ԃ�
	//���Ƀt�@�C���������݂���(=���̃e�N�X�`����ǂݍ���)�Ȃ�Ή����郊�\�[�X��Ԃ��Ώ����̊ȗ����Ɍq����
	auto it = _resourceTable.find(texPath);
	if (it != _resourceTable.end())
	{
		return _resourceTable[texPath];
	}

	string strTexPath = texPath;	//�t�@�C�������R�s�[

	TexMetadata metaData = {};		//���^�f�[�^(�摜�t�@�C���Ɋ֐����)
	ScratchImage scratchImg = {};	//���ۂ̃e�N�X�`���f�[�^������I�u�W�F�N�g

	auto wtexPath = ToWideString(strTexPath);	//�t�@�C������wstring�^�ɕϊ�
	auto ext = FileExtension(strTexPath);					//�t�@�C�����̊g���q���擾

	//�֐��p�A�z�z��̕�����̊g���q���m�F���A����ΑΉ�����֐���p���ēǂݍ���
	auto result = _loadLambdaTable[ext](wtexPath,
		&metaData,
		scratchImg);
	if (FAILED(result))
	{
		assert(0);
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);	//�e�N�X�`���̐��f�[�^�擾

	//�q�[�v�v���p�e�B�쐬�E�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	//�o�b�t�@�[�p���\�[�X�f�B�X�N�\���̍쐬�E�ݒ�
	D3D12_RESOURCE_DESC texResDesc = {};
	texResDesc.Format = metaData.format;
	texResDesc.Width = metaData.width;
	texResDesc.Height = (UINT)metaData.height;
	texResDesc.DepthOrArraySize = (UINT16)metaData.arraySize;
	texResDesc.SampleDesc.Count = 1;
	texResDesc.SampleDesc.Quality = 0;
	texResDesc.MipLevels = (UINT16)metaData.mipLevels;
	texResDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);
	texResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//���\�[�X�쐬
	ID3D12Resource* texBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuff));
	if (FAILED(result))
	{
		return nullptr;
	}

	//�쐬�������\�[�X�Ƀe�N�X�`������������
	result = texBuff->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		(UINT)img->rowPitch,
		(UINT)img->slicePitch
	);
	if (FAILED(result))
	{
		return nullptr;
	}

	_resourceTable[texPath] = texBuff;	//�e�N�X�`���p�A�z�z��Ƀt�@�C�����Ƃ���Ɋ֘A�����e�N�X�`����o�^

	return texBuff;
}

/// <summary>
/// //DXTK12�p�̃q�[�v���쐬����֐�
/// </summary>
/// <returns>�q�[�v</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::CreateDescriptorHeapForSpriteFont()
{
	ID3D12DescriptorHeap* ret = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 256;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	_dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ret));

	return ret;
}

/// <summary>
/// ���[�g�V�O�l�`���E�p�C�v���C���E�`����@���Z�b�g����֐�
/// </summary>
/// <param name="rootSignature">���[�g�V�O�l�`��</param>
/// <param name="pipeline">�p�C�v���C��</param>
/// <param name="topology">�`����@</param>
void
Dx12Wrapper::SetPipelines(ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipeline, D3D12_PRIMITIVE_TOPOLOGY topology)
{
	_cmdList->SetGraphicsRootSignature(rootSignature);
	_cmdList->SetPipelineState(pipeline);
	_cmdList->IASetPrimitiveTopology(topology);
}

/// <summary>
/// //��ʘc�݁E�t�F�[�h�C���^�A�E�g�f�[�^���V�F�[�_�[�ɔ��f����֐�
/// </summary>
void
Dx12Wrapper::UpdateFactor()
{
	//�l�͈̔͂𐧌�
	_dist = clamp(_dist, 0.0f, 1.0f);
	_fade = clamp(_fade, 0.0f, 1.0f);

	//�V�F�[�_�[�֔��f������
	_mappedFactor->dist = _dist;
	_mappedFactor->fade = _fade;
}

/// <summary>
/// ��ʂ�c�܂���f�[�^��ݒ肷��֐�
/// </summary>
/// <param name="factor">�f�[�^�̒l</param>
void
Dx12Wrapper::SetDist(float dist)
{
	_dist = dist;
}

/// <summary>
/// �t�F�[�h�C���^�A�E�g�����s����֐�
/// </summary>
/// <param name="func">�e�V�[���̕`��֐�</param>
/// <param name="start">�t�F�[�h�l�̏����l</param>
/// <param name="end">�t�F�[�h�l�̍ŏI�l</param>
void
Dx12Wrapper::Fade(function<void()> func, float start, float end)
{
	MSG msg = {};							//���b�Z�[�W���󂯎��\����

	float t = 0.0f;							//����
	_start = start;							//�t�F�[�h�l�̏����l
	_end = end;								//�t�F�[�h�l�̍ŏI�l
	for (int i = 0; i < FADE_TIME; i++)
	{
		_fade = lerp(_start, _end, t);		//�t�F�[�h�l����`���

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	//���b�Z�[�W�𒲂ׂ�
		{
			TranslateMessage(&msg);								//���b�Z�[�W��|��
			DispatchMessage(&msg);								//�E�B���h�E�v���V�[�W���փ��b�Z�[�W�𑗂�

			if (msg.message == WM_QUIT)						//�A�v�����I�������烋�[�v���I��������
			{
				break;
			}
		}
		else
		{
			func();								//�e�V�[���̕`�揈�������s
		}

		t += FADE_DIFF;						//���Z
	}
}

/// <summary>
/// �f�o�C�X��Ԃ��֐�
/// </summary>
/// <returns>�f�o�C�X�̃|�C���^</returns>
ID3D12Device*
Dx12Wrapper::Device()
{
	return _dev.Get();
}

/// <summary>
/// �X���b�v�`�F�[����Ԃ��֐�
/// </summary>
/// <returns>�X���b�v�`�F�[���̃|�C���^</returns>
IDXGISwapChain4*
Dx12Wrapper::Swapchain()
{
	return _swapchain.Get();
}

/// <summary>
/// �R�}���h���X�g��Ԃ��֐�
/// </summary>
/// <returns>�R�}���h���X�g�̃|�C���^</returns>
ID3D12GraphicsCommandList*
Dx12Wrapper::CommandList()
{
	return _cmdList.Get();
}

/// <summary>
/// �R�}���h�L���[��Ԃ��֐�
/// </summary>
/// <returns>�R�}���h�L���[�̃|�C���^</returns>
ID3D12CommandQueue*
Dx12Wrapper::CommandQueue()
{
	return _cmdQueue.Get();
}

/// <summary>
/// �o�b�N�o�b�t�@�[�i1���ځj��Ԃ��֐�
/// </summary>
/// <returns></returns>
ID3D12Resource*
Dx12Wrapper::BackBuffer() const
{
	return _backBuffers[0];
}

/// <summary>
/// RTV�q�[�v��Ԃ��֐�
/// </summary>
/// <returns>RTV�q�[�v</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::RTVHeap() const
{
	return _rtvHeap.Get();
}

/// <summary>
/// �[�x�X�e���V���q�[�v��Ԃ��֐�
/// </summary>
/// <returns>�[�x�X�e���V���q�[�v</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::DSVHeap() const
{
	return _dsvHeap.Get();
}

/// <summary>
/// �G�t�F�N�g�p�q�[�v��Ԃ��֐�
/// </summary>
/// <returns>�G�t�F�N�g�p�q�[�v</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::EffectSRVHeap() const
{
	return _effectSRVHeap.Get();
}

/// <summary>
/// �����p�q�[�v��Ԃ��֐�
/// </summary>
/// <returns>�����p�q�[�v</returns>
ID3D12DescriptorHeap* 
Dx12Wrapper::FactorCBVHeap() const
{
	return _factorCBVHeap.Get();
}

/// <summary>
/// �r���[�|�[�g��Ԃ��֐�
/// </summary>
/// <returns>�r���[�|�[�g</returns>
D3D12_VIEWPORT*
Dx12Wrapper::ViewPort() const
{
	return _viewPort.get();
}

/// <summary>
/// �V�U�[��`��Ԃ��֐�
/// </summary>
/// <returns>�V�U�[��`</returns>
D3D12_RECT*
Dx12Wrapper::Rect() const
{
	return _rect.get();
}