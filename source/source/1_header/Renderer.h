#pragma once
#include"Application.h"

/// <summary>
/// パイプラインステートやルートシグネチャ等の描画処理に必要なオブジェクトを管理するクラス
/// </summary>
class Dx12Wrapper;
class Renderer
{
	SIZE _winSize;	//ウィンドウサイズ
	template<typename T>
	using ComPtr = ComPtr<T>;

private:
	Dx12Wrapper& _dx12;	//Dx12Wrapperインスタンス

	ComPtr<ID3D12PipelineState> _pipeline;			//グラフィックスパイプラインステート(パイプライン設定を定義するオブジェクト)
	ComPtr<ID3D12RootSignature> _rootSignature;		//ルートシグネチャ(スロットと各ビューが管理するバッファーを関連付ける)

	HRESULT CreateRootSignature();								//ルートシグネチャ初期化関数
	HRESULT CreateGraphicsPipelineForPMD();						//パイプラインステート初期化関数
	bool CheckCompilerResult(HRESULT result, ID3DBlob* error);	//シェーダー読み込みの成否を確認する関数

public:
	Renderer(Dx12Wrapper& dx12);				//コンストラクタ
	ID3D12PipelineState* GetPipelineState();	//パイプラインステートを返す関数
	ID3D12RootSignature* GetRootSignature();	//ルートシグネチャを返す関数
};