Texture2D<float4> tex : register(t0);		//通常テクスチャ
Texture2D<float4> effectTex : register(t1);	//法線テクスチャ
SamplerState smp : register(s0);			//サンプラー

cbuffer Factor : register(b0)				//画面のゆがみに影響を与える数値
{
	float dist;
	float fade;
};				

//出力用構造体
struct Output
{
	float4 svpos : SV_POSITION;		//頂点情報
	float2 uv : TEXCOORD;			//UV座標
};