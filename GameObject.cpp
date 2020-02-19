#include "GameObject.h"
#include "d3dUtil.h"
using namespace DirectX;

GameObject::GameObject()
	: m_IndexCount(),
	m_Material(),
	m_VertexStride(),
	m_ShadowMaterial({}),
	m_WorldMatrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f)
{
}

XMFLOAT3 GameObject::GetPosFloat3() const
{
	return XMFLOAT3(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2));
}

XMVECTOR GameObject::GetPosXM()const
{
	return XMVectorSet(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2), 1.0);
}

void GameObject::SetPosition(const DirectX::XMVECTOR& v)
{
	Move(v - GetPosXM());
}

void GameObject::SetPosition(const DirectX::XMFLOAT3 & v)
{
	SetPosition(XMLoadFloat3(&v));
}

void GameObject::Move(const XMFLOAT3& v)
{
	SetWorldMatrix(GetWorldMATRIX() * DirectX::XMMatrixTranslation(v.x, v.y, v.z));
}
void GameObject::Move(const DirectX::XMVECTOR& v)
{
	DirectX::XMFLOAT3 moveDis;
	DirectX::XMStoreFloat3(&moveDis, v);
	Move(moveDis);
}
void GameObject::Move(float x, float y, float z)
{
	SetWorldMatrix(GetWorldMATRIX() * DirectX::XMMatrixTranslation(x, y, z));
}

void GameObject::SetTexture(ID3D11ShaderResourceView * texture)
{
	m_pTexture = texture;
}

void GameObject::SetMaterial(const Material & material)
{
	m_Material = material;
}

void GameObject::SetWorldMatrix(const XMFLOAT4X4 & world)
{
	m_WorldMatrix = world;
}

void GameObject::SetWorldMatrix(FXMMATRIX world)
{
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

void GameObject::Draw(ID3D11DeviceContext * deviceContext, BasicEffect& effect)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// 更新数据并应用
	effect.SetWorldMatrix(XMLoadFloat4x4(&m_WorldMatrix));
	effect.SetMaterial(m_Material);
	effect.SetTexture(m_pTexture.Get());
	effect.Apply(deviceContext);

	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameObject::DrawShadow(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// 更新数据并应用
	effect.SetWorldMatrix(XMLoadFloat4x4(&m_WorldMatrix));
	effect.SetMaterial(m_ShadowMaterial);
	effect.SetTexture(m_pTexture.Get());
	effect.Apply(deviceContext);

	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	std::string vbName = name + ".VertexBuffer";
	std::string ibName = name + ".IndexBuffer";
	std::string texName = name + ".Texture";
	m_pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(vbName.length()), vbName.c_str());
	m_pIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(ibName.length()), ibName.c_str());
	m_pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(texName.length()), texName.c_str());
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

