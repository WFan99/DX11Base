#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "ctime"
#include "GeometryGenerator.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_WoodCrateMat()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!InitResource())
		return false;

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	return true;
}

void GameApp::OnResize()
{
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);

	D3DApp::OnResize();
	
	// 摄像机变更显示

	m_Camera.SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_Camera.SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_BasicEffect.SetProjMatrix(m_Camera.GetProjXM());
}

void GameApp::UpdateScene(float dt)
{

	// 更新鼠标事件，获取相对偏移量
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	m_MouseTracker.Update(mouseState);

	Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);
	
	XMVECTOR moveDis = { 0,0,0,0 };
	if (keyState.IsKeyDown(Keyboard::W))
	{
		moveDis += dt * 3.0f * m_Camera.GetLookXM();
	}
	if (keyState.IsKeyDown(Keyboard::S))
	{
		moveDis -= dt * 3.0f * m_Camera.GetLookXM();

	}
	if (keyState.IsKeyDown(Keyboard::A))
	{
		moveDis -= dt * 3.0f * m_Camera.GetRightXM();
	}
	if (keyState.IsKeyDown(Keyboard::D))
	{
		moveDis += dt * 3.0f * m_Camera.GetRightXM();
	}
	
	m_WoodCrate.SetPosition(XMVectorClamp(moveDis + m_WoodCrate.GetPosXM(), XMVectorSet(-90.0f, 0.0f, -90.0f, 0.0f), XMVectorSet(9.0F, 0.0f, 9.0f, 0.0f)));
	m_Camera.SetTarget(m_WoodCrate.GetPosFloat3());
	m_Camera.RotateX(mouseState.y * dt * 1.25f);
	m_Camera.RotateY(mouseState.x * dt * 1.25f);
	m_Camera.Approach(-mouseState.scrollWheelValue / 120 * 1.0f);
	m_pMouse->ResetScrollWheelValue();

	// 更新观察矩阵
	m_Camera.UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(m_Camera.GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera.GetPositionXM());
	//设置灯光位置
	m_Light.SetPosition(XMVector3TransformCoord(m_Light.GetPosXM(),XMMatrixRotationAxis(XMVectorSet(0,1,0,0),0.01)));
	m_PointLight.position = m_Light.GetPosFloat3();
	m_BasicEffect.SetPointLight(0,m_PointLight);
	//设置闪电纹理和位置
	static float texTime = 0;
	static int index = 0;
	texTime += dt;
	if (texTime > 1.0f / 60)
	{
		texTime -= 1.0f / 60;
		m_BoltAnim.SetTexture(m_BoltTex[(index++)%60].Get());
	}
	m_BoltAnim.SetPosition(m_WoodCrate.GetPosFloat3());
	m_BoltAnim.Move(0, 1, 0);
	//白天黑夜转换
	static float realTime = 0;
	realTime += dt;
	float timeColor;

	if (realTime < 12)
		timeColor = realTime / 12;
	else if (realTime < 24)
		timeColor = (24 - realTime)/12;
	else
	{
		realTime -= 24;
		timeColor = 0.0f;
	}
	m_BasicEffect.SetFogPara(5, 400, XMFLOAT4(timeColor, timeColor, timeColor, 1.0));
		
	


	// 退出程序，这里应向窗口发送销毁信息
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
	for (auto& wall : m_Walls)
		wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Light.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	// 画闪电
	m_BasicEffect.SetRenderTransWithoutDepthWrite(m_pd3dImmediateContext.Get());
	m_BoltAnim.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	//画阴影
	m_BasicEffect.SetShadowState(true);
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0, 1, 0, 0.99), XMVectorSet(m_PointLight.position.x, m_PointLight.position.y, m_PointLight.position.z, 1.0)));	
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);
	m_WoodCrate.DrawShadow(m_pd3dImmediateContext.Get(), m_BasicEffect);
	//画树的阴影
	m_BasicEffect.SetRenderBillboardNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);
	m_Tree.DrawShadow(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_BasicEffect.SetShadowState(false);
	
	//画树
	m_BasicEffect.SetRenderBillboard(m_pd3dImmediateContext.Get());
	m_Tree.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	HR(m_pSwapChain->Present(0, 0));
}



