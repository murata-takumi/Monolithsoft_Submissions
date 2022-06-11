#include "AssimpLoader.h"
#include "Functions.h"

/// <summary>
/// FBX���f����ǂݍ��ފ֐�
/// </summary>
/// <param name="setting">ImportSettings�\����</param>
/// <returns>�ǂݍ��߂����ǂ���</returns>
bool
AssimpLoader::Load(ImportSettings setting)
{
	//�t�@�C�������������������珈�����f
	if (setting.filename == nullptr)
	{
		return false;
	}

	auto& meshes = setting.meshes;		//���b�V���z��
	auto inverseU = setting.inverseU;	//U���W�𔽓]�����邩
	auto inverseV = setting.inverseV;	//V���W�𔽓]�����邩

	auto path = ToString(setting.filename);	//�t�@�C��������p�X���擾

	Assimp::Importer importer;			//�C���|�[�^�[
	
	int flag = 0;						//�r�b�g���Z���s���A�t���O��ݒ肷��
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_OptimizeMeshes;

	auto scene = importer.ReadFile(path, flag);		//�C���|�[�^�[�Ƀt�@�C���p�X�ƃt���O��ǂݍ��܂���
	if (scene == nullptr)
	{
		return false;
	}

	//���b�V���ǂݍ��ݏ���
	if (0 < scene->mNumMeshes)
	{
		meshes.clear();										//���b�V�������Z�b�g
		meshes.resize(scene->mNumMeshes);					//���b�V�������Đݒ�
		for (size_t i = 0; i < meshes.size(); ++i)
		{
			const aiMesh* pMesh = scene->mMeshes[i];			//�e���b�V�����擾
			LoadMesh(meshes[i], pMesh, inverseU, inverseV);		//���b�V����ǂݍ���

			const aiMaterial* pMaterial = scene->mMaterials[pMesh->mMaterialIndex];			//�e�}�e���A�����擾
			LoadTexture(setting.filename, meshes[i], pMaterial);
		}
	}

	scene = nullptr;	//�J��

	return true;
}

/// <summary>
/// FBX�̃��b�V����ǂݍ��ފ֐�
/// </summary>
/// <param name="dst">���b�V���z��</param>
/// <param name="src">FBX���f�����̃��b�V��</param>
/// <param name="inverseU">U���W�𔽓]�����邩</param>
/// <param name="inverseV">V���W�𔽓]�����邩</param>
void
AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV)
{
	aiVector3D zero3D(0.0f, 0.0f, 0.0f);				//���b�V�����\�����钸�_�̍��W
	aiColor4D zeroColor(0.0f, 0.0f, 0.0f, 0.0f);		//���_�̃J���[

	dst.vertices.resize(src->mNumVertices);				//���_�����Đݒ�

	//���_���ɏ�������
	for (auto i = 0u; i < src->mNumVertices; ++i)
	{
		auto position = &(src->mVertices[i]);												//���_���W
		auto normal = &(src->mNormals[i]);													//�@��
		auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;		//UV���W
		auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;	//����
		auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;		//���_�J���[

		//���]�I�v�V��������������UV���W�𔽓]������
		if (inverseU)
		{
			uv->x = 1 - uv->x;
		}

		if (inverseV)
		{
			uv->y = 1 - uv->y;
		}

		FBXVertex vertex = {};													//���_
		vertex.position = XMFLOAT3(position->x, position->y, position->z);		//���W
		vertex.normal = XMFLOAT3(normal->x, normal->y, normal->z);				//�@��
		vertex.uv = XMFLOAT2(uv->x, uv->y);										//UV���W
		vertex.tangent = XMFLOAT3(tangent->x, tangent->y, tangent->z);			//����?
		vertex.color = XMFLOAT4(color->r, color->g, color->b, color->a);		//���_�J���[

		dst.vertices[i] = vertex;	//�i�[
	}

	dst.indices.resize(src->mNumFaces * 3);	//�C���f�b�N�X�������b�V�����~3�ɒ���

	//�C���f�b�N�X���̊i�[
	for (auto i = 0u; i < src->mNumFaces; ++i)
	{
		const auto& face = src->mFaces[i];

		dst.indices[i * 3 + 0] = face.mIndices[0];
		dst.indices[i * 3 + 1] = face.mIndices[1];
		dst.indices[i * 3 + 2] = face.mIndices[2];
	}
}

/// <summary>
/// FBX�t�@�C������e�N�X�`����ǂݍ��ފ֐�
/// </summary>
/// <param name="filename">FBX�t�@�C����</param>
/// <param name="dst">���b�V���z��</param>
/// <param name="src">�}�e���A�����</param>
void
AssimpLoader::LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* src)
{
	aiString path;

	if (src->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		//�e�N�X�`���p�X�͑��΃p�X�œ����Ă���̂ŁA��΃p�X�ɒ������߃t�@�C���̏ꏊ�Ƃ����t����K�v������
		auto dir = GetDirectoryPath(filename);
		auto file = string(path.C_Str());

		size_t found1 = file.find("..");
		if (found1 != string::npos)
		{
			file.replace(found1, 2, "");
		}

		size_t found2 = file.find("\\");

		//�t�@�C���̃p�X��"\\"���܂܂�Ă����珈�������s
		if (found2 != string::npos)
		{
			file.replace(found2, 1, "/");
		}

		dst.diffuseMap = dir + ToWideString(file);	//�f�B�t���[�Y�̃p�X��ݒ�
	}
	else
	{
		dst.diffuseMap.clear();	//�f�B�t���[�Y�̃p�X�����Z�b�g
	}
}

