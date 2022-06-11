#include "AssimpLoader.h"
#include "Dx12Wrapper.h"
#include "FBXActor.h"
#include "Functions.h"

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="dx12">Dx12Wrapperインスタンス</param>
/// <param name="modelPath">FBXファイルのパス</param>
FBXActor::FBXActor(Dx12Wrapper& dx12, const wchar_t* modelPath):_dx12(dx12)
{
	result = S_OK;		//関数の返り値を初期化

	LoadFBXModel(modelPath);	//FBXモデル読み込み

	CreateVertexBufferView();	//頂点バッファー・ビュー作成

	CreateIndexBufferView();	//インデックスバッファー・ビュー作成

	CreateTransformView();		//座標変換用バッファー・ビュー作成

	CreateShaderResourceView();	//シェーダーリソース・ビュー作成
}

/// <summary>
/// FBXモデルを読み込む関数
/// </summary>
/// <param name="modelPath">FBXモデルのファイルパス</param>
/// <returns>処理が成功したかどうか</returns>
bool
FBXActor::LoadFBXModel(const wchar_t* modelPath)
{
	//モデル読み込み用設定
	ImportSettings settings =
	{
		modelPath,
		_meshes,
		false,
		true,
	};

	//FBXモデルを読み込む
	AssimpLoader loader;
	if (!loader.Load(settings))
	{
		assert(0);
		return false;
	}

	return true;
}

/// <summary>
/// メッシュ毎に頂点バッファー・ビューを作成する関数
/// </summary>
/// <returns>処理が成功したかどうか</returns>
HRESULT 
FBXActor::CreateVertexBufferView()
{
	result = S_OK;	//返り値を初期化

	//頂点バッファー・ビューを用意する
	_vertexBuffers.reserve(_meshes.size());
	_VBViews.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpVertexBuffer = nullptr;							//格納用バッファー
		D3D12_VERTEX_BUFFER_VIEW tmpVBView = {};							//格納用ビュー

		auto size = sizeof(FBXVertex) * _meshes[i].vertices.size();			//頂点全体のデータサイズ
		auto stride = sizeof(FBXVertex);									//頂点一個のデータサイズ

		auto vertexResDesc = CD3DX12_RESOURCE_DESC::Buffer(size);					//リソース設定

		//バッファー作成
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

		auto data = _meshes[i].vertices;	//頂点データ取得

		//データをバッファーへ書き込む
		FBXVertex* mappedVertex = nullptr;
		tmpVertexBuffer->Map(0, nullptr, (void**)&mappedVertex);
		copy(begin(data), end(data), mappedVertex);
		tmpVertexBuffer->Unmap(0, nullptr);

		//ビューにバッファー情報を書き込む
		tmpVBView.BufferLocation = tmpVertexBuffer->GetGPUVirtualAddress();
		tmpVBView.SizeInBytes = (UINT)tmpVertexBuffer->GetDesc().Width;
		tmpVBView.StrideInBytes = sizeof(FBXVertex);

		//頂点バッファー・ビューを配列に追加
		_vertexBuffers.push_back(tmpVertexBuffer);
		_VBViews.push_back(tmpVBView);

	}

	return result;
}

/// <summary>
/// メッシュ毎にインデックスバッファー・ビューを作成する関数
/// </summary>
/// <returns></returns>
HRESULT
FBXActor::CreateIndexBufferView()
{
	result = S_OK;	//返り値を初期化

	//インデックスバッファー・ビューを用意する
	_indexBuffers.reserve(_meshes.size());
	_IBViews.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpIndexBuffer = nullptr;	//格納用バッファー
		D3D12_INDEX_BUFFER_VIEW tmpIBView = {};		//格納用ビュー

		auto size = sizeof(uint32_t) * _meshes[i].indices.size();	//インデックス全体のデータサイズ

		auto indexResDesc = CD3DX12_RESOURCE_DESC::Buffer(size);					//リソース設定

		//バッファー作成
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

		auto data = _meshes[i].indices;	//インデックスデータ

		//データをバッファーに書き込む
		uint32_t* mappedIndex = nullptr;
		tmpIndexBuffer->Map(0, nullptr, (void**)&mappedIndex);
		copy(begin(data), end(data), mappedIndex);
		tmpIndexBuffer->Unmap(0, nullptr);

		//ビューにバッファー情報を書き込む
		tmpIBView.BufferLocation = tmpIndexBuffer->GetGPUVirtualAddress();
		tmpIBView.Format = DXGI_FORMAT_R32_UINT;
		tmpIBView.SizeInBytes = static_cast<UINT>(size);

		//インデックスバッファー・ビューを配列に追加
		_indexBuffers.push_back(tmpIndexBuffer);
		_IBViews.push_back(tmpIBView);
	}

	return result;
}

