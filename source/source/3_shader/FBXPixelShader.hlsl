#include"FBXShaderHeader.hlsli"

/// <summary>
/// FBX���f���̃s�N�Z����`�悷��֐�
/// </summary>
/// <param name="input">���_�V�F�[�_�[����n���ꂽ���</param>
/// <returns>�s�N�Z���̐F</returns>
float4 FBXPS(Output input) : SV_TARGET
{
	return tex.Sample(smp,input.uv);
}