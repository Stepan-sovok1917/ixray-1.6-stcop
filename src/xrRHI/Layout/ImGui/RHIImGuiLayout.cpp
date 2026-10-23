#include "../../RHI.h"
#include <imgui.h>

#include <d3d11.h>

#include "imgui_impl_dx11.h"

ImGui_ImplDX11_Data* ImGui_ImplDX11_GetBackendData();

#define DX11Device ((ID3D11Device*)GRHI->DevicePtr->RawDevice)
#define DX11Context ((ID3D11DeviceContext*)GRHI->GetContext())

RHI_API void RHIUtils::ImGui::Init()
{
	switch (GRHI->APILevel)
	{
		case ERHI_API_LAYER::D3D11: ImGui_ImplDX11_Init(DX11Device, DX11Context);  break;
	}
}

RHI_API void RHIUtils::ImGui::NewFrame()
{
	switch (GRHI->APILevel)
	{
		case ERHI_API_LAYER::D3D11: ImGui_ImplDX11_NewFrame();  break;
	}
}

RHI_API void RHIUtils::ImGui::DrawData()
{
	switch (GRHI->APILevel)
	{
		case ERHI_API_LAYER::D3D11: ImGui_ImplDX11_RenderDrawData(::ImGui::GetDrawData());  break;
	}
}

RHI_API void RHIUtils::ImGui::Destroy()
{
	switch (GRHI->APILevel)
	{
		case ERHI_API_LAYER::D3D11: ImGui_ImplDX11_Shutdown();  break;
	}
}

RHI_API void RHIUtils::ImGui::Reset()
{
}

RHI_API void* RHIUtils::ImGui::GetBlenderState()
{
#ifdef IXR_WINDOWS
	if (GRHI->APILevel == ERHI_API_LAYER::D3D11)
	{
		if (auto State = ImGui_ImplDX11_GetBackendData())
		{
			return State->pBlendState;
		}
	}
#endif

	return nullptr;
}