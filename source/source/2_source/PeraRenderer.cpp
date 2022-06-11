#include "PeraRenderer.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

/// <summary>
/// ペラポリゴン用RT・RTV・SRVを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
PeraRenderer::CreatePeraResourcesAndView()
{
	auto resDesc = _dx12.BackBuffer()->GetDesc();	//元々使用していたリソース設定用構造体を利用

	//クリア値はレンダリング時と同じ値を設定する必要がある
	float clsClr[4] = { 1.0f,1.0f,1.0f,1.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	//リソース(レンダーターゲット)の作成
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

	//RTV用ヒープ用構造体の設定
	auto heapDesc = _dx12.RTVHeap()->GetDesc();
	heapDesc.NumDescriptors = 1;

	//RTV用ヒープの作成
	result = _dx12.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	if (FAILED(result))
	{
		assert(0);
		return result;
	}

	//RTV用構造体の作成・設定
	D3D12_RENDER_TARGET_VIEW_DESC peraRTVDesc = {};
	peraRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	peraRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//RTVの作成
	_dx12.Device()->CreateRenderTargetView(
		_peraResource.Get(),
		&peraRTVDesc,
		_peraRTVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//SRV用ヒープ用構造体の設定
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//SRV用ヒープの作成
	result = _dx12.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	//SRV用構造体の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC peraSRVDesc = {};
	peraSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	peraSRVDesc.Format = peraRTVDesc.Format;
	peraSRVDesc.Texture2D.MipLevels = 1;
	peraSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//SRVの作成
	_dx12.Device()->CreateShaderResourceView(
		_peraResource.Get(),
		&peraSRVDesc,
		_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return result;
}

/// <summary>
/// ペラポリゴン用頂点バッファー・VBVを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
PeraRenderer::CreatePeraVertex()
{
	//ペラポリゴンを作成するための頂点構造体
	struct PeraVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	//ペラポリゴンを構成する4つの頂点
	PeraVertex pv[] =
	{
		{{-1.0f,-1.0f,0.1f},{0.0f,1.0f}},
		{{-1.0f,1.0f,0.1f},{0.0f,0.0f}},
		{{1.0f,-1.0f,0.1f},{1.0f,1.0f}},
		{{1.0f,1.0f,0.1f},{1.0f,0.0f}}
	};

	auto pvResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));	//リソースのサイズを設定

	//ペラポリゴン用頂点バッファーの作成
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

	//ペラポリゴンのマップ
	PeraVertex* mappedPera = nullptr;
	_peraVB->Map(0, nullptr, (void**)&mappedPera);
	copy(begin(pv), end(pv), mappedPera);
	_peraVB->Unmap(0, nullptr);

	//頂点バッファービューの設定
	_peraVBV.BufferLocation = _peraVB->GetGPUVirtualAddress();
	_peraVBV.SizeInBytes = sizeof(pv);
	_peraVBV.StrideInBytes = sizeof(PeraVertex);

	return result;
}

/// <summary>
/// ペラポリゴン用パイプラインを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
PeraRenderer::CreatePeraPipeline()
{
	//ペラポリゴン用頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		//頂点座標
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		//UV座標
		{
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
	};

	//ペラポリゴン用パイプライン設定構造体の作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);	//入力レイアウト数
	gpsDesc.InputLayout.pInputElementDescs = layout;	//入力レイアウト設定

	ID3DBlob* vs = nullptr;			//頂点シェーダー用データ
	ID3DBlob* ps = nullptr;			//ピクセルシェーダー用データ
	ID3DBlob* errorBlob = nullptr;	//エラー確認用データ
	auto result = S_OK;

#ifdef _DEBUG
	//頂点シェーダー読み込み
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

	//ピクセルシェーダー読み込み
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

	//構造体に頂点・ピクセルシェーダーを設定
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
#else
	//構造体に頂点・ピクセルシェーダーを設定
	gpsDesc.VS.BytecodeLength = sizeof(g_vs);
	gpsDesc.VS.pShaderBytecode = &g_vs;
	gpsDesc.PS.BytecodeLength = sizeof(g_ps);
	gpsDesc.PS.pShaderBytecode = &g_ps;
#endif // _DEBUG

	//以下パイプラインステートの設定
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	//ディスクリプタレンジ
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

	//ルートパラメータ
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

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);	//サンプラー

	//ルートシグネチャ設定構造体の作成
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 3;
	rsDesc.pParameters = rp;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rsBlob = nullptr;	//ルートシグネチャ用データ

	//ルートシグネチャ用データの初期化
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

	//ルートシグネチャの作成
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

	rsBlob->Release();	//不要になったので開放

	gpsDesc.pRootSignature = _peraRS.Get();	//パイプラインへルートシグネチャを登録

	result = _dx12.Device()->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_peraPipeline));	//パイプラインの作成
	if (result != S_OK)
	{
		assert(0);
		return result;
	}

	return result;
}

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
PeraRenderer::PeraRenderer(Dx12Wrapper& dx12):_dx12(dx12)
{
	CreatePeraResourcesAndView();			//ペラポリゴン用RT・RTV・SRVを作成
	 
	CreatePeraVertex();						//ペラポリゴン用頂点バッファー・VBVを作成
	
	CreatePeraPipeline();					//ペラポリゴン用パイプラインを作成
}

