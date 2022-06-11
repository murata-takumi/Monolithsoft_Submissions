#include"peraHeader.hlsli"

/// <summary>
/// ペラポリゴンをピクセルシェーダーに渡す関数
/// </summary>
/// <param name="pos">頂点座標</param>
/// <param name="uv">UV座標</param>
/// <returns>データを格納した構造体</returns>
Output vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
	Output output;			//出力用構造体を宣言
	output.svpos = pos;		//頂点座標を入力
	output.uv = uv;			//UV座標を入力

	return output;			//ピクセルシェーダーに渡す
}