#pragma once
#include "Camera.h"

class Scene
{
public:
	bool Init();

	void Update();
	void Draw();
	bool CreateIrradianceMapResource();
	void RenderIrradianceMap();

	void ProcessMouseMovement(int xPos,  int yPos);

	void UpdateCamera(CameraMovement movement, float deltaTime);

private:
	Camera* m_pCamera;
	void ProcessInput();
};

extern Scene* g_Scene;