bool GameApp::InitResource()
{
	
	// ******************
	// 初始化游戏对象

	//初始化所需要用到的变量
	//纹理资源
	ComPtr<ID3D11ShaderResourceView> texture;
	//材质
	Material material{};
	Material shadowMat{};
	//顶点定义
	GeometryGenerator::VertexDesc desc;
	//图形生成器
	GeometryGenerator generator(desc);
	//顶点数组
	std::vector<VertexPosNormalTex> vertices;
	//索引数组
	std::vector<WORD> indices;

	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	shadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shadowMat.diffuse= XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMat.specular= XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);


	// 初始化木盒
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	generator.GenerateCube(vertices, indices);
	m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), vertices, indices);
	// 抬起高度避免深度缓冲区资源争夺0f, 0.01f, 5.0f));
	m_WoodCrate.SetTexture(texture.Get());
	m_WoodCrate.SetWorldMatrix(XMMatrixTranslation(0.0f, 0.01f, 0.0f));
	m_WoodCrate.SetMaterial(material);
	m_WoodCrate.SetShadowMaterial(shadowMat);

	//初始化灯
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\ice.dds", nullptr, texture.GetAddressOf()));
	generator.GenerateCube(vertices, indices, 0.5);
	m_Light.SetBuffer(m_pd3dDevice.Get(), vertices, indices);
	m_Light.SetTexture(texture.Get());
	m_Light.SetMaterial(material);


	
	// 初始化地板
	generator.GenerateSurface(vertices, indices, 200, 200, XMFLOAT4(0, 0, 5, 5));
	
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(),vertices,indices);
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);
	m_Floor.SetWorldMatrix(XMMatrixTranslation(0, -1, 0));

	// 初始化墙体
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	generator.GenerateSurface(vertices, indices, 200.0f, 200.0f, XMFLOAT4(0, 0, 5, 5));
	for (int i = 0; i < 4; ++i)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(), vertices, indices);
		m_Walls[i].SetWorldMatrix(XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(i * -XM_PIDIV2) *XMMatrixTranslation((i-2)%2 *100.0,9.0f,(1-i)%2*100.0));
	}
	
	//初始化闪电
	for (int i = 0; i < 60; i++)
	{
		wchar_t fileName[50];
		wsprintf(fileName, L"Texture\\BoltAnim\\Bolt%03d.bmp", i+1);
		HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), fileName, nullptr, m_BoltTex[i].GetAddressOf()));
	}
	generator.GenerateCylinderSurface(vertices, indices, 3.0, 4.0, 20);
	m_BoltAnim.SetBuffer(m_pd3dDevice.Get(), vertices, indices);
	m_BoltAnim.SetTexture(m_BoltTex[0].Get());
	m_BoltAnim.SetMaterial(material);

	//初始化树
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\tree\\tree0.dds", nullptr, texture.GetAddressOf()));
	
	srand((unsigned)time(nullptr));
	std::vector <VertexPosSize>  treeVec(16);
	for (int i = 0; i < 16; ++i)
	{
		float theta = rand() / double(RAND_MAX)*XM_2PI;
		float radius = rand() % 70 + 20;
		treeVec[i].pos = XMFLOAT3(radius * cos(theta), 13, radius * sin(theta));
		treeVec[i].size = XMFLOAT2(30, 30);
	}
	std::vector<VertexPosSize> posV(treeVec);
	m_Tree.SetBuffer(m_pd3dDevice.Get(), posV);
	m_Tree.SetTexture(texture.Get());
	m_Tree.SetMaterial(material);
	m_Tree.SetShadowMaterial(shadowMat);

	// ******************
	// 初始化摄像机
	//

	m_Camera.SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_Camera.SetTarget(m_WoodCrate.GetPosFloat3());
	m_Camera.SetDistance(8.0f);
	m_Camera.SetDistanceMinMax(2.0f, 14.0f);
	
	m_BasicEffect.SetViewMatrix(m_Camera.GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera.GetPositionXM());

	m_Camera.SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_BasicEffect.SetProjMatrix(m_Camera.GetProjXM());

	m_BasicEffect.SetFogState(true);
	

	// ******************
	// 初始化不会变化的值
	//

	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

	// 环境光

	m_DirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, m_DirLight);
	// 灯光

	m_PointLight.position = XMFLOAT3(0.0f, 20.0f, -4.0f);
	m_PointLight.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_PointLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_PointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 200.0f;
	m_BasicEffect.SetPointLight(0, m_PointLight);

	//设置点光等位置
	m_Light.SetPosition(m_PointLight.position);

	// ******************
	// 设置调试对象名
	//
	m_Floor.SetDebugObjectName("Floor");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	m_WoodCrate.SetDebugObjectName("WoodCrate");
	m_BoltAnim.SetDebugObjectName("BoltAnim");
	m_Tree.SetDebugObjectName("Tree");

	return true;
}



