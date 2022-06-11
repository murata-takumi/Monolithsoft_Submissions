Texture2D<float4> tex : register(t0);		//�e�N�X�`��
SamplerState smp : register(s0);			//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

cbuffer SceneData : register(b0)		//�r���[�v���W�F�N�V�����p�X���b�g
{
	matrix view;						//�r���[�s��
	matrix proj;						//�v���W�F�N�V�����s��
	float3 eye;							//�J�������W
};

cbuffer Transform : register(b1)	//���W�ϊ��p�X���b�g
{
	matrix world;					//���[���h�ϊ��s��
}

struct Output							//�p�C�v���C���X�e�[�g���痬��Ă���f�[�^���󂯎~�߂�\����
{
	float4 svpos : SV_POSITION;			//�V�X�e���p���W
	float4 pos : POSITION;				//���W
	float4 normal : NORMAL;				//�@��
	float2 uv : TEXCOORD;				//�e�N�X�`����UV�l
	float3 tangent : TANGENT;			//����
	float4 color : COLOR;				//�J���[
};