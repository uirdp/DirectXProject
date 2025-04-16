#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 up, float yaw, float pitch)
	: m_Front(0.0f, 0.0f, 1.0f), m_MovementSpeed(0.25f), m_MouseSensitivity(0.1f), m_Zoom(45.0f)
{
	m_Position = position;
	m_WorldUp = up;
	m_Yaw = yaw;
	m_Pitch = pitch;
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: m_Front(0.0f, 0.0f, -1.0f), m_MovementSpeed(2.5f), m_MouseSensitivity(0.1f), m_Zoom(45.0f)
{
	m_Position = DirectX::XMFLOAT3(posX, posY, posZ);
	m_WorldUp = DirectX::XMFLOAT3(upX, upY, upZ);
	m_Yaw = yaw;
	m_Pitch = pitch;
	updateCameraVectors();
}

DirectX::CXMMATRIX Camera::GetViewMatrix() const
{
	auto posVecotor = DirectX::XMLoadFloat3(&m_Position);
	auto frontVector = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Position), DirectX::XMLoadFloat3(&m_Front));
	auto upVector = DirectX::XMLoadFloat3(&m_Up);
    return DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&m_Position),
		DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Position),DirectX::XMLoadFloat3(&m_Front)),
		DirectX::XMLoadFloat3(&m_Up)
    );
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = m_MovementSpeed * deltaTime;
	if (direction == FORWARD)
		m_Position.z -= m_Front.z * velocity;
	if (direction == BACKWARD)
		m_Position.z += m_Front.z * velocity;
	if (direction == LEFT)
		m_Position.x += m_Right.x * velocity;
	if (direction == RIGHT)
		m_Position.x -= m_Right.x * velocity;

	const float oneOverSqrt2 = 0.707; // 0.707 = 1/ã2
	if (direction == FORWARD_LEFT)
	{
		m_Position.z -= m_Front.z * velocity * oneOverSqrt2;
		m_Position.x += m_Right.x * velocity * oneOverSqrt2;
	}

	if (direction == FORWARD_RIGHT)
	{
		m_Position.z -= m_Front.z * velocity * oneOverSqrt2;
		m_Position.x -= m_Right.x * velocity * oneOverSqrt2;
	}

	if (direction == BACKWARD_LEFT)
	{
		m_Position.z += m_Front.z * velocity * oneOverSqrt2;
		m_Position.x += m_Right.x * velocity * oneOverSqrt2;
	}

	if (direction == BACKWARD_RIGHT)
	{
		m_Position.z += m_Front.z * velocity * oneOverSqrt2;
		m_Position.x -= m_Right.x * velocity * oneOverSqrt2;
	}
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	xOffset *= m_MouseSensitivity;
	yOffset *= m_MouseSensitivity;
	m_Yaw += xOffset;
	m_Pitch += yOffset;
	if (constrainPitch)
	{
		if (m_Pitch > 89.0f)
			m_Pitch = 89.0f;
		if (m_Pitch < -89.0f)
			m_Pitch = -89.0f;
	}
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yOffset)
{
	if (m_Zoom >= 1.0f && m_Zoom <= 45.0f)
		m_Zoom -= yOffset;
	if (m_Zoom <= 1.0f)
		m_Zoom = 1.0f;
	if (m_Zoom >= 45.0f)
		m_Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
		DirectX::XMVECTOR front = DirectX::XMVectorSet(
		cos(DirectX::XMConvertToRadians(m_Yaw)) * cos(DirectX::XMConvertToRadians(m_Pitch)),
		sin(DirectX::XMConvertToRadians(m_Pitch)),
		sin(DirectX::XMConvertToRadians(m_Yaw)) * cos(DirectX::XMConvertToRadians(m_Pitch)),
		0.0f
	);
	m_Front = DirectX::XMFLOAT3(front.m128_f32[0], front.m128_f32[1], front.m128_f32[2]);

	DirectX::XMVECTOR worldUp = DirectX::XMLoadFloat3(&m_WorldUp);
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(front, worldUp);
	DirectX::XMVECTOR up = DirectX::XMVector3Cross(right, front);

	right = DirectX::XMVector3Normalize(right);
	DirectX::XMStoreFloat3(&m_Right, right);
	
	up = DirectX::XMVector3Normalize(up);
	DirectX::XMStoreFloat3(&m_Up, up);
}
