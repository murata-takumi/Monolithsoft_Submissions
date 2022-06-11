#pragma once
#include "Application.h"

/// <summary>
/// ペラポリゴンに関するオブジェクト、処理を管理するクラス
/// </summary>
class Dx12Wrapper;
class PeraRenderer
{
private:
	Dx12Wrapper& _dx12;		//Dx12Wrapperインスタンス

	//ペラポリゴン周り
	ComPtr<ID3D12Resource> _peraVB;					//ペラポリゴン用の頂点バッファー
	ComPtr<ID3D12Resource> _peraResource;			//RTV/SRV両方を書き込むためのリソース
	ComPtr<ID3D12DescriptorHeap> _peraRTVHeap;		//RTV用ヒープ
	ComPtr<ID3D12DescriptorHeap> _peraSRVHeap;		//SRV用ヒープ
	ComPtr<ID3D12PipelineState> _peraPipeline;		//ペラポリゴン用のパイプラインステート
	ComPtr<ID3D12RootSignature> _peraRS;			//ペラポリゴン用のルートシグネチャ

	D3D12_VERTEX_BUFFER_VIEW _peraVBV = {};		//ペラポリゴン用の頂点バッファービュー

	HRESULT CreatePeraResourcesAndView();		//ペラポリゴン用RT・SR・RTV・SRVを作成する関数

	HRESULT CreatePeraVertex();					//ペラポリゴン用頂点バッファー・VBVを作成する関数

	HRESULT CreatePeraPipeline();				//ペラポリゴン用パイプラインを作成する関数

public:

	PeraRenderer(Dx12Wrapper& dx12);	//コンストラクタ
	~PeraRenderer();					//デコンストラクタ

	void BeginPeraDraw();		//ペラポリゴン用リソースの遷移(SHADER_RESOURCE→RENDER_TARGET)・RTVのセットを実行する関数
	void SetPeraPipelines();	//ペラポリゴン用ルートシグネチャ・パイプラインをセットする関数
	void EndPeraDraw();			//ペラポリゴン用リソースの遷移(RENDER_TARGET→SHADER_RESOURCE)を実行する関数

	D3D12_VERTEX_BUFFER_VIEW PeraVBView();		//ペラポリゴン用頂点バッファービューを返す関数
	ID3D12PipelineState* PeraPipeline();		//ペラポリゴン用パイプラインを返す関数
	ID3D12RootSignature* PeraRootSignature();	//ペラポリゴン用ルートシグネチャを返す関数
};