#include"FBXShaderHeader.hlsli"

/// <summary>
/// FBXモデルのピクセルを描画する関数
/// </summary>
/// <param name="input">頂点シェーダーから渡された情報</param>
/// <returns>ピクセルの色</returns>
float4 FBXPS(Output input) : SV_TARGET
{
	return tex.Sample(smp,input.uv);
}