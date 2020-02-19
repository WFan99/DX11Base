#include "BillboardObject.h"
#include "d3dUtil.h"
void BillboardObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	
	
	// ���ö���/����������
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);

	// �������ݲ�Ӧ��
	effect.SetWorldMatrix(DirectX::XMMatrixIdentity());
	effect.SetMaterial(m_Material);
	effect.SetTexture(m_pTexture.Get());
	effect.Apply(deviceContext);

	deviceContext->Draw(m_VertexCount, 0);
}

void BillboardObject::DrawShadow(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{


	// ���ö���/����������
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);

	// �������ݲ�Ӧ��
	effect.SetWorldMatrix(DirectX::XMMatrixIdentity());
	effect.SetMaterial(m_ShadowMaterial);
	effect.SetTexture(m_pTexture.Get());
	effect.Apply(deviceContext);

	deviceContext->Draw(m_VertexCount, 0);
}

void BillboardObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	std::string vbName = name + ".VertexBuffer";
	std::string texName = name + ".Texture";
	m_pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(vbName.length()), vbName.c_str());
	m_pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(texName.length()), texName.c_str());
#else
	UNREFERENCED_PARAMETER(name);
#endif
}