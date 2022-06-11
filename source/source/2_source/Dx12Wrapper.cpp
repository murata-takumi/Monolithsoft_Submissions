#include "Dx12Wrapper.h"
#include "Functions.h"
#include "SphericalCoordinates.h"

const int FADE_TIME = 80;								//フェードイン／アウトにかかる時間
const float FADE_DIFF = (float)1 / (float)FADE_TIME;	//フェード時の切り替え間隔

/// <summary>
/// デバッグ用レイヤーを初期化する関数
/// </summary>
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));
		
	debugLayer->EnableDebugLayer();	//デバッグレイヤーを有効化する
	debugLayer->Release();			//有効化したらインターフェイスを開放する
}

/// <summary>
/// デコンストラクタ
/// 特に処理は行わない
/// </summary>
Dx12Wrapper::~Dx12Wrapper()
{

}

/// <summary>
/// デバイス関連を初期化する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Dx12Wrapper::InitializeDXGIDevice()
{
	//Debugかそうでないかでファクトリーの作成関数を変える
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG
	
	//フィーチャーレベルの配列を初期化
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	vector<IDXGIAdapter*> adapters;			//列挙したアダプターを格納
	IDXGIAdapter* tmpAdapter = nullptr;		//アダプター

	//ファクトリー内の全アダプターに対しループ
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);	//アダプターをベクトルに格納
	}

	//格納した全アダプターに対しループ
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);					//アダプターの説明を記述する構造体を取得

		wstring strDesc = adesc.Description;	//アダプターの名前を取得

		//アダプターの名前が特定の名前と一致したらループ終了
		if (strDesc.find(L"NVIDIA") != string::npos)
		{
			tmpAdapter = adpt;	//デバイス作成に使用するアダプターを決定
			break;				//ループ終了
		}
	}

	//初期化したフィーチャーレベルに対しループ
	for (auto lv : levels)
	{
		//デバイスを作成できるフィーチャーレベルが見つかったらそのまま作成してループ終了
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			result = S_OK;		//作成が成功したことを返す
			break;				//生成可能なバージョンが見つかったらループ終了
		}
	}
	return result;
}

/// <summary>
/// コマンド関連を初期化する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Dx12Wrapper::InitializeCommand()
{
	//コマンドアロケータを作成
	auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));

	//コマンドリストを作成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(), nullptr,
		IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));

	//コマンドキュー設定構造体を作成・設定
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	//コマンドキューを作成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));

	return result;	//処理の成功を返す
}

/// <summary>
/// スワップチェーンを作成する関数
/// </summary>
/// <param name="hwnd">ウィンドウ識別用データ</param>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Dx12Wrapper::CreateSwapChain(const HWND& hwnd)
{
	//スワップチェーン設定用構造体・設定
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

	//スワップチェーン作成
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
/// レンダーターゲットを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT 
Dx12Wrapper::CreateRenderTargetsView()
{
	//レンダーターゲット用ディスクリプタヒープ設定構造体の作成・設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//レンダーターゲット用ディスクリプタヒープの作成
	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_rtvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//スワップチェーンの情報取得
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//バックバッファーのサイズをスワップチェーンのバッファー数に合わせる
	_backBuffers.resize(swcDesc.BufferCount);

	//ディスクリプタヒープの先頭アドレス(ハンドル)
	D3D12_CPU_DESCRIPTOR_HANDLE handle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	//レンダーターゲットビュー設定用構造体の作成・設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//各バックバッファーに対しループを回す
	for (int idx = 0; idx < (int)swcDesc.BufferCount; ++idx)
	{
		//スワップチェーンからデータを取得、バックバッファーに割り当てる
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));	
		if (FAILED(result))
		{
			assert(0);
			return result;
		}

		//バックバッファーを対象にレンダーターゲットビューを作成
		_dev->CreateRenderTargetView(
			_backBuffers[idx],
			&rtvDesc,
			handle);

		//ハンドルの位置をずらす
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//スワップチェーンの情報を取得(上記のswcDescとは別物)
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	result = _swapchain->GetDesc1(&desc);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//スワップチェーンの情報を元にビューポート、シザー矩形を初期化
	_viewPort.reset(new CD3DX12_VIEWPORT(_backBuffers[0]));
	_rect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));

	return result;	//関数が成功したのを返す
}

