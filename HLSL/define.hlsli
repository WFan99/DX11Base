#include "LightHelper.hlsli"
Texture2D g_Tex : register(t0);
Texture2DArray g_TexArray : register(t1);
SamplerState g_Sam : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
	matrix g_World;
	matrix g_WorldInvTranspose;
	Material g_Material;
}

cbuffer CBDrawingStates : register(b1)
{
	int g_IsReflection;
	int g_IsShadow;
}

cbuffer CBChangesEveryFrame : register(b2)
{
	matrix g_View;
	float3 g_EyePosW;
	float pad;
	float4 gFogColor;
	int g_IsFog;
	float gFogStart;
	float gFogRange;
	
}

cbuffer CBChangesOnResize : register(b3)
{
	matrix g_Proj;
}

cbuffer CBChangesRarely : register(b4)
{
	matrix g_Reflection;
	matrix g_Shadow;
	matrix g_RefShadow;
	DirectionalLight g_DirLight[5];
	PointLight g_PointLight[5];
	SpotLight g_SpotLight[5];
}