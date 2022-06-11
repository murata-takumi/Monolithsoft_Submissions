Texture2D<float4> tex : register(t0);		//テクスチャ
SamplerState smp : register(s0);			//0番スロットに設定されたサンプラー

//座標変換用定数スロット(使用しない)
cbuffer Parameters : register(b0)
{
    row_major float4x4 MatrixTransform;
};

//HPゲージ用スロット
cbuffer HitPoint : register(b1)
{
    float hp;
}