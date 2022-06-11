#include"FBXShaderHeader.hlsli"

/// <summary>
/// FBX���f���̃f�[�^�̍��W�ϊ����s���֐�
/// </summary>
/// <param name="pos">���_���W</param>
/// <param name="normal">�@�����W</param>
/// <param name="uv">UV���W</param>
/// <param name="tangent">����</param>
/// <param name="color">���_�J���[</param>
/// <returns>���W�ϊ��ςݍ\����</returns>
Output FBXVS
(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	float3 tangent : TANGENT,
	float4 color : COLOR
)
{
	Output output;	//�\���̂�������

	float4 worldPos = mul(world, float4(pos));		//���[���h�s��ŕϊ�
	float4 viewPos = mul(view, worldPos);			//�r���[�s��ŕϊ�
	float4 projPos = mul(proj, viewPos);			//�v���W�F�N�V�����s��ŕϊ�

	output.svpos = projPos;								//�V�X�e���p���_���W
	output.pos = projPos;								//���_���W
	output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);		//�@��
	output.uv = uv;										//UV���W
	output.tangent = float3(0.0f, 0.0f, 0.0f);			//����
	output.color = color;								//���_�J���[

	return output;
}