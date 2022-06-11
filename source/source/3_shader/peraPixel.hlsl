#include"peraHeader.hlsli"

/// <summary>
/// ペラポリゴン（FBXモデル等の描画結果をそのまま描画するポリゴン）を描画する関数
/// ゲーム画面を元に加工・描画を行う
/// </summary>
/// <param name="input">頂点シェーダーから渡された情報</param>
/// <returns>ピクセルの色</returns>
float4 ps(Output input) : SV_TARGET
{
	//法線マップ画像のテクスチャを取得
	float4 nmTex = effectTex.Sample(smp,input.uv);
	nmTex = nmTex * 2.0f - 1.0f;

	float4 res = tex.Sample(smp, input.uv + (nmTex.xy * 0.1f * dist));		//ペラポリゴンを描画し、その上から法線マップで画面の歪みを加算する
	res.rgb -= (res.rgb * fade);											//フェードイン／フェードアウトする

	return res;		//法線マップ画像に沿って画像をぼかす
}