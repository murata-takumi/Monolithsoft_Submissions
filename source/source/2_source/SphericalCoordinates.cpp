#include "SphericalCoordinates.h"

const float PI = 3.141592653f;				//円周率

/// <summary>
/// コンストラクタ(引数が無い場合)
/// 特に処理はしない
/// </summary>
SphericalCoordinates::SphericalCoordinates()
{

}

/// <summary>
/// コンストラクタ(引数がある場合)
/// </summary>
/// <param name="cartasianCoordinate">注視点から視点へのベクトル</param>
SphericalCoordinates::SphericalCoordinates(XMVECTOR cartasianCoordinate)
{
	_radius = XMVector3Length(cartasianCoordinate).m128_f32[0];							//注視点から視点への距離
	_azimuth = atan2(cartasianCoordinate.m128_f32[3], cartasianCoordinate.m128_f32[1]);	//方位角
	_elevation = asin(cartasianCoordinate.m128_f32[2] / _radius);							//仰角
}

/// <summary>
/// 半径を設定する関数
/// </summary>
/// <param name="radius">設定する半径</param>
void
SphericalCoordinates::SetRadius(float radius)
{
	_radius = radius;
}

/// <summary>
/// 仰角を設定する関数
/// </summary>
/// <param name="elevation">設定する仰角</param>
void
SphericalCoordinates::SetElevation(float elevation)
{
	_elevation = elevation;
}

/// <summary>
/// 仰角を設定する関数
/// </summary>
/// <param name="elevation">設定する仰角</param>
void
SphericalCoordinates::SetAzimth(float azimth)
{
	_azimuth = azimth;
}

/// <summary>
/// 半径を取得する関数
/// </summary>
/// <returns>制限付き半径</returns>
float
SphericalCoordinates::GetRadius()
{
	return clamp(_radius, 60.0f, 310.0f);
}

/// <summary>
/// 仰角を取得する関数
/// </summary>
/// <returns>制限付き仰角</returns>
float
SphericalCoordinates::GetElevation()
{
	return clamp(_elevation, -45.0f * (PI / 180.0f), 75.0f * (PI / 180.0f));
}

/// <summary>
/// 方位角、仰角を調整する関数
/// </summary>
/// <param name="newAzimuth">方位角に加算する値</param>
/// <param name="newElevation">仰角に加算する値</param>
/// <returns>方位角、仰角が調整されたインスタンス</returns>
SphericalCoordinates
SphericalCoordinates::Rotate(float newAzimuth, float newElevation)
{
	//度数法から弧度法に変換
	newAzimuth = newAzimuth * (PI / 180.0f);
	newElevation = newElevation * (PI / 180.0f);

	//角度を加算
	_azimuth += newAzimuth;
	_elevation += newElevation;
	_elevation = GetElevation();

	return *this;
}

/// <summary>
/// カメラを注視点に近付ける・遠ざける関数
/// </summary>
/// <param name="x">注視点からの距離に加算する値</param>
/// <returns>距離を調整したインスタンス</returns>
SphericalCoordinates
SphericalCoordinates::TranslateRadius(float x)
{
	_radius += x;
	_radius = GetRadius();

	return *this;
}

/// <summary>
/// 球面座標から通常座標に変換する関数
/// </summary>
/// <returns>通常座標の値</returns>
XMFLOAT3
SphericalCoordinates::ToCartesian()
{
	float t = _radius * cos(GetElevation());												//注視点から視点への距離(XZ平面)
	return XMFLOAT3(t * sin(_azimuth), _radius * sin(GetElevation()), t * cos(_azimuth));	//方位角・仰角で演算して返す
}