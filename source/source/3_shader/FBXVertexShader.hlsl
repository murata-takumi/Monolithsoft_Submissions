#include"FBXShaderHeader.hlsli"

/// <summary>
/// FBXモデルのデータの座標変換を行う関数
/// </summary>
/// <param name="pos">頂点座標</param>
/// <param name="normal">法線座標</param>
/// <param name="uv">UV座標</param>
/// <param name="tangent">正接</param>
/// <param name="color">頂点カラー</param>
/// <returns>座標変換済み構造体</returns>
Output FBXVS
(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	float3 tangent : TANGENT,
	float4 color : COLOR
)
{
	Output output;	//構造体を初期化

	float4 worldPos = mul(world, float4(pos));		//ワールド行列で変換
	float4 viewPos = mul(view, worldPos);			//ビュー行列で変換
	float4 projPos = mul(proj, viewPos);			//プロジェクション行列で変換

	output.svpos = projPos;								//システム用頂点座標
	output.pos = projPos;								//頂点座標
	output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);		//法線
	output.uv = uv;										//UV座標
	output.tangent = float3(0.0f, 0.0f, 0.0f);			//正接
	output.color = color;								//頂点カラー

	return output;
}