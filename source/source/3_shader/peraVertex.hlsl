#include"peraHeader.hlsli"

/// <summary>
/// �y���|���S�����s�N�Z���V�F�[�_�[�ɓn���֐�
/// </summary>
/// <param name="pos">���_���W</param>
/// <param name="uv">UV���W</param>
/// <returns>�f�[�^���i�[�����\����</returns>
Output vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
	Output output;			//�o�͗p�\���̂�錾
	output.svpos = pos;		//���_���W�����
	output.uv = uv;			//UV���W�����

	return output;			//�s�N�Z���V�F�[�_�[�ɓn��
}