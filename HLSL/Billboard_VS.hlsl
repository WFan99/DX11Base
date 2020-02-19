#include "Billboard.hlsli"
VertexCenterSize VS(VertexPosSize vIn)
{
	VertexCenterSize vOut;
	vOut.CenterW = vIn.PosW;
	vOut.SizeW = vIn.SizeW;
	return vOut;
}