/// <summary>
/// デコンストラクタ
/// 特に処理はしない
/// </summary>
PeraRenderer::~PeraRenderer()
{

}

/// <summary>
/// ペラポリゴン用リソースの遷移(SHADER_RESOURCE→RENDER_TARGET)・RTVのセットを実行する関数
/// </summary>
void
PeraRenderer::BeginPeraDraw()
{
	//ペラポリゴン用リソースをRENDER_TARGETに遷移
	_dx12.BarrierTransition(_peraResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtvH = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();	//ペラポリゴン用RTV/SRVヒープのハンドルを取得
	auto dsvH = _dx12.DSVHeap()->GetCPUDescriptorHandleForHeapStart();		//深度ステンシルヒープのハンドルを取得

	//レンダーターゲット・深度ステンシルヒープのハンドルをコマンドリストに設定
	_dx12.CommandList()->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };	//背景色を設定

	//レンダーターゲット・深度ステンシルビューをクリア
	_dx12.CommandList()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_dx12.CommandList()->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	_dx12.CommandList()->RSSetViewports(1, _dx12.ViewPort());		//ビューポートをコマンドリストに設定
	_dx12.CommandList()->RSSetScissorRects(1, _dx12.Rect());		//シザー矩形をコマンドリストに設定
}

/// <summary>
/// ペラポリゴン用ルートシグネチャ・パイプラインをセットする関数
/// </summary>
void
PeraRenderer::SetPeraPipelines()
{
	//パイプラインステート、ルートシグネチャ、プリミティブトポロジーを設定する関数
	_dx12.SetPipelines(_peraRS.Get(), _peraPipeline.Get(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//ペラポリゴン用SRV、及びハンドルを設定
	ID3D12DescriptorHeap* peraSRVHeaps[] = { _peraSRVHeap.Get() };
	_dx12.CommandList()->SetDescriptorHeaps(1, peraSRVHeaps);
	auto handle = _peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(0, handle);

	//エフェクト用SRV、及びハンドルを設定
	ID3D12DescriptorHeap* effectSRVHeap[] = { _dx12.EffectSRVHeap()};
	_dx12.CommandList()->SetDescriptorHeaps(1, effectSRVHeap);
	handle = _dx12.EffectSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(1, handle);

	//エフェクト適用データ用CBV、及びハンドルを設定
	ID3D12DescriptorHeap* factorCBVHeap[] = { _dx12.FactorCBVHeap() };
	_dx12.CommandList()->SetDescriptorHeaps(1, factorCBVHeap);
	handle = _dx12.FactorCBVHeap()->GetGPUDescriptorHandleForHeapStart();
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, handle);

	//データを更新
	_dx12.UpdateFactor();
}

/// <summary>
/// ペラポリゴン用リソースの遷移(RENDER_TARGET→SHADER_RESOURCE)を実行する関数
/// </summary>
void
PeraRenderer::EndPeraDraw()
{
	//ペラポリゴン用リソースをSHADER_RESOURCEに遷移
	_dx12.BarrierTransition(_peraResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

/// <summary>
/// ペラポリゴン用頂点バッファービューを返す関数
/// </summary>
/// <returns>ペラポリゴン用頂点バッファービュー</returns>
D3D12_VERTEX_BUFFER_VIEW
PeraRenderer::PeraVBView()
{
	return _peraVBV;
}

/// <summary>
/// ペラポリゴン用パイプラインを返す関数
/// </summary>
/// <returns>ペラポリゴン用パイプラインのポインタ</returns>
ID3D12PipelineState*
PeraRenderer::PeraPipeline()
{
	return _peraPipeline.Get();
}

/// <summary>
/// ペラポリゴン用ルートシグネチャを返す関数
/// </summary>
/// <returns>ペラポリゴン用ルートシグネチャのポインタ</returns>
ID3D12RootSignature*
PeraRenderer::PeraRootSignature()
{
	return _peraRS.Get();
}