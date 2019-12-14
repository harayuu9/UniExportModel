#pragma once
#include "DirectX11Manager.h"

class UnityExportSkinnedModel
{
	InputLayout il;
	VertexShader vs;
	PixelShader ps;

	ConstantBuffer boneMtxCb;
	XMMATRIX boneMtx[200];
public:
	struct VertexData
	{
		XMFLOAT3 position;
		XMFLOAT3 normal = XMFLOAT3(0, 0, 0);
		XMFLOAT2 uv = XMFLOAT2(0, 0);
		XMUINT4 boneIndex = XMUINT4(0, 0, 0, 0);
		XMFLOAT4 boneWeight = XMFLOAT4(0, 0, 0, 0);
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

	uem::SkinnedModel<VertexData> uemData;

	vector<ModelData> models;
	vector<Material> materials;

	UnityExportSkinnedModel();

	void LoadAscii(string filename);
	void LoadBinary(string filename);

	//void DrawImGui(std::shared_ptr<uem::Transform> trans);
	void Draw();
};