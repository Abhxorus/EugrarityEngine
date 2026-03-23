#include "RasterizerState.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa el estado del rasterizador con la configuración por defecto del motor.
 * * Configura un modo de relleno sólido, culling de caras traseras (Back-face culling)
 * y habilita el clip de profundidad.
 * * @param device Objeto Device que contiene el dispositivo D3D11 real.
 * @return HRESULT S_OK si el estado se creó correctamente, código de error en caso contrario.
 */
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

/**
 * @brief Inicializa el estado del rasterizador con parámetros personalizados.
 * * Permite definir dinámicamente cómo se comportará la etapa de rasterización (ej. para renderizar wireframes).
 * * @param device Referencia al dispositivo de hardware.
 * @param fill Modo de relleno (D3D11_FILL_SOLID o D3D11_FILL_WIREFRAME).
 * @param cull Modo de descarte de caras (D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK).
 * @param frontCCW Define si las caras frontales siguen el orden antihorario (Counter-Clockwise).
 * @param depthClip Define si se habilita el recorte basado en la distancia de profundidad.
 * @return HRESULT Resultado de la creación del recurso en la GPU.
 */
HRESULT
RasterizerState::init(Device& device,
	D3D11_FILL_MODE fill,
	D3D11_CULL_MODE cull,
	bool frontCCW,
	bool depthClip) {
	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = fill;
	desc.CullMode = cull;
	desc.FrontCounterClockwise = frontCCW ? true : false;
	desc.DepthClipEnable = depthClip ? true : false;

	HRESULT hr = S_OK;
	hr = device.m_device->CreateRasterizerState(&desc, &m_rasterizerState);

	if (FAILED(hr)) {
		ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
	}
	return hr;
}

/**
 * @brief Método de actualización para lógica interna.
 * * Actualmente no realiza ninguna operación.
 */
void
RasterizerState::update() {
}

/**
 * @brief Establece este estado de rasterización en el pipeline de renderizado.
 * * Llama a RSSetState en el contexto del dispositivo para aplicar la configuración actual.
 * * @param deviceContext Contexto del dispositivo donde se aplicará el estado.
 */
void
RasterizerState::render(DeviceContext& deviceContext) {
	if (!m_rasterizerState)
	{
		ERROR("RasterizerState", "render", "RasterizerState is nullptr (init failed or not called)");
		return;
	}
	deviceContext.RSSetState(m_rasterizerState);
}

/**
 * @brief Libera el recurso de la GPU de forma segura.
 * * Utiliza la macro SAFE_RELEASE para invalidar el puntero de la interfaz COM.
 */
void
RasterizerState::destroy() {
	SAFE_RELEASE(m_rasterizerState);
}