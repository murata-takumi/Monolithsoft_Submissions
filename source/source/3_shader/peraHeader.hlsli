Texture2D<float4> tex : register(t0);		//�ʏ�e�N�X�`��
Texture2D<float4> effectTex : register(t1);	//�@���e�N�X�`��
SamplerState smp : register(s0);			//�T���v���[

cbuffer Factor : register(b0)				//��ʂ̂䂪�݂ɉe����^���鐔�l
{
	float dist;
	float fade;
};				

//�o�͗p�\����
struct Output
{
	float4 svpos : SV_POSITION;		//���_���
	float2 uv : TEXCOORD;			//UV���W
};