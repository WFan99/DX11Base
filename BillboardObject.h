#pragma once
#include "GameObject.h"
class BillboardObject :public GameObject
{
public:
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect) override;
	 void DrawShadow(ID3D11DeviceContext* deviceContext, BasicEffect& effect) override;
	void SetDebugObjectName(const std::string& name) override;
	template<typename VertexType>
	void SetBuffer(ID3D11Device* device, const std::vector<VertexType>& vertices);
	
private:
	UINT m_VertexCount;

};


template<typename VertexType>
void BillboardObject::SetBuffer(ID3D11Device* device, const std::vector<VertexType>& vertices)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	// 检查D3D设备
	if (device == nullptr)
	return;

	// 设置顶点缓冲区描述
	m_VertexCount = vertices.size();
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)vertices.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices.data();
	device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf());
}


