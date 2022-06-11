#pragma once
#include "Application.h"

/// <summary>
/// カメラ座標を管理するためのクラス
/// </summary>
class SphericalCoordinates
{
public:
	void SetRadius(float radius);			//半径を設定する関数
	void SetElevation(float elevation);		//仰角を設定する関数
	void SetAzimth(float azimth);			//方位角を設定する関数

	float GetRadius();					//半径を取得する関数
	float GetElevation();				//仰角を取得する関数

	SphericalCoordinates();									//コンストラクタ
	SphericalCoordinates(XMVECTOR cartasianCoordinate);		//コンストラクタ（カメラの初期座標設定）

	SphericalCoordinates Rotate(float newAzimuth, float newElevation);	//カメラを回転させる関数
	SphericalCoordinates TranslateRadius(float x);						//カメラを注視点に近付ける・遠ざける関数

	XMFLOAT3 ToCartesian();	//球面座標から通常座標に変換する関数

private:
	float _radius, _azimuth, _elevation;	//それぞれ半径・方位角・仰角
};
