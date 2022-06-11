#include"peraHeader.hlsli"

/// <summary>
/// �y���|���S���iFBX���f�����̕`�挋�ʂ����̂܂ܕ`�悷��|���S���j��`�悷��֐�
/// �Q�[����ʂ����ɉ��H�E�`����s��
/// </summary>
/// <param name="input">���_�V�F�[�_�[����n���ꂽ���</param>
/// <returns>�s�N�Z���̐F</returns>
float4 ps(Output input) : SV_TARGET
{
	//�@���}�b�v�摜�̃e�N�X�`�����擾
	float4 nmTex = effectTex.Sample(smp,input.uv);
	nmTex = nmTex * 2.0f - 1.0f;

	float4 res = tex.Sample(smp, input.uv + (nmTex.xy * 0.1f * dist));		//�y���|���S����`�悵�A���̏ォ��@���}�b�v�ŉ�ʂ̘c�݂����Z����
	res.rgb -= (res.rgb * fade);											//�t�F�[�h�C���^�t�F�[�h�A�E�g����

	return res;		//�@���}�b�v�摜�ɉ����ĉ摜���ڂ���
}