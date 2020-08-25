#include "DirectX11Manager.h"
#include "UnityExportModel.h"
#include "UnityExportSkinnedModel.h"

ConstantBufferMatrix constantBuffer;
ConstantBuffer cb;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	if (FAILED(g_DX11Manager.Init(hInstance, nCmdShow)))
		return -1;

	//コンスタントバッファの作成
	g_DX11Manager.CreateConstantBuffer(sizeof(ConstantBufferMatrix), &cb);
	constantBuffer.proj = XMMatrixTranspose(
		XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f),
			1270.0f / 760.0f, 0.5f, 4096.0f * 8.0f));
	XMVECTOR eyePos = XMVectorSet(0, 10, -20.0f, 0);


	XMVECTOR targetPos = XMVectorSet(0, 1, 0, 0);
	XMVECTOR upVector = XMVectorSet(0, 1, 0, 0);
	constantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(eyePos, targetPos, upVector));

	UnityExportModel model;
	//model.LoadAscii("Assets/Models/MeshData.uma");
	model.LoadBinary("Assets/Models/MeshData.umb");

	UnityExportSkinnedModel skinnedModel;
	//skinnedModel.LoadAscii("Assets/Models/SkinnedMeshData.usa");
	skinnedModel.LoadBinary("Assets/Models/SkinnedMeshData.usb");

	uem::SkinnedAnimation animation;
	//animation.LoadAscii("Assets/Models/JUMP00anim.usaa", skinnedModel.uemData.root.get());
	animation.LoadBinary("Assets/Models/JUMP00anim.usab", skinnedModel.uemData.m_root.get());
	MSG msg = { 0 };
	while (true)
	{
		g_DX11Manager.input.InputUpdate();
		if (g_DX11Manager.input.Keyboard()->ChkKeyDown(DIK_W))
		{
			eyePos += constantBuffer.view.r[2];
		}
		if (g_DX11Manager.input.Keyboard()->ChkKeyDown(DIK_S))
		{
			eyePos -= constantBuffer.view.r[2];
		}
		if (g_DX11Manager.input.Keyboard()->ChkKeyDown(DIK_A))
		{
			eyePos -= constantBuffer.view.r[0];
		}
		if (g_DX11Manager.input.Keyboard()->ChkKeyDown(DIK_D))
		{
			eyePos += constantBuffer.view.r[0];
		}
		constantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(eyePos, eyePos + constantBuffer.view.r[2], upVector));

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (WM_QUIT == msg.message) break;

		constantBuffer.world = XMMatrixTranspose(XMMatrixIdentity());
		g_DX11Manager.UpdateConstantBuffer(cb.Get(), constantBuffer);

		//MainLoop
		g_DX11Manager.DrawBegin();

		static float animeTime = 0.0f;
		ImGui::SliderFloat("AnimTime", &animeTime, 0.0f, animation.GetMaxAnimationTime());
		animation.SetTransform(animeTime);

		ID3D11Buffer* tmpCb[] = { cb.Get() };
		g_DX11Manager.m_pImContext->VSSetConstantBuffers(0, 1, tmpCb);

		model.Draw();
		skinnedModel.Draw();

		g_DX11Manager.DrawEnd();
	}

	g_DX11Manager.Cleanup();
	return 0;
}