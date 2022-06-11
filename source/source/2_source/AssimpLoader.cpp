#include "AssimpLoader.h"
#include "Functions.h"

/// <summary>
/// FBXモデルを読み込む関数
/// </summary>
/// <param name="setting">ImportSettings構造体</param>
/// <returns>読み込めたかどうか</returns>
bool
AssimpLoader::Load(ImportSettings setting)
{
	//ファイル名がおかしかったら処理中断
	if (setting.filename == nullptr)
	{
		return false;
	}

	auto& meshes = setting.meshes;		//メッシュ配列
	auto inverseU = setting.inverseU;	//U座標を反転させるか
	auto inverseV = setting.inverseV;	//V座標を反転させるか

	auto path = ToString(setting.filename);	//ファイル名からパスを取得

	Assimp::Importer importer;			//インポーター
	
	int flag = 0;						//ビット演算を行い、フラグを設定する
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_OptimizeMeshes;

	auto scene = importer.ReadFile(path, flag);		//インポーターにファイルパスとフラグを読み込ませる
	if (scene == nullptr)
	{
		return false;
	}

	//メッシュ読み込み処理
	if (0 < scene->mNumMeshes)
	{
		meshes.clear();										//メッシュをリセット
		meshes.resize(scene->mNumMeshes);					//メッシュ数を再設定
		for (size_t i = 0; i < meshes.size(); ++i)
		{
			const aiMesh* pMesh = scene->mMeshes[i];			//各メッシュを取得
			LoadMesh(meshes[i], pMesh, inverseU, inverseV);		//メッシュを読み込み

			const aiMaterial* pMaterial = scene->mMaterials[pMesh->mMaterialIndex];			//各マテリアルを取得
			LoadTexture(setting.filename, meshes[i], pMaterial);
		}
	}

	scene = nullptr;	//開放

	return true;
}

/// <summary>
/// FBXのメッシュを読み込む関数
/// </summary>
/// <param name="dst">メッシュ配列</param>
/// <param name="src">FBXモデル内のメッシュ</param>
/// <param name="inverseU">U座標を反転させるか</param>
/// <param name="inverseV">V座標を反転させるか</param>
void
AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV)
{
	aiVector3D zero3D(0.0f, 0.0f, 0.0f);				//メッシュを構成する頂点の座標
	aiColor4D zeroColor(0.0f, 0.0f, 0.0f, 0.0f);		//頂点のカラー

	dst.vertices.resize(src->mNumVertices);				//頂点数を再設定

	//頂点毎に処理する
	for (auto i = 0u; i < src->mNumVertices; ++i)
	{
		auto position = &(src->mVertices[i]);												//頂点座標
		auto normal = &(src->mNormals[i]);													//法線
		auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;		//UV座標
		auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;	//正接
		auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;		//頂点カラー

		//反転オプションがあったらUV座標を反転させる
		if (inverseU)
		{
			uv->x = 1 - uv->x;
		}

		if (inverseV)
		{
			uv->y = 1 - uv->y;
		}

		FBXVertex vertex = {};													//頂点
		vertex.position = XMFLOAT3(position->x, position->y, position->z);		//座標
		vertex.normal = XMFLOAT3(normal->x, normal->y, normal->z);				//法線
		vertex.uv = XMFLOAT2(uv->x, uv->y);										//UV座標
		vertex.tangent = XMFLOAT3(tangent->x, tangent->y, tangent->z);			//正接?
		vertex.color = XMFLOAT4(color->r, color->g, color->b, color->a);		//頂点カラー

		dst.vertices[i] = vertex;	//格納
	}

	dst.indices.resize(src->mNumFaces * 3);	//インデックス数をメッシュ数×3に調整

	//インデックス情報の格納
	for (auto i = 0u; i < src->mNumFaces; ++i)
	{
		const auto& face = src->mFaces[i];

		dst.indices[i * 3 + 0] = face.mIndices[0];
		dst.indices[i * 3 + 1] = face.mIndices[1];
		dst.indices[i * 3 + 2] = face.mIndices[2];
	}
}

/// <summary>
/// FBXファイルからテクスチャを読み込む関数
/// </summary>
/// <param name="filename">FBXファイル名</param>
/// <param name="dst">メッシュ配列</param>
/// <param name="src">マテリアル情報</param>
void
AssimpLoader::LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* src)
{
	aiString path;

	if (src->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		//テクスチャパスは相対パスで入っているので、絶対パスに直すためファイルの場所とくっ付ける必要がある
		auto dir = GetDirectoryPath(filename);
		auto file = string(path.C_Str());

		size_t found1 = file.find("..");
		if (found1 != string::npos)
		{
			file.replace(found1, 2, "");
		}

		size_t found2 = file.find("\\");

		//ファイルのパスに"\\"が含まれていたら処理を実行
		if (found2 != string::npos)
		{
			file.replace(found2, 1, "/");
		}

		dst.diffuseMap = dir + ToWideString(file);	//ディフューズのパスを設定
	}
	else
	{
		dst.diffuseMap.clear();	//ディフューズのパスをリセット
	}
}

