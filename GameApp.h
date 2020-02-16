#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Camera.h"
#include "GameObject.h"


class GameApp : public D3DApp
{
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

private:
	

	GameObject m_WoodCrate;									    // 木盒
	GameObject m_Floor;										    // 地板
	std::vector<GameObject> m_Walls;							// 墙壁

	Material m_WoodCrateMat;									// 木盒材质

	BasicEffect m_BasicEffect;								    // 对象渲染特效管理

	ThirdPersonCamera m_Camera;						    // 摄像机

};


#endif