/// <summary>
/// ビュープロジェクション用ビューを作成
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT 
Dx12Wrapper::CreateSceneView()
{
	auto sceneResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + _constSize) & ~_constSize);

	//定数バッファー(リソース)を作成
	_dev->CreateCommittedResource(
		&_uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&sceneResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_sceneConstBuff));

	//定数バッファーに対し書き込み可能なデータをセット
	_mappedScene = nullptr;
	auto result = _sceneConstBuff->Map(0,nullptr,(void**)&_mappedScene);

	//下記の座標を書き込む
	_mappedScene->view = XMMatrixLookAtLH(XMLoadFloat3(&_eye),XMLoadFloat3(&_target),XMLoadFloat3(&_up));
	_mappedScene->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		static_cast<float>(_winSize.cx)/ static_cast<float>(_winSize.cy),
		0.1f,
		1000.0f
	);
	_mappedScene->eye = _eye;

	//定数バッファー用ディスクリプタヒープ設定用構造体の作成・設定
	D3D12_DESCRIPTOR_HEAP_DESC sceneDescHeapDesc = {};
	sceneDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	sceneDescHeapDesc.NodeMask = 0;
	sceneDescHeapDesc.NumDescriptors = 1;
	sceneDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//ディスクリプタヒープ作成
	result = _dev->CreateDescriptorHeap(&sceneDescHeapDesc,IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));

	//定数バッファービュー設定用構造体の作成・設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_sceneConstBuff->GetDesc().Width;
	
	//定数バッファービューの作成
	_dev->CreateConstantBufferView(
		&cbvDesc,
		_sceneDescHeap->GetCPUDescriptorHandleForHeapStart());

	return result;	//関数が成功したのを返す
}

/// <summary>
/// 深度ステンシルビューを作成を作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Dx12Wrapper::CreateDepthStencilView()
{
	//深度ステンシルバッファー用ヒーププロパティを作成・設定
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//バッファー用リソースディスクを作成・設定
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = _winSize.cx;
	depthResDesc.Height = _winSize.cy;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//クリアバリュー？の作成・設定
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//バッファーの作成
	auto result = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));

	//バッファー用ディスクリプタヒープ設定用構造体の作成・設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	//ディスクリプタヒープの作成
	result = _dev->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&_dsvHeap));

	//バッファービュー設定用構造体の作成・設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	//ビューの作成
	_dev->CreateDepthStencilView(_depthBuffer.Get(),&dsvDesc,_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return result;	//関数が成功したのを返す
}



