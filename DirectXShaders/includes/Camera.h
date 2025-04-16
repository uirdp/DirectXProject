#pragma once

#include <DirectXMath.h>

enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	FORWARD_LEFT,
	FORWARD_RIGHT,
	BACKWARD_LEFT,
	BACKWARD_RIGHT
};

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 up, float yaw, float pitch);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	DirectX::CXMMATRIX GetViewMatrix() const;
	float GetZoom() const { return m_Zoom; }
	void ProcessKeyboard(CameraMovement direction, float deltaTime);
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yOffset);

private:
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Up;
	DirectX::XMFLOAT3 m_Front;
	DirectX::XMFLOAT3 m_Right;
	DirectX::XMFLOAT3 m_WorldUp;
	float m_Yaw;
	float m_Pitch;
	float m_MovementSpeed;
	float m_MouseSensitivity;
	float m_Zoom;

	void updateCameraVectors();
};

