#include "Renderer.h"
#include "BinaryFile.h"
#include "Dx12Wrapper.h"
#include "Functions.h"

/// <summary>
/// コンストラクタ
/// :_dx12(dx12)は_dx12変数に引数を格納しているのを示す
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
Renderer::Renderer(Dx12Wrapper& dx12):_dx12(dx12)
{
	CreateRootSignature();						//ルートシグネチャ初期化
	CreateGraphicsPipelineForPMD();				//パイプラインステート初期化
}

/// <summary>
/// ルートシグネチャを初期化する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Renderer::CreateRootSignature()
{
	//シェーダーのスロットと紐付くディスクリプタレンジの作成
	CD3DX12_DESCRIPTOR_RANGE descTblRange[3] = {};
	descTblRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	//定数[b0]
	descTblRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	//定数[b1]
	descTblRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	//テクスチャ[t0]


	//バッファーとディスクリプタレンジを紐づけるルートパラメータの作成
	CD3DX12_ROOT_PARAMETER rootparam[3] = {};
	rootparam[0].InitAsDescriptorTable(1, &descTblRange[0]);
	rootparam[1].InitAsDescriptorTable(1, &descTblRange[1]);
	rootparam[2].InitAsDescriptorTable(1, &descTblRange[2], D3D12_SHADER_VISIBILITY_PIXEL);

	//サンプラー(テクスチャのXY座標の取得方法を決めるオブジェクト)の作成
	CD3DX12_STATIC_SAMPLER_DESC samplerDescs = {};
	samplerDescs.Init(0);

	//設定用構造体の作成・設定
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(3,rootparam,1,&samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//ルートシグネチャの初期化
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

	//作成
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

	rootSigBlob->Release();		//不要になったデータを開放

	return result;
}

/// <summary>
/// グラフィックスパイプラインを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
Renderer::CreateGraphicsPipelineForPMD()
{
	ID3DBlob* vsBlob = nullptr;		//頂点シェーダーオブジェクト
	ID3DBlob* psBlob = nullptr;		//ピクセルシェーダーオブジェクト
	ID3DBlob* errorBlob = nullptr;	//エラー判別用オブジェクト

	auto result = S_OK;

	//入力レイアウト(GPUに頂点データをどう解釈するかを教えるデータ)
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeline = {};			//パイプラインステート設定用構造体
	gPipeline.pRootSignature = _rootSignature.Get();			//ルートシグネチャセット

#ifdef _DEBUG
		//頂点シェーダー読み込み
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

	//ピクセルシェーダー読み込み
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

	gPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob);				//頂点シェーダーセット
	gPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob);				//ピクセルシェーダーセット

#else
	gPipeline.VS.BytecodeLength = sizeof(g_FBXVS);				//頂点シェーダーセット
	gPipeline.VS.pShaderBytecode = &g_FBXVS;
	gPipeline.PS.BytecodeLength = sizeof(g_FBXPS);				//頂点シェーダーセット
	gPipeline.PS.pShaderBytecode = &g_FBXPS;
#endif // DEBUG

	gPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;			//サンプルマスク

	gPipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);	//ブレンドステート
	
	gPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);	//ラスタライザーステート(ポリゴンをピクセルに変換する仕組み)
	gPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;			//カリングモード(描画不要なポリゴンを描画しないよう仕組み)

	//深度値周りの設定
	gPipeline.DepthStencilState.DepthEnable = true;								//深度値を有効に
	gPipeline.DepthStencilState.StencilEnable = false;							//ステンシル値は無効のまま
	gPipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//ピクセル描画時に深度バッファーに深度値を書き込むよう設定
	gPipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;			//ピクセル同士の深度を比較し、小さい方を描画

	gPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//32bit値を深度値として使用する

	gPipeline.InputLayout.pInputElementDescs = inputLayout;		//入力レイアウト設定
	gPipeline.InputLayout.NumElements = _countof(inputLayout);	//レイアウト配列の要素数

	gPipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	//ポリゴンの切り取り方
	gPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//ポリゴンを三角形で表現する

	gPipeline.NumRenderTargets = 1;								//レンダーターゲット数
	gPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//レンダーターゲットのフォーマット

	gPipeline.SampleDesc.Count = 1;		//ピクセル当たりサンプリング数
	gPipeline.SampleDesc.Quality = 0;	//クオリティ

	result = _dx12.Device()->CreateGraphicsPipelineState(&gPipeline, IID_PPV_ARGS(_pipeline.ReleaseAndGetAddressOf()));	//パイプラインステート作成

	return result;	//関数が成功したのを返す
}

/// <summary>
/// シェーダー読み込みが成功したかどうかを判別する関数
/// </summary>
/// <param name="result">シェーダー読み込み関数の返り値</param>
/// <param name="error">エラーデータ格納用オブジェクト</param>
/// <returns>成功したかどうか</returns>
bool
Renderer::CheckCompilerResult(HRESULT result, ID3DBlob* error)
{
	//関数が失敗したら
	if (FAILED(result))
	{
		//エラー種類が以下のものだったらファイルが見つからない旨を記述
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見つかりません");
		}
		else
		{
			string errStr;							//エラー情報記述用文字列
			errStr.resize(error->GetBufferSize());	//文字列のサイズを変更

			//オブジェクトのアドレスからバッファーサイズ分データを取得し、文字列にコピーする
			copy_n((char*)error->GetBufferPointer(),error->GetBufferSize(),errStr.begin());

			errStr += "\n";							//末尾に改行文字を加える
			OutputDebugStringA(errStr.c_str());		//出力
		}
		return false;	//関数が失敗したら偽を返す
	}
	else
	{
		return true;	//関数が成功したら真を返す
	}
}

/// <summary>
///	パイプラインステートを返す関数
/// </summary>
/// <returns>パイプラインステート</returns>
ID3D12PipelineState*
Renderer::GetPipelineState()
{
	return _pipeline.Get();
}

/// <summary>
/// ルートシグネチャを返す関数
/// </summary>
/// <returns>ルートシグネチャ</returns>
ID3D12RootSignature*
Renderer::GetRootSignature()
{
	return _rootSignature.Get();
}