/// <summary>
/// ペラポリゴンにかけるエフェクト用のバッファー・ビューを作成する関数
/// </summary>
/// <returns>処理が成功したかどうか</returns>
HRESULT
Dx12Wrapper::CreateEffectBufferAndView()
{
	//テクスチャ読み込み用データ
	TexMetadata meta = {};
	ScratchImage scratch = {};

	wstring filePath = L"5_image/normalmap.png";	//エフェクトのパス

	auto ext = FileExtension(filePath);	//拡張子を取得

	//エフェクトデータを読み込み
	auto result = _loadLambdaTable[ToString(ext)](filePath, &meta, scratch);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	auto img = scratch.GetImage(0, 0, 0);

	DXGI_FORMAT format = meta.format;							//テクスチャのフォーマット
	size_t width = meta.width;									//幅
	size_t height = meta.height;								//高さ
	UINT16 arraySize = static_cast<UINT16>(meta.arraySize);		//テクスチャサイズ
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

	//シェーダーリソースの作成
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

	//テクスチャの書き込み
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

	//テクスチャ用ディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NodeMask = 1;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.NumDescriptors = 1;
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	_dev->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(_effectSRVHeap.ReleaseAndGetAddressOf()));

	auto desc = _effectTexBuffer->GetDesc();
	//テクスチャバッファービュー用構造体の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_dev->CreateShaderResourceView(_effectTexBuffer.Get(), &srvDesc, _effectSRVHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// エフェクト適用を決めるデータ用のヒープ・バッファーを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Dx12Wrapper::CreateFactorBufferAndView()
{
	//バッファー作成
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

	//バッファーへの書き込み
	result = _factorConstBuff->Map(0, nullptr, (void**)&_mappedFactor);
	if (FAILED(result))
	{
		assert(0);
		return result;
	}
	UpdateFactor();

	//定数バッファー用ディスクリプタヒープ設定用構造体の作成・設定
	D3D12_DESCRIPTOR_HEAP_DESC factorHeapDesc = {};
	factorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	factorHeapDesc.NodeMask = 0;
	factorHeapDesc.NumDescriptors = 1;
	factorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//ディスクリプタヒープ作成
	result = _dev->CreateDescriptorHeap(&factorHeapDesc, IID_PPV_ARGS(_factorCBVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//定数バッファービュー設定用構造体の作成・設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _factorConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_factorConstBuff->GetDesc().Width;

	//定数バッファービューの作成
	_dev->CreateConstantBufferView(
		&cbvDesc,
		_factorCBVHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// テクスチャロード用テーブルを作成する関数
/// </summary>
void
Dx12Wrapper::CreateTextureLoaderTable()
{
	//ファイル名に含まれている拡張子に応じて実行する関数を変える
	//sph,spa,png,jpg拡張子をkeyに、LoadFromWICFile関数をvalueにする
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

	//tga拡張子をkeyに、LoadFromTGAFile関数をvalueにする
	_loadLambdaTable["tga"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& img)
		->HRESULT
	{
		return LoadFromTGAFile(path.c_str(), meta, img);
	};

	//dds拡張子をkeyに、LoadFromDDSFile関数をvalueにする
	_loadLambdaTable["dds"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& img)
		->HRESULT
	{
		return LoadFromDDSFile(path.c_str(), 0, meta, img);
	};
}

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="hwnd">ウィンドウハンドル</param>
/// <param name="deltaTime">フレームの切り替え間隔</param>
Dx12Wrapper::Dx12Wrapper(HWND hwnd, float deltaTime) :
	_initialPos(0, 100, 180), _target(0, 100, 0), _up(0, 1, 0), _deltaTime(deltaTime), _dist(0.0f),_fade(1.0f)
{
#ifdef _DEBUG
	EnableDebugLayer();		//デバッグ用レイヤーを起動
#endif 
	auto& app = Application::Instance();	//Applicationインスタンスを取得
	_winSize = app.GetWindowSize();			//ウィンドウのサイズを取得

	InitializeDXGIDevice();					//デバイス関連を初期化

	InitializeCommand();					//コマンド関連を初期化

	CreateSwapChain(hwnd);					//スワップチェーンを作成

	CreateRenderTargetsView();				//レンダーターゲットを作成

	CreateSceneView();						//ビュープロジェクション用ビューを作成

	CreateTextureLoaderTable();				//テクスチャロード用テーブルを作成

	CreateDepthStencilView();				//深度ステンシルビューを作成

	CreateEffectBufferAndView();			//歪み画像用バッファー・ビューを作成

	CreateFactorBufferAndView();			//歪み・フェードイン／アウト用バッファー・ビューを作成

	_dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));		//フェンスを作成
}


/// <summary>
/// リソースを遷移させる関数
/// </summary>
/// <param name="resource">遷移させたいリソース</param>
/// <param name="before">遷移前のステート</param>
/// <param name="after">遷移後のステート</param>
void
Dx12Wrapper::BarrierTransition(
	ID3D12Resource* resource,
	D3D12_RESOURCE_STATES before,
	D3D12_RESOURCE_STATES after)
{
	_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);	//変数を設定

	//リソースを遷移させる
	_cmdList->ResourceBarrier(1, &_barrier);
}

/// <summary>
/// カメラを近付ける・遠ざける関数
/// </summary>
/// <param name="x">移動距離</param>
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
/// SphericalCoordinates上のカメラ座標を回転させる関数
/// </summary>
/// <param name="rotation">方位角か仰角か決める</param>
/// <param name="direction">上か下か、右か左か</param>
void
Dx12Wrapper::RotateSphericalCoordinates(Degree rotation,int direction)
{
	switch (rotation)
	{
	case Degree::Azimth:	//方位角の方向に回転させる
		_eye = XMFLOAT3
		(
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().x + _target.x,
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().y + _target.y,
			_coordinates->Rotate(direction * 90.0f * _deltaTime, 0.0f).ToCartesian().z + _target.z
		);
		break;
	case Degree::Elevation:	//仰角の方向に回転させる
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
/// カメラの位置を初期化する関数
/// </summary>
void
Dx12Wrapper::ResetSphericalCoordinates()
{
	_eye = _initialPos;

	_coordinates.reset(new SphericalCoordinates());	//球面座標を扱うSphericalCoordinatesインスタンスを作成
	_coordinates->SetRadius(180.0f);					//半径は180で固定
	_coordinates->SetAzimth(0.0f);					//方位角、仰角は0度で初期化
	_coordinates->SetElevation(0.0f);
}

/// <summary>
/// ビュープロジェクション用ビューをセットする関数
/// </summary>
void
Dx12Wrapper::SetScene()
{
	//書き込み可能なデータに上記の座標を書き込む
	_mappedScene->view = XMMatrixLookAtLH(XMLoadFloat3(&_eye), XMLoadFloat3(&_target), XMLoadFloat3(&_up));
	_mappedScene->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		static_cast<float>(_winSize.cx) / static_cast<float>(_winSize.cy),
		0.1f,
		1000.0f
	);
	_mappedScene->eye = _eye;

	//ディスクリプタヒープをコマンドリストにセット
	ID3D12DescriptorHeap* sceneHeaps[] = { _sceneDescHeap.Get() };
	_cmdList->SetDescriptorHeaps(1, sceneHeaps);

	//ディスクリプタヒープのハンドルをルートパラメータと関連付け
	_cmdList->SetGraphicsRootDescriptorTable(0, _sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// ゲーム画面用リソースの遷移(PRESENT→RENDER_TARGET)・RTVのセットを実行する関数
/// </summary>
void
Dx12Wrapper::BeginGameDraw()
{
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();		//現在のバックバッファーのインデックスを取得

	//ゲーム画面用リソースをRENDER_TARGETに遷移
	BarrierTransition(_backBuffers[bbIdx], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtvH = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);

	float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };	//背景色を設定

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

/// <summary>
/// ゲーム用リソースの遷移(RENDER_TARGET→STATE_PRESENT)を実行する関数
/// </summary>
void
Dx12Wrapper::EndGameDraw()
{
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();		//現在のバックバッファーのインデックスを取得

	//ゲーム画面用リソースをPRESENTに遷移
	BarrierTransition(_backBuffers[bbIdx], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	_cmdList->Close();	//コマンドリストのクローズ

	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	WaitForCommandQueue();

	_cmdAllocator->Reset();						//キューのクリア
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);	//再度コマンドリストを貯める準備
}
/// <summary>
/// 処理の同期待ちを行う関数
/// </summary>
void
Dx12Wrapper::WaitForCommandQueue()
{
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);	//フェンス値を更新

	//CPUとGPUのフェンス値を比較し、一致するまで処理を待ち合わせる
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);	//空のイベントを取得？

		_fence->SetEventOnCompletion(_fenceVal, event);				//フェンス値が_fenceValになった時イベントを通知

		WaitForSingleObject(event, INFINITE);						//イベントが発生するまで待ち続ける

		CloseHandle(event);											//イベントを閉じる
	}
}

/// <summary>
/// テクスチャを読み込む関数
/// </summary>
/// <param name="texPath">テクスチャファイル名</param>
/// <returns>テクスチャ(シェーダーリソース)バッファー</returns>
ID3D12Resource*
Dx12Wrapper::LoadTextureFromFile(const char* texPath)
{
	//リソース用連想配列の文字列にファイル名が存在しないか確認し、あれば対応するリソースを返す
	//既にファイル名が存在する(=そのテクスチャを読み込んだ)なら対応するリソースを返せば処理の簡略化に繋がる
	auto it = _resourceTable.find(texPath);
	if (it != _resourceTable.end())
	{
		return _resourceTable[texPath];
	}

	string strTexPath = texPath;	//ファイル名をコピー

	TexMetadata metaData = {};		//メタデータ(画像ファイルに関数情報)
	ScratchImage scratchImg = {};	//実際のテクスチャデータを入れるオブジェクト

	auto wtexPath = ToWideString(strTexPath);	//ファイル名をwstring型に変換
	auto ext = FileExtension(strTexPath);					//ファイル名の拡張子を取得

	//関数用連想配列の文字列の拡張子を確認し、あれば対応する関数を用いて読み込み
	auto result = _loadLambdaTable[ext](wtexPath,
		&metaData,
		scratchImg);
	if (FAILED(result))
	{
		assert(0);
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);	//テクスチャの生データ取得

	//ヒーププロパティ作成・設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	//バッファー用リソースディスク構造体作成・設定
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

	//リソース作成
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

	//作成したリソースにテクスチャを書き込む
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

	_resourceTable[texPath] = texBuff;	//テクスチャ用連想配列にファイル名とそれに関連したテクスチャを登録

	return texBuff;
}

/// <summary>
/// //DXTK12用のヒープを作成する関数
/// </summary>
/// <returns>ヒープ</returns>
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
/// ルートシグネチャ・パイプライン・描画方法をセットする関数
/// </summary>
/// <param name="rootSignature">ルートシグネチャ</param>
/// <param name="pipeline">パイプライン</param>
/// <param name="topology">描画方法</param>
void
Dx12Wrapper::SetPipelines(ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipeline, D3D12_PRIMITIVE_TOPOLOGY topology)
{
	_cmdList->SetGraphicsRootSignature(rootSignature);
	_cmdList->SetPipelineState(pipeline);
	_cmdList->IASetPrimitiveTopology(topology);
}

/// <summary>
/// //画面歪み・フェードイン／アウトデータをシェーダーに反映する関数
/// </summary>
void
Dx12Wrapper::UpdateFactor()
{
	//値の範囲を制限
	_dist = clamp(_dist, 0.0f, 1.0f);
	_fade = clamp(_fade, 0.0f, 1.0f);

	//シェーダーへ反映させる
	_mappedFactor->dist = _dist;
	_mappedFactor->fade = _fade;
}

/// <summary>
/// 画面を歪ませるデータを設定する関数
/// </summary>
/// <param name="factor">データの値</param>
void
Dx12Wrapper::SetDist(float dist)
{
	_dist = dist;
}

/// <summary>
/// フェードイン／アウトを実行する関数
/// </summary>
/// <param name="func">各シーンの描画関数</param>
/// <param name="start">フェード値の初期値</param>
/// <param name="end">フェード値の最終値</param>
void
Dx12Wrapper::Fade(function<void()> func, float start, float end)
{
	MSG msg = {};							//メッセージを受け取る構造体

	float t = 0.0f;							//時間
	_start = start;							//フェード値の初期値
	_end = end;								//フェード値の最終値
	for (int i = 0; i < FADE_TIME; i++)
	{
		_fade = lerp(_start, _end, t);		//フェード値を線形補間

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	//メッセージを調べる
		{
			TranslateMessage(&msg);								//メッセージを翻訳
			DispatchMessage(&msg);								//ウィンドウプロシージャへメッセージを送る

			if (msg.message == WM_QUIT)						//アプリが終了したらループを終了させる
			{
				break;
			}
		}
		else
		{
			func();								//各シーンの描画処理を実行
		}

		t += FADE_DIFF;						//加算
	}
}

/// <summary>
/// デバイスを返す関数
/// </summary>
/// <returns>デバイスのポインタ</returns>
ID3D12Device*
Dx12Wrapper::Device()
{
	return _dev.Get();
}

/// <summary>
/// スワップチェーンを返す関数
/// </summary>
/// <returns>スワップチェーンのポインタ</returns>
IDXGISwapChain4*
Dx12Wrapper::Swapchain()
{
	return _swapchain.Get();
}

/// <summary>
/// コマンドリストを返す関数
/// </summary>
/// <returns>コマンドリストのポインタ</returns>
ID3D12GraphicsCommandList*
Dx12Wrapper::CommandList()
{
	return _cmdList.Get();
}

/// <summary>
/// コマンドキューを返す関数
/// </summary>
/// <returns>コマンドキューのポインタ</returns>
ID3D12CommandQueue*
Dx12Wrapper::CommandQueue()
{
	return _cmdQueue.Get();
}

/// <summary>
/// バックバッファー（1枚目）を返す関数
/// </summary>
/// <returns></returns>
ID3D12Resource*
Dx12Wrapper::BackBuffer() const
{
	return _backBuffers[0];
}

/// <summary>
/// RTVヒープを返す関数
/// </summary>
/// <returns>RTVヒープ</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::RTVHeap() const
{
	return _rtvHeap.Get();
}

/// <summary>
/// 深度ステンシルヒープを返す関数
/// </summary>
/// <returns>深度ステンシルヒープ</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::DSVHeap() const
{
	return _dsvHeap.Get();
}

/// <summary>
/// エフェクト用ヒープを返す関数
/// </summary>
/// <returns>エフェクト用ヒープ</returns>
ID3D12DescriptorHeap*
Dx12Wrapper::EffectSRVHeap() const
{
	return _effectSRVHeap.Get();
}

/// <summary>
/// 因数用ヒープを返す関数
/// </summary>
/// <returns>因数用ヒープ</returns>
ID3D12DescriptorHeap* 
Dx12Wrapper::FactorCBVHeap() const
{
	return _factorCBVHeap.Get();
}

/// <summary>
/// ビューポートを返す関数
/// </summary>
/// <returns>ビューポート</returns>
D3D12_VIEWPORT*
Dx12Wrapper::ViewPort() const
{
	return _viewPort.get();
}

/// <summary>
/// シザー矩形を返す関数
/// </summary>
/// <returns>シザー矩形</returns>
D3D12_RECT*
Dx12Wrapper::Rect() const
{
	return _rect.get();
}