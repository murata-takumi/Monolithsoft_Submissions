#include"SpriteShaderHeader.hlsli"

/// <summary>
/// DXTK12で扱うスプライトを加工する関数
/// HPゲージの減少を実装する
/// </summary>
/// <param name="color">頂点カラー</param>
/// <param name="texCoord">UV座標/param>
/// <returns>ピクセルの色</returns>
float4 SpritePixelShader(float4 color : COLOR,
	float2 texCoord : TEXCOORD) : SV_TARGET
{
	//ゲージ幅の半分を取得
	//hp変数がゲージ幅となる
	float size = hp * 0.5f;
	//sizeより大きい場合U座標を1-HP分ずらす、V座標はそのまま
	float2 coord = float2(saturate(texCoord.x + (1 - hp) * step(size, texCoord.x)), texCoord.y);
	//テクスチャ情報
	float4 col = color * tex.Sample(smp, coord);
	//0か1を返す
	float mask = abs(step(hp, texCoord.x) - 1);
	//乗算してピクセルを透明にする
	col *= mask;
	return col;
}