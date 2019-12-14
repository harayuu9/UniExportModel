#pragma once
#include "DirectX11Manager.h"

class UnityExportModel
{
	InputLayout il;
	VertexShader vs;
	PixelShader ps;
public:
	struct VertexData
	{
		XMFLOAT3 position;
		XMFLOAT3 normal = XMFLOAT3(0, 0, 0);
		XMFLOAT2 uv = XMFLOAT2(0, 0);
	};

	struct Material
	{
		ShaderTexture albedoTexture;
	};

	struct ModelData
	{
		VertexBuffer vb;
		IndexBuffer ib;
	};

	uem::Model<VertexData> uemData;

	vector<ModelData> models;
	vector<Material> materials;

	UnityExportModel();

	void LoadAscii(string filename);
	void LoadBinary(string filename);

	void Draw();
};