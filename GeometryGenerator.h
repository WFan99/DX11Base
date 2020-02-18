#pragma once
#include "d3dApp.h"
#include <vector>
class GeometryGenerator
{
public:
	struct VertexDesc
	{
		VertexDesc(bool Color = false, bool Tangent = false, bool Normal = true, bool Tex = true, bool Pos = true) :
			color(Color), tangent(Tangent), tex(Tex),normal(Normal), pos(Pos){}

		bool color;
		bool tangent;
		bool normal;
		bool pos;
		bool tex;
	};
//��������巽��
public:
	
	GeometryGenerator(VertexDesc desc):vertexDesc(desc){}
	//����������
	template<typename VertexType>
	void GenerateCube(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float sideLength = 2);
	//��������,��������slicesƬ��ÿƬ��Ϊdivides��
	template<typename VertexType>
	void GenerateSphere(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float radius, UINT slices=20, UINT divides=20);
	//��������NDC��ƽ��
	template<typename VertexType>
	void GeneratePlane(std::vector<VertexType>& vertices, std::vector<WORD>& indices);
	//����һ����
	template<typename VertexType>
	void GenerateSurface(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float length = 20, float width = 20, const DirectX::XMFLOAT4& TexCoord = { 0.0,0.0,1.0,1.0 });
	//����Բ����
	template<typename VertexType>
	void GenerateCylinderSurface(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float radius = 1, float height = 2, UINT slices = 20);
	//���ö�������
	void SetVertexDesc(const VertexDesc& desc) { vertexDesc = desc; };
private:
	VertexDesc vertexDesc;
};

template<typename VertexType>
void GeometryGenerator::GenerateCube(std::vector<VertexType>& vertices, std::vector<WORD>& indices,float sideLength)
{
	using namespace DirectX;
	indices.resize(36);
	vertices.resize(24);
	//λ��
	if (vertexDesc.pos)
	{
		float p = sideLength / 2;
		//����
		vertices[0].pos = XMFLOAT3(+p, -p, -p);
		vertices[1].pos = XMFLOAT3(-p, -p, -p);
		vertices[2].pos = XMFLOAT3(-p, +p, -p);
		vertices[3].pos = XMFLOAT3(+p, +p, -p);
		//����
		vertices[4].pos = XMFLOAT3(-p, -p, +p);
		vertices[5].pos = XMFLOAT3(+p, -p, +p);
		vertices[6].pos = XMFLOAT3(+p, +p, +p);
		vertices[7].pos = XMFLOAT3(-p, +p, +p);
	
		//����
		vertices[8].pos = XMFLOAT3(-p, -p, -p);
		vertices[9].pos = XMFLOAT3(-p, -p, +p);
		vertices[10].pos = XMFLOAT3(-p, +p, +p);
		vertices[11].pos = XMFLOAT3(-p, +p, -p);

		//����
		vertices[12].pos = XMFLOAT3(+p, -p, p);
		vertices[13].pos = XMFLOAT3(+p, -p, -p);
		vertices[14].pos = XMFLOAT3(+p, +p, -p);
		vertices[15].pos = XMFLOAT3(+p, +p, +p);

		//����
		vertices[16].pos = XMFLOAT3(-p, -p, -p);
		vertices[17].pos = XMFLOAT3(+p, -p, -p);
		vertices[18].pos = XMFLOAT3(+p, -p, +p);
		vertices[19].pos = XMFLOAT3(-p, -p, +p);

		//����
		vertices[20].pos = XMFLOAT3(+p, +p, -p);
		vertices[21].pos = XMFLOAT3(-p, +p, -p);
		vertices[22].pos = XMFLOAT3(-p, +p, +p);
		vertices[23].pos = XMFLOAT3(+p, +p, +p);
	}
	//����
	if (vertexDesc.normal)
	{
		for (int i = 0; i < 4; i++)
			vertices[i].normal = XMFLOAT3(0, 0, -1);
		for (int i = 4; i < 8; i++)
			vertices[i].normal = XMFLOAT3(0, 0, 1);
		for (int i = 8; i < 12; i++)
			vertices[i].normal = XMFLOAT3(-1, 0, 0);
		for (int i = 12; i < 16; i++)
			vertices[i].normal = XMFLOAT3(1, 0, 0);
		for (int i = 16; i < 20; i++)
			vertices[i].normal = XMFLOAT3(0, -1, 0);
		for (int i = 20; i < 24; i++)
			vertices[i].normal = XMFLOAT3(0, 1, 0);
	}
	//����
	if (vertexDesc.tex)
	{
		for (int i = 0; i < 6; i++)
			vertices[i*4].tex = XMFLOAT2(1, 1);
		for (int i = 0; i < 6; i++)
			vertices[i*4+1].tex = XMFLOAT2(0, 1);
		for (int i = 0; i < 6; i++)
			vertices[i*4+2].tex = XMFLOAT2(0, 0);
		for (int i = 0; i < 6; i++)
			vertices[i*4+3].tex = XMFLOAT2(1, 0);
	}

	indices = {
		//����
		0,1,2,
		2,3,0,

		//����
		4,5,6,
		6,7,4,
		//����
		8,9,10,
		10,11,8,
		//����
		12,13,14,
		14,15,12,
		//����
		16,17,18,
		18,19,16,
		//����
		20,21,22,
		22,23,20
	};
}

template<typename VertexType>
void GeometryGenerator::GenerateSphere(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float radius, UINT slices, UINT divides)
{
	using namespace DirectX;
	float delTheta = XM_2PI / divides, delGama = XM_PI/ slices;
	
	UINT vertexCount = 2 + (slices - 1) * divides ;
	UINT indexCount = 6 * (slices-1) * divides;
	vertices.resize(vertexCount);
	indices.resize(indexCount);

	//����
	vertices[0].pos = XMFLOAT3{ 0,radius,0 };
	if (vertexDesc.normal)
		vertices[0].normal = XMFLOAT3(0, 1, 0);
	

	int index = 1;

	//���ϵ�
	for (float gama = delGama; gama < XM_PI; gama += delGama)
	{
		float r = radius * sin(gama);
		float h = radius * cos(gama);
		for (float theta = 0; theta < XM_2PI && fabs(theta - 2 * XM_PI)>1e-6; theta += delTheta)
		{
			vertices[index].pos = XMFLOAT3(r * cos(theta), h, r * sin(theta));
			if (vertexDesc.normal)
				vertices[index].normal = XMFLOAT3(cos(theta), cos(gama), sin(gama) * sin(theta));
			index++;
		}
	}
	//�����
	vertices[index].pos = XMFLOAT3(0, -radius, 0);
	if (vertexDesc.normal)
		vertices[index].normak = XMFLOAT3(0, -1, 0);

	index = 0;
	for (int i = 1; i <= divides; i++)
	{
		//����
		indices[index++] = 0;
		indices[index++] = i % divides + 1;
		indices[index++] = i;
		
	}
	for (int i = 0; i < slices - 2; i++)
	{
		for (int j = 0; j < divides; j++)
		{
			indices[index++] = i * divides + j + 1;
			indices[index++] = i * divides + (j + 1) % divides + 1;
			indices[index++] = i * divides + j + divides + 1;

			indices[index++] = i * divides + j + divides + 1;
			indices[index++] = i * divides + (j + 1) % divides + 1;
			indices[index++] = i * divides + (j + 1) % divides + divides + 1;

		}
	}
	for (int i = 1; i <= divides; i++)
	{
		//��
		int size = index-1;
		indices[index++]=size;
		indices[index++]=size - divides + i - 1;
		indices[index++]=size - divides + i % divides;
	}

}

template<typename VertexType>
void  GeometryGenerator::GeneratePlane(std::vector<VertexType>& vertices, std::vector<WORD>& indices)
{

}

template<typename VertexType>
void  GeometryGenerator::GenerateSurface(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float length, float width, const DirectX::XMFLOAT4& TexCoord)
{
	using namespace DirectX;
	vertices.resize(4);
	indices.resize(6);
	if (vertexDesc.pos)
	{
		vertices[0].pos = XMFLOAT3(-length / 2, 0, width / 2);
		vertices[1].pos = XMFLOAT3(length / 2, 0, width / 2);
		vertices[2].pos = XMFLOAT3(length / 2, 0, -width / 2);
		vertices[3].pos = XMFLOAT3(-length / 2, 0, -width / 2);
	}

	if (vertexDesc.normal)
	{
		for (int i = 0; i < 4; i++)
			vertices[i].normal = XMFLOAT3(0, 1, 0);
	}

	if (vertexDesc.tex)
	{
		vertices[0].tex = XMFLOAT2(TexCoord.x, TexCoord.y);
		vertices[1].tex = XMFLOAT2(TexCoord.z, TexCoord.y);
		vertices[2].tex = XMFLOAT2(TexCoord.z, TexCoord.w);
		vertices[3].tex = XMFLOAT2(TexCoord.x, TexCoord.w);
	}
	indices =
	{
		0,1,2,
		2,3,0
	};
}

template<typename VertexType>
void GeometryGenerator::GenerateCylinderSurface(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float radius, float height, UINT slices)
{
	using namespace DirectX;
	vertices.resize(2 * (slices + 1));
	indices.resize(6 * slices);
	int index = 0;
	float theta = XM_2PI / slices;
	for (UINT i = 0; i <= slices; i++)
	{
		float angle = i * theta;
		//�ϲ��
		vertices[index].pos = XMFLOAT3(radius * cos(angle), height / 2, radius * sin(angle));
		if (vertexDesc.normal)
			vertices[index].normal = XMFLOAT3(cos(i), 0, sin(i));
		if (vertexDesc.tex)
			vertices[index].tex = XMFLOAT2(static_cast<float>(i) / slices, 0);
		index++;
		//�²��
		vertices[index].pos = XMFLOAT3(radius * cos(angle), -height / 2, radius * sin(angle));
		if (vertexDesc.normal)
			vertices[index].normal = XMFLOAT3(cos(i), 0, sin(i));
		if (vertexDesc.tex)
			vertices[index].tex = XMFLOAT2(static_cast<float>(i) / slices, 1);
		index++;
	}
	index = 0;
	for (WORD i = 0; i < slices * 2; i += 2)
	{
		indices[index++] = i;
		indices[index++] = i + 3;
		indices[index++] = i + 1;
		
		indices[index++] = i;
		indices[index++] = i + 2;
		indices[index++] = i + 3;
	}

}