#pragma once
#define NOMINMAX
#include "Application.h"
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>

//FBXモデル読み込みに必要な構造体(Applicationクラスで定義)
struct Mesh;
struct FBXVertex;

//FBXモデル読み込みに必要な構造体(Assimpで定義)
struct aiMesh;
struct aiMaterial;

/// <summary>
/// モデル読み込み時に必要な情報をまとめた構造体
/// </summary>
struct ImportSettings
{
	const wchar_t* filename = nullptr;	//ファイルパス
	vector<Mesh>& meshes;				//出力先のメッシュベクトル
	bool inverseU = false;				//U座標を反転させるか
	bool inverseV = false;				//V座標を反転させるか
};

/// <summary>
/// FBXモデルを読み込む為のクラス
/// </summary>
class AssimpLoader
{
public:
	bool Load(ImportSettings setting);	//モデルをロードする関数

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);	//メッシュを読み込む関数
	void LoadTexture(const wchar_t* filename,Mesh& dst,const aiMaterial* src);	//テクスチャを読み込む関数	
};