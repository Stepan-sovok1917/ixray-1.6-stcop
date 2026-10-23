#include "stdafx.h"

#include "dxRenderDeviceRender.h"

void CRenderTarget::PhaseGammaApply(IRHIRenderTargetView* Destination)
{
	// The editor presents into the window, which is larger than the viewport canvas,
	// so the quad has to cover the whole swap chain.
	float w = Destination ? float(Device.GetSwapchainWidth()) : float(Device.TargetWidth);
	float h = Destination ? float(Device.GetSwapchainHeight()) : float(Device.TargetHeight);

	auto color_grading = dxRenderDeviceRender::Instance().m_Gamma.cBalance;

	float brightness = dxRenderDeviceRender::Instance().m_Gamma.fBrightness;
	float contrast = dxRenderDeviceRender::Instance().m_Gamma.fContrast;
	float gamma = dxRenderDeviceRender::Instance().m_Gamma.fGamma;

	brightness = (brightness - contrast) * 0.25f;
	contrast = contrast * 0.5f + 0.5f;
	gamma = 1.0f / std::max(EPS_S, gamma);

	u_setrt(w, h, Destination ? Destination : RTarget, nullptr, nullptr, nullptr);

	DrawPassSQ(s_gamma, 0, [&]
	{
		GRHI->StateManager->SetCullMode(ERHI_CULLMODE::NONE);
		RImplementation.rmNormal();
		RCache.set_c("color_params", contrast, gamma, brightness, 0.0f);
		RCache.set_c("color_grading", color_grading.r, color_grading.g, color_grading.b, 0.0f);
	});
}