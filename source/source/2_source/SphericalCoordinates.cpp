#include "SphericalCoordinates.h"

const float PI = 3.141592653f;				//�~����

/// <summary>
/// �R���X�g���N�^(�����������ꍇ)
/// ���ɏ����͂��Ȃ�
/// </summary>
SphericalCoordinates::SphericalCoordinates()
{

}

/// <summary>
/// �R���X�g���N�^(����������ꍇ)
/// </summary>
/// <param name="cartasianCoordinate">�����_���王�_�ւ̃x�N�g��</param>
SphericalCoordinates::SphericalCoordinates(XMVECTOR cartasianCoordinate)
{
	_radius = XMVector3Length(cartasianCoordinate).m128_f32[0];							//�����_���王�_�ւ̋���
	_azimuth = atan2(cartasianCoordinate.m128_f32[3], cartasianCoordinate.m128_f32[1]);	//���ʊp
	_elevation = asin(cartasianCoordinate.m128_f32[2] / _radius);							//�p
}

/// <summary>
/// ���a��ݒ肷��֐�
/// </summary>
/// <param name="radius">�ݒ肷�锼�a</param>
void
SphericalCoordinates::SetRadius(float radius)
{
	_radius = radius;
}

/// <summary>
/// �p��ݒ肷��֐�
/// </summary>
/// <param name="elevation">�ݒ肷��p</param>
void
SphericalCoordinates::SetElevation(float elevation)
{
	_elevation = elevation;
}

/// <summary>
/// �p��ݒ肷��֐�
/// </summary>
/// <param name="elevation">�ݒ肷��p</param>
void
SphericalCoordinates::SetAzimth(float azimth)
{
	_azimuth = azimth;
}

/// <summary>
/// ���a���擾����֐�
/// </summary>
/// <returns>�����t�����a</returns>
float
SphericalCoordinates::GetRadius()
{
	return clamp(_radius, 60.0f, 310.0f);
}

/// <summary>
/// �p���擾����֐�
/// </summary>
/// <returns>�����t���p</returns>
float
SphericalCoordinates::GetElevation()
{
	return clamp(_elevation, -45.0f * (PI / 180.0f), 75.0f * (PI / 180.0f));
}

/// <summary>
/// ���ʊp�A�p�𒲐�����֐�
/// </summary>
/// <param name="newAzimuth">���ʊp�ɉ��Z����l</param>
/// <param name="newElevation">�p�ɉ��Z����l</param>
/// <returns>���ʊp�A�p���������ꂽ�C���X�^���X</returns>
SphericalCoordinates
SphericalCoordinates::Rotate(float newAzimuth, float newElevation)
{
	//�x���@����ʓx�@�ɕϊ�
	newAzimuth = newAzimuth * (PI / 180.0f);
	newElevation = newElevation * (PI / 180.0f);

	//�p�x�����Z
	_azimuth += newAzimuth;
	_elevation += newElevation;
	_elevation = GetElevation();

	return *this;
}

/// <summary>
/// �J�����𒍎��_�ɋߕt����E��������֐�
/// </summary>
/// <param name="x">�����_����̋����ɉ��Z����l</param>
/// <returns>�����𒲐������C���X�^���X</returns>
SphericalCoordinates
SphericalCoordinates::TranslateRadius(float x)
{
	_radius += x;
	_radius = GetRadius();

	return *this;
}

/// <summary>
/// ���ʍ��W����ʏ���W�ɕϊ�����֐�
/// </summary>
/// <returns>�ʏ���W�̒l</returns>
XMFLOAT3
SphericalCoordinates::ToCartesian()
{
	float t = _radius * cos(GetElevation());												//�����_���王�_�ւ̋���(XZ����)
	return XMFLOAT3(t * sin(_azimuth), _radius * sin(GetElevation()), t * cos(_azimuth));	//���ʊp�E�p�ŉ��Z���ĕԂ�
}