#include "UnityExportModel.h"

UnityExportModel::UnityExportModel()
{
	vs.Attach(g_DX11Manager.CreateVertexShader("Assets/Shaders/UnityExportModel.hlsl", "vsMain"));
	ps.Attach(g_DX11Manager.CreatePixelShader("Assets/Shaders/UnityExportModel.hlsl", "psMain"));

	//InputLayoutÇÃçÏê¨
	D3D11_INPUT_ELEMENT_DESC elem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "NORMAL"	,	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	24,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "COLOR"	,	0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	32,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
	};
	il.Attach(g_DX11Manager.CreateInputLayout(elem, 4, "Assets/Shaders/UnityExportModel.hlsl", "vsMain"));
}

void UnityExportModel::LoadAscii(string filename)
{
	uemData.LoadAscii(filename);

	//VertexBuffer IndexBufferçÏê¨
	for (auto& mesh : uemData.meshs)
	{
		ModelData tmpData;
		tmpData.vb.Attach(g_DX11Manager.CreateVertexBuffer(mesh.vertexDatas.data(), (UINT)mesh.vertexDatas.size()));
		tmpData.ib.Attach(g_DX11Manager.CreateIndexBuffer(mesh.indexs.data(), (UINT)mesh.indexs.size()));
		models.push_back(tmpData);
	}

	//TextureLoad
	for (auto& material : uemData.materials)
	{
		Material tmpMaterial;
		tmpMaterial.albedoTexture.Attach(g_DX11Manager.CreateTextureFromFile(material.GetTexture("_MainTex")));
		materials.push_back(tmpMaterial);
	}
}

void UnityExportModel::LoadBinary(string filename)
{
	uemData.LoadBinary(filename);

	//VertexBuffer IndexBufferçÏê¨
	for (auto& mesh : uemData.meshs)
	{
		ModelData tmpData;
		tmpData.vb.Attach(g_DX11Manager.CreateVertexBuffer(mesh.vertexDatas.data(), (UINT)mesh.vertexDatas.size()));
		tmpData.ib.Attach(g_DX11Manager.CreateIndexBuffer(mesh.indexs.data(), (UINT)mesh.indexs.size()));
		models.push_back(tmpData);
	}

	//TextureLoad
	for (auto& material : uemData.materials)
	{
		Material tmpMaterial;
		tmpMaterial.albedoTexture.Attach(g_DX11Manager.CreateTextureFromFile(material.GetTexture("_MainTex")));
		materials.push_back(tmpMaterial);
	}
}

void UnityExportModel::Draw()
{
	g_DX11Manager.SetVertexShader(vs.Get());
	g_DX11Manager.SetPixelShader(ps.Get());

	g_DX11Manager.SetInputLayout(il.Get());

	for (int i = 0; i < uemData.meshs.size();i++) {
		auto& model = uemData.meshs[i];
		g_DX11Manager.SetVertexBuffer(models[i].vb.Get(), sizeof(VertexData));
		g_DX11Manager.SetIndexBuffer(models[i].ib.Get());
		if (materials[model.materialNo].albedoTexture.Get() != nullptr)
			g_DX11Manager.SetTexture2D(0, materials[model.materialNo].albedoTexture.Get());

		//DrawCall
		g_DX11Manager.DrawIndexed(static_cast<UINT>(model.indexs.size()));
	}
}
