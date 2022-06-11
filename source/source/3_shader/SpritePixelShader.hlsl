#include"SpriteShaderHeader.hlsli"

/// <summary>
/// DXTK12�ň����X�v���C�g�����H����֐�
/// HP�Q�[�W�̌�������������
/// </summary>
/// <param name="color">���_�J���[</param>
/// <param name="texCoord">UV���W/param>
/// <returns>�s�N�Z���̐F</returns>
float4 SpritePixelShader(float4 color : COLOR,
	float2 texCoord : TEXCOORD) : SV_TARGET
{
	//�Q�[�W���̔������擾
	//hp�ϐ����Q�[�W���ƂȂ�
	float size = hp * 0.5f;
	//size���傫���ꍇU���W��1-HP�����炷�AV���W�͂��̂܂�
	float2 coord = float2(saturate(texCoord.x + (1 - hp) * step(size, texCoord.x)), texCoord.y);
	//�e�N�X�`�����
	float4 col = color * tex.Sample(smp, coord);
	//0��1��Ԃ�
	float mask = abs(step(hp, texCoord.x) - 1);
	//��Z���ăs�N�Z���𓧖��ɂ���
	col *= mask;
	return col;
}