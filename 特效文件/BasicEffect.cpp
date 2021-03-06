#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"
using namespace DirectX;


//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		Material material;
	};

	struct CBDrawingStates
	{
		int isReflection;
		int isShadow;
		DirectX::XMINT2 pad;
	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMVECTOR eyePos;
		DirectX::XMFLOAT4 gFogColor;
		int gFogOn;
		float gFogStart;
		float gFogRange;
		
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};


	struct CBChangesRarely
	{
		DirectX::XMMATRIX reflection;
		DirectX::XMMATRIX shadow;
		DirectX::XMMATRIX refShadow;
		DirectionalLight dirLight[BasicEffect::maxLights];
		PointLight pointLight[BasicEffect::maxLights];
		SpotLight spotLight[BasicEffect::maxLights];
	};

public:
	// 必须显式指定
	Impl() : m_IsDirty() {}
	~Impl() = default;

public:
	// 需要16字节对齐的优先放在前面
	CBufferObject<0, CBChangesEveryDrawing> m_CBDrawing;		// 每次对象绘制的常量缓冲区
	CBufferObject<1, CBDrawingStates>       m_CBStates;		    // 每次绘制状态变更的常量缓冲区
	CBufferObject<2, CBChangesEveryFrame>   m_CBFrame;		    // 每帧绘制的常量缓冲区
	CBufferObject<3, CBChangesOnResize>     m_CBOnResize;		// 每次窗口大小变更的常量缓冲区
	CBufferObject<4, CBChangesRarely>		m_CBRarely;		    // 几乎不会变更的常量缓冲区
	BOOL m_IsDirty;												// 是否有值变更
	std::vector<CBufferBase*> m_pCBuffers;					    // 统一管理上面所有的常量缓冲区


	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShader3D;				// 用于3D的像素着色器

	ComPtr<ID3D11VertexShader> m_pVertexShaderBillboard;		// 用于告示板的顶点着色器
	ComPtr<ID3D11GeometryShader>  m_pGeometryShaderBillboard;		// 用于告示板的几何着色器
	ComPtr<ID3D11PixelShader>  m_pPixelShaderBillboard;			// 用于告示板的像素着色器

	ComPtr<ID3D11InputLayout>  m_pVertexLayout3D;				// 用于3D的顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexPosSizeLayout;		// 用于告示板的输入布局

	ComPtr<ID3D11ShaderResourceView> m_pTexture;				// 用于绘制的纹理

};

//
// BasicEffect
//

namespace
{
	// BasicEffect单例
	static BasicEffect * g_pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
	if (g_pInstance)
		throw std::exception("BasicEffect is a singleton!");
	g_pInstance = this;
	pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
}

BasicEffect::BasicEffect(BasicEffect && moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

BasicEffect & BasicEffect::operator=(BasicEffect && moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

BasicEffect & BasicEffect::Get()
{
	if (!g_pInstance)
		throw std::exception("BasicEffect needs an instance!");
	return *g_pInstance;
}


bool BasicEffect::InitAll(ID3D11Device * device)
{
	if (!device)
		return false;

	if (!pImpl->m_pCBuffers.empty())
		return true;

	if (!RenderStates::IsInit())
		throw std::exception("RenderStates need to be initialized first!");

	ComPtr<ID3DBlob> blob;


	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pVertexShader3D.GetAddressOf()));
	
	// 创建顶点布局(3D)
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexLayout3D.GetAddressOf()));

	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPixelShader3D.GetAddressOf()));




	
	// 创建顶点着色器(Billboard)
	HR(CreateShaderFromFile(L"HLSL\\Billboard_VS.cso", L"HLSL\\Billboard_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pVertexShaderBillboard.GetAddressOf()));
	
	// 创建顶点布局(Billboard)
	
	HR(device->CreateInputLayout(VertexPosSize::inputLayout, ARRAYSIZE(VertexPosSize::inputLayout), blob->GetBufferPointer(),
		blob->GetBufferSize(), pImpl->m_pVertexPosSizeLayout.GetAddressOf()));
		

	// 创建像素着色器(Billboard)
	HR(CreateShaderFromFile(L"HLSL\\Billboard_GS.cso", L"HLSL\\Billboard_GS.hlsl", "GS", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pGeometryShaderBillboard.GetAddressOf()));

	// 创建像素着色器(Billboard)
	HR(CreateShaderFromFile(L"HLSL\\Billboard_PS.cso", L"HLSL\\Billboard_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPixelShaderBillboard.GetAddressOf()));


	pImpl->m_pCBuffers.assign({
		&pImpl->m_CBDrawing, 
		&pImpl->m_CBStates, 
		& pImpl->m_CBFrame,
		&pImpl->m_CBOnResize, 
		&pImpl->m_CBRarely});

	// 创建常量缓冲区
	for (auto& pBuffer : pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(pImpl->m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(pImpl->m_pVertexPosSizeLayout.Get(), "VertexPosSizeLayout");

	D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cBuffer.Get(), "CBDrawing");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[1]->cBuffer.Get(), "CBStates");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[2]->cBuffer.Get(), "CBFrame");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[3]->cBuffer.Get(), "CBOnResize");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[4]->cBuffer.Get(), "CBRarely");

	D3D11SetDebugObjectName(pImpl->m_pVertexShader3D.Get(), "Basic_VS_3D");
	D3D11SetDebugObjectName(pImpl->m_pPixelShader3D.Get(), "Basic_PS_3D");

	D3D11SetDebugObjectName(pImpl->m_pVertexShaderBillboard.Get(), "Billboard_VS");
	D3D11SetDebugObjectName(pImpl->m_pGeometryShaderBillboard.Get(), "Billboard_GS");
	D3D11SetDebugObjectName(pImpl->m_pPixelShaderBillboard.Get(), "Billboard_PS");


	return true;
}

void BasicEffect::SetRenderDefault(ID3D11DeviceContext * deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetRenderTransWithoutDepthWrite(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexLayout3D.Get());
	deviceContext->VSSetShader(pImpl->m_pVertexShader3D.Get(), nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->m_pPixelShader3D.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(),0);
	deviceContext->OMSetBlendState(RenderStates::BSAddictive.Get(), nullptr, 0xFFFFFFFF);
}
void BasicEffect::SetRenderBillboard(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexPosSizeLayout.Get());
	deviceContext->VSSetShader(pImpl->m_pVertexShaderBillboard.Get(), nullptr, 0);
	deviceContext->GSSetShader(pImpl->m_pGeometryShaderBillboard.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->m_pPixelShaderBillboard.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderBillboardNoDoubleBlend(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexPosSizeLayout.Get());
	deviceContext->VSSetShader(pImpl->m_pVertexShaderBillboard.Get(), nullptr, 0);
	deviceContext->GSSetShader(pImpl->m_pGeometryShaderBillboard.Get(), nullptr, 0);
	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
	deviceContext->PSSetShader(pImpl->m_pPixelShaderBillboard.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), stencilRef);
	deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}


void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	auto& cBuffer = pImpl->m_CBDrawing;
	cBuffer.data.world = XMMatrixTranspose(W);
	cBuffer.data.worldInvTranspose = XMMatrixInverse(nullptr, W);	// 两次转置抵消
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetViewMatrix(FXMMATRIX V)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.view = XMMatrixTranspose(V);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetProjMatrix(FXMMATRIX P)
{
	auto& cBuffer = pImpl->m_CBOnResize;
	cBuffer.data.proj = XMMatrixTranspose(P);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetReflectionMatrix(FXMMATRIX R)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.reflection = XMMatrixTranspose(R);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetShadowMatrix(FXMMATRIX S)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.shadow = XMMatrixTranspose(S);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetRefShadowMatrix(DirectX::FXMMATRIX RefS)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.refShadow = XMMatrixTranspose(RefS);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetDirLight(size_t pos, const DirectionalLight & dirLight)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.dirLight[pos] = dirLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetPointLight(size_t pos, const PointLight & pointLight)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.pointLight[pos] = pointLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetSpotLight(size_t pos, const SpotLight & spotLight)
{
	auto& cBuffer = pImpl->m_CBRarely;
	cBuffer.data.spotLight[pos] = spotLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetMaterial(const Material & material)
{
	auto& cBuffer = pImpl->m_CBDrawing;
	cBuffer.data.material = material;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetTexture(ID3D11ShaderResourceView * texture)
{
	pImpl->m_pTexture = texture;
}

void XM_CALLCONV BasicEffect::SetEyePos(FXMVECTOR eyePos)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.eyePos = eyePos;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetReflectionState(bool isOn)
{
	auto& cBuffer = pImpl->m_CBStates;
	cBuffer.data.isReflection = isOn;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetShadowState(bool isOn)
{
	auto& cBuffer = pImpl->m_CBStates;
	cBuffer.data.isShadow = isOn;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetFogState(bool isOn)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.gFogOn = isOn?1:0;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}


void XM_CALLCONV BasicEffect::SetFogPara(float start, float range, DirectX::XMFLOAT4 color)
{
	auto& cBuffer = pImpl->m_CBFrame;
	cBuffer.data.gFogStart = start;
	cBuffer.data.gFogRange = range;
	cBuffer.data.gFogColor = color;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext * deviceContext)
{
	auto& pCBuffers = pImpl->m_pCBuffers;
	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindVS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);
	pCBuffers[3]->BindVS(deviceContext);
	pCBuffers[4]->BindVS(deviceContext);

	pCBuffers[0]->BindPS(deviceContext);
	pCBuffers[1]->BindPS(deviceContext);
	pCBuffers[2]->BindPS(deviceContext);
	pCBuffers[4]->BindPS(deviceContext);
	
	pCBuffers[1]->BindGS(deviceContext);
	pCBuffers[2]->BindGS(deviceContext);
	pCBuffers[3]->BindGS(deviceContext);
	pCBuffers[4]->BindGS(deviceContext);
	// 设置纹理
	deviceContext->PSSetShaderResources(0, 1, pImpl->m_pTexture.GetAddressOf());

	if (pImpl->m_IsDirty)
	{
		pImpl->m_IsDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}