/// <summary>
/// モデル用トランスフォームバッファー・ビューを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
FBXActor::CreateTransformView()
{
	result = S_OK;

	//ワールド行列用バッファーの作成
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

	//データの書き込み
	XMMATRIX* mappedMat = nullptr;
	_transformBuffer->Map(0, nullptr, (void**)&mappedMat);
	*mappedMat = XMMatrixIdentity();

	//ディスクリプタヒープ設定用構造体の作成
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;									//とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;
	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別

	//ヒープの作成
	result = _dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	//ビュー設定用構造体の作成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)_transformBuffer->GetDesc().Width;

	//ビューの作成
	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

/// <summary>
/// モデル用テクスチャバッファー・ビューを作成する関数
/// </summary>
/// <returns>関数が成功したかどうか</returns>
HRESULT
FBXActor::CreateShaderResourceView()
{
	result = S_OK;

	//テクスチャ用ディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.NodeMask = 1;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	texHeapDesc.NumDescriptors = (UINT)_meshes.size();
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	_dx12.Device()->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(_texHeap.ReleaseAndGetAddressOf()));

	//テクスチャバッファービュー用構造体の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto CPUHeapHandle = _texHeap->GetCPUDescriptorHandleForHeapStart();							//ヒープの先頭アドレス(CPU)
	auto GPUHeapHandle = _texHeap->GetGPUDescriptorHandleForHeapStart();							//ヒープの先頭アドレス(GPU)	
	auto incrementSize = _dx12.Device()->GetDescriptorHandleIncrementSize(texHeapDesc.Type);	//アドレスのずらす幅

	//テクスチャ読み込み用データ
	TexMetadata meta = {};
	ScratchImage scratch = {};

	DXGI_FORMAT format;		//テクスチャのフォーマット
	size_t width;			//幅
	size_t height;			//高さ
	UINT16 arraySize;		//テクスチャサイズ
	UINT16 mipLevels;
	void* pixels;
	UINT rowPitch;
	UINT slicePitch;

	//テクスチャの読み込み処理
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		ID3D12Resource* tmpTexBuffer = nullptr;

		auto path = _meshes[i].diffuseMap;	//テクスチャのパス

		//白テクスチャ
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
		//通常テクスチャ
		else
		{
			auto ext = FileExtension(path);		//拡張子を取得
			
			//拡張子が"psd"だった場合、"tga"に変換する
			if (wcscmp(ext.c_str(), L"psd") == 0)
			{
				path = ReplaceExtension(path, "tga");	//パスの拡張子を変換
				ext = FileExtension(path);				//拡張子を取得しなおす
			}

			result = _dx12._loadLambdaTable[ToString(ext)](path, &meta, scratch);	//拡張子に応じて読み込み関数を変える
			if (FAILED(result))
			{
				assert(0);
				return result;
			}

			//テクスチャデータの用意
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

		//シェーダーリソースの作成
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

		//テクスチャの書き込み
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

		srvDesc.Format = tmpTexBuffer->GetDesc().Format;									//フォーマットを合わせる
		_dx12.Device()->CreateShaderResourceView(tmpTexBuffer, &srvDesc, CPUHeapHandle);	//テクスチャバッファービューを作成

		_GPUHandles.push_back(GPUHeapHandle);	//GPUのアドレスを追加

		CPUHeapHandle.ptr += incrementSize;	//CPUのアドレスをずらす
		GPUHeapHandle.ptr += incrementSize;	//GPUのアドレスをずらす
	}

	return result;
}

/// <summary>
/// 毎フレームの更新処理
/// </summary>
void
FBXActor::Update()
{
	//座標変換用ディスクリプタヒープをセット
	ID3D12DescriptorHeap* transformHeaps[] = { _transformHeap.Get() };
	_dx12.CommandList()->SetDescriptorHeaps(1, transformHeaps);	

	//ルートパラメータとディスクリプタヒープのハンドルを関連付け
	_dx12.CommandList()->SetGraphicsRootDescriptorTable(1, _transformHeap->GetGPUDescriptorHandleForHeapStart());

	//頂点・インデックスバッファービューのセット、頂点描画
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		_dx12.CommandList()->IASetVertexBuffers(0, 1, &_VBViews[i]);
		_dx12.CommandList()->IASetIndexBuffer(&_IBViews[i]);

		//テクスチャバッファービューのセット

		ID3D12DescriptorHeap* SetTexHeap[] = { _texHeap.Get() };
		_dx12.CommandList()->SetDescriptorHeaps(1, SetTexHeap);
		_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, _GPUHandles[i]);

		_dx12.CommandList()->DrawIndexedInstanced((UINT)_meshes[i].indices.size(), 1, 0, 0, 0);
	}
}