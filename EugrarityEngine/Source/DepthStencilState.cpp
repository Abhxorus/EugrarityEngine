#include "DepthStencilState.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
DepthStencilState::init(Device& device, bool enableDepth, bool enableStencil) {
    if (!device.m_device) {
        ERROR("ShaderProgram", "init", "Device is null.");
        return E_POINTER;
    }

    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = enableDepth;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;

    desc.StencilEnable = enableStencil;
    desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    // Stencil operations for front-facing pixels
    desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations for back-facing pixels
    desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HRESULT hr = device.m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
    if (FAILED(hr)) {
        ERROR("DepthStencilState", "init", "Failed to create DepthStencilState");
    }
    return S_OK;
}

void
DepthStencilState::update() {

}

void
DepthStencilState::render(DeviceContext& deviceContext,
    unsigned int stencilRef,
    bool reset) {
    if (!deviceContext.m_deviceContext) {
        ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
        return;
    }
    if (!m_depthStencilState) {
        ERROR("DepthStencilState", "render", "DepthStencilState is nullptr");
        return;
    }

    if (!reset) {
        deviceContext.m_deviceContext->OMSetDepthStencilState(m_depthStencilState, stencilRef);
    }
    else {
        deviceContext.m_deviceContext->OMSetDepthStencilState(nullptr, stencilRef);
    }
}

void
DepthStencilState::destroy() {
    SAFE_RELEASE(m_depthStencilState);
}