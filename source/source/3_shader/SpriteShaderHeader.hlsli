Texture2D<float4> tex : register(t0);		//�e�N�X�`��
SamplerState smp : register(s0);			//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

//���W�ϊ��p�萔�X���b�g(�g�p���Ȃ�)
cbuffer Parameters : register(b0)
{
    row_major float4x4 MatrixTransform;
};

//HP�Q�[�W�p�X���b�g
cbuffer HitPoint : register(b1)
{
    float hp;
}