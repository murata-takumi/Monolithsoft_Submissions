#pragma once
#include"Application.h"

/// <summary>
/// 3Dモデル及びそのデータを管理するクラス
/// </summary>
class Dx12Wrapper;
class FBXActor
{
	SIZE _winSize;	//ウィンドウサイズ
	template<typename T>
	using ComPtr = ComPtr<T>;

private:
	HRESULT result;								//関数の返り値

	Dx12Wrapper& _dx12;							//Dx12Wrapperインスタンス

	vector<Mesh> _meshes;						//モデル読み込み用メッシュ配列

	vector<ID3D12Resource*> _vertexBuffers;		//メッシュ用頂点バッファー配列
	vector<D3D12_VERTEX_BUFFER_VIEW> _VBViews;	//メッシュ用頂点バッファービュー配列

	vector<ID3D12Resource*> _indexBuffers;		//メッシュ用インデックスバッファー配列
	vector<D3D12_INDEX_BUFFER_VIEW> _IBViews;	//メッシュ用インデックスバッファービュー配列

	ComPtr<ID3D12Resource> _transformBuffer;			//ワールド行列用バッファー
	ComPtr<ID3D12DescriptorHeap> _transformHeap;		//ワールド行列用ヒープ

	ComPtr<ID3D12DescriptorHeap> _texHeap;				//テクスチャ用ヒープ
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> _GPUHandles;		//テクスチャバッファービューのGPUハンドル配列

	bool LoadFBXModel(const wchar_t* modelPath);	//FBXモデル読み込み関数

	HRESULT CreateVertexBufferView();				//頂点バッファー・ビュー作成関数

	HRESULT CreateIndexBufferView();				//インデックスバッファー・ビュー作成関数

	HRESULT CreateTransformView();					//座標変換用バッファー・ビュー作成関数

	HRESULT CreateShaderResourceView();				//シェーダーリソース・ビュー作成関数
public:
	FBXActor(Dx12Wrapper& dx12, const wchar_t* modelPath);	//コンストラクタ

	void Update();	//毎フレームの処理
};