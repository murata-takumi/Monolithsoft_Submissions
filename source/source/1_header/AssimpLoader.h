#pragma once
#define NOMINMAX
#include "Application.h"
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>

//FBX���f���ǂݍ��݂ɕK�v�ȍ\����(Application�N���X�Œ�`)
struct Mesh;
struct FBXVertex;

//FBX���f���ǂݍ��݂ɕK�v�ȍ\����(Assimp�Œ�`)
struct aiMesh;
struct aiMaterial;

/// <summary>
/// ���f���ǂݍ��ݎ��ɕK�v�ȏ����܂Ƃ߂��\����
/// </summary>
struct ImportSettings
{
	const wchar_t* filename = nullptr;	//�t�@�C���p�X
	vector<Mesh>& meshes;				//�o�͐�̃��b�V���x�N�g��
	bool inverseU = false;				//U���W�𔽓]�����邩
	bool inverseV = false;				//V���W�𔽓]�����邩
};

/// <summary>
/// FBX���f����ǂݍ��ވׂ̃N���X
/// </summary>
class AssimpLoader
{
public:
	bool Load(ImportSettings setting);	//���f�������[�h����֐�

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);	//���b�V����ǂݍ��ފ֐�
	void LoadTexture(const wchar_t* filename,Mesh& dst,const aiMaterial* src);	//�e�N�X�`����ǂݍ��ފ֐�	
};