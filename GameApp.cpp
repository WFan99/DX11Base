#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
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
	
	m_WoodCrate.SetPosition(XMVectorClamp(moveDis + m_WoodCrate.GetPosXM(), XMVectorSet(-9.0f, 0.0f, -9.0f, 0.0f), XMVectorSet(9.0F, 0.0f, 9.0f, 0.0f)));
	m_Camera.SetTarget(m_WoodCrate.GetPosFloat3());
	m_Camera.RotateX(mouseState.y * dt * 1.25f);
	m_Camera.RotateY(mouseState.x * dt * 1.25f);


	// 更新观察矩阵
	m_Camera.UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(m_Camera.GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera.GetPositionXM());

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

	m_WoodCrateMat = material;

	// 初始化木盒
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	generator.GenerateCube(vertices, indices);
	m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), vertices, indices);
	// 抬起高度避免深度缓冲区资源争夺
	m_WoodCrate.SetWorldMatrix(XMMatrixTranslation(0.0f, 0.01f, 5.0f));
	m_WoodCrate.SetTexture(texture.Get());
	m_WoodCrate.SetMaterial(material);
	
	// 初始化地板
	generator.GenerateSurface(vertices, indices, 20, 20, XMFLOAT4(0, 0, 5, 5));
	
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(),vertices,indices);
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);
	m_Floor.SetWorldMatrix(XMMatrixTranslation(0, -1, 0));

	// 初始化墙体
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	generator.GenerateSurface(vertices, indices, 20.0f, 20.0f, XMFLOAT4(0, 0, 5, 5));
	for (int i = 0; i < 4; ++i)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(), vertices, indices);
		m_Walls[i].SetWorldMatrix(XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(i * -XM_PIDIV2) *XMMatrixTranslation((i-2)%2 *10.0,9.0f,(1-i)%2*10.0));
	}
	


	// ******************
	// 初始化摄像机
	//

	m_Camera.SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	m_Camera.SetTarget(m_WoodCrate.GetPosFloat3());
	m_Camera.SetDistance(5.0f);
	m_Camera.SetDistanceMinMax(2.0f, 14.0f);
	
	m_BasicEffect.SetViewMatrix(m_Camera.GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera.GetPositionXM());

	m_Camera.SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_BasicEffect.SetProjMatrix(m_Camera.GetProjXM());

	// ******************
	// 初始化不会变化的值
	//

	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

	// 环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	// 灯光
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, -10.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 25.0f;
	m_BasicEffect.SetPointLight(0, pointLight);

	// ******************
	// 设置调试对象名
	//
	m_Floor.SetDebugObjectName("Floor");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	m_WoodCrate.SetDebugObjectName("WoodCrate");

	return true;
}



