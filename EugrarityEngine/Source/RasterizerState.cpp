#include "RasterizerState.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
RasterizerState::init(Device device) {
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	HRESULT hr = S_OK;
	hr = device.m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

	if (FAILED(hr)) {
		ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
	}
	return hr;
}

HRESULT
RasterizerState::init(Device device, unsigned int FillMode, unsigned int CullMode) {
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = (D3D11_FILL_MODE)FillMode;
	rasterizerDesc.CullMode = (D3D11_CULL_MODE)CullMode;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	HRESULT hr = S_OK;
	hr = device.m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

	if (FAILED(hr)) {
		ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
	}
	return hr;
}

void
RasterizerState::update() {
}

void
RasterizerState::render(DeviceContext& deviceContext) {
	deviceContext.RSSetState(m_rasterizerState);
}

void
RasterizerState::destroy() {
	SAFE_RELEASE(m_rasterizerState);
}