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
//创建结合体方法
public:
	
	GeometryGenerator(VertexDesc desc):vertexDesc(desc){}
	//创建正方体
	template<typename VertexType>
	void GenerateCube(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float sideLength = 2);
	//创建覆盖NDC的平面
	template<typename VertexType>
	void GeneratePlane(std::vector<VertexType>& vertices, std::vector<WORD>& indices);
	//创建一个面
	template<typename VertexType>
	void GenerateSurface(std::vector<VertexType>& vertices, std::vector<WORD>& indices, float length = 20, float width = 20, const DirectX::XMFLOAT4& TexCoord = { 0.0,0.0,1.0,1.0 });
	//设置顶点描述
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
	//位置
	if (vertexDesc.pos)
	{
		float p = sideLength / 2;
		//正面
		vertices[0].pos = XMFLOAT3(+p, -p, -p);
		vertices[1].pos = XMFLOAT3(-p, -p, -p);
		vertices[2].pos = XMFLOAT3(-p, +p, -p);
		vertices[3].pos = XMFLOAT3(+p, +p, -p);
		//后面
		vertices[4].pos = XMFLOAT3(-p, -p, +p);
		vertices[5].pos = XMFLOAT3(+p, -p, +p);
		vertices[6].pos = XMFLOAT3(+p, +p, +p);
		vertices[7].pos = XMFLOAT3(-p, +p, +p);
	
		//左面
		vertices[8].pos = XMFLOAT3(-p, -p, -p);
		vertices[9].pos = XMFLOAT3(-p, -p, +p);
		vertices[10].pos = XMFLOAT3(-p, +p, +p);
		vertices[11].pos = XMFLOAT3(-p, +p, -p);

		//右面
		vertices[12].pos = XMFLOAT3(+p, -p, p);
		vertices[13].pos = XMFLOAT3(+p, -p, -p);
		vertices[14].pos = XMFLOAT3(+p, +p, -p);
		vertices[15].pos = XMFLOAT3(+p, +p, +p);

		//下面
		vertices[16].pos = XMFLOAT3(-p, -p, -p);
		vertices[17].pos = XMFLOAT3(+p, -p, -p);
		vertices[18].pos = XMFLOAT3(+p, -p, +p);
		vertices[19].pos = XMFLOAT3(-p, -p, +p);

		//下面
		vertices[20].pos = XMFLOAT3(+p, +p, -p);
		vertices[21].pos = XMFLOAT3(-p, +p, -p);
		vertices[22].pos = XMFLOAT3(-p, +p, +p);
		vertices[23].pos = XMFLOAT3(+p, +p, +p);
	}
	//向量
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
	//纹理
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
		//正面
		0,1,2,
		2,3,0,

		//左面
		4,5,6,
		6,7,4,
		//后面
		8,9,10,
		10,11,8,
		//右面
		12,13,14,
		14,15,12,
		//上面
		16,17,18,
		18,19,16,
		//下面
		20,21,22,
		22,23,20
	};
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