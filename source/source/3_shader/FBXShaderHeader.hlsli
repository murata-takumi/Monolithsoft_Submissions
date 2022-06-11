Texture2D<float4> tex : register(t0);		//テクスチャ
SamplerState smp : register(s0);			//0番スロットに設定されたサンプラー

cbuffer SceneData : register(b0)		//ビュープロジェクション用スロット
{
	matrix view;						//ビュー行列
	matrix proj;						//プロジェクション行列
	float3 eye;							//カメラ座標
};

cbuffer Transform : register(b1)	//座標変換用スロット
{
	matrix world;					//ワールド変換行列
}

struct Output							//パイプラインステートから流れてくるデータを受け止める構造体
{
	float4 svpos : SV_POSITION;			//システム用座標
	float4 pos : POSITION;				//座標
	float4 normal : NORMAL;				//法線
	float2 uv : TEXCOORD;				//テクスチャのUV値
	float3 tangent : TANGENT;			//正接
	float4 color : COLOR;				//カラー
};