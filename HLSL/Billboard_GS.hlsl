#include "Billboard.hlsli"

static const float2 g_TexCoord[4] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f)
};

[maxvertexcount(4)]
void GS(
	point VertexCenterSize gin[1],
	uint primId : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream
)
{
	//计算相对坐标系中的信息
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = g_EyePosW - gin[0].CenterW;
	look.y = 0.0f;
	look = normalize(look);
	float3 right = cross(up, look);
	
	//计算三角形顶点在世界坐标中的位置
	
	
	float4 v[4];
	float3 center = gin[0].CenterW;
	float halfWidth = 0.5f * gin[0].SizeW.x;
	float halfHeight = 0.5f * gin[0].SizeW.y;
	v[0] = float4(center + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(center + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(center - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(center - halfWidth * right + halfHeight * up, 1.0f);
	
	GeoOut gout;
	matrix viewProj = mul(g_View, g_Proj);
	
	 

	
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		
		// 若当前在绘制阴影，先进行投影操作
		[flatten]
		if (g_IsShadow)
		{
			v[i] = mul(v[i], g_Shadow);
		}
		gout.PosW = v[i].xyz;
		gout.PosH = mul(v[i], viewProj);
		gout.NormalW = look;
		gout.Tex = g_TexCoord[i];
		gout.PrimID = primId;
		triStream.Append(gout);

	}

}

	