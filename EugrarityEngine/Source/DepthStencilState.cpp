#include "DepthStencilState.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa un estado de profundidad y stencil personalizado.
 * * Configura cómo el pipeline de DirectX debe manejar las comparaciones de profundidad
 * y si debe escribir en el depth buffer. El stencil se desactiva por defecto en esta implementación.
 * * @param device Referencia al dispositivo de hardware para crear el recurso.
 * @param depthEnable Define si se activa la prueba de profundidad (Z-Test).
 * @param writeMask Define si se permite escribir en el buffer de profundidad (D3D11_DEPTH_WRITE_MASK_ALL o ZERO).
 * @param depthFunc Función de comparación (ej. D3D11_COMPARISON_LESS para renderizado estándar).
 * @return HRESULT S_OK si la creación fue exitosa, E_POINTER si el dispositivo es nulo.
 */
HRESULT
DepthStencilState::init(Device& device,
	bool depthEnable,
	D3D11_DEPTH_WRITE_MASK writeMask,
	D3D11_COMPARISON_FUNC depthFunc) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	D3D11_DEPTH_STENCIL_DESC desc{};
	desc.DepthEnable = depthEnable;
	desc.DepthWriteMask = writeMask;
	desc.DepthFunc = depthFunc;
	desc.StencilEnable = false; // Stencil desactivado por defecto para simplicidad

	// Creación física del estado en la GPU
	HRESULT hr = device.m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
	if (FAILED(hr)) {
		ERROR("DepthStencilState", "init", "Failed to create DepthStencilState");
		return hr;
	}
	return hr;
}

/**
 * @brief Método de actualización para lógica interna.
 * * Actualmente no realiza ninguna operación.
 */
void
DepthStencilState::update() {

}

/**
 * @brief Establece este estado de profundidad en el pipeline de renderizado.
 * * Vincula el objeto de estado al Output Merger (OM). Permite también resetear el estado
 * a los valores por defecto de DirectX si se activa el flag de reset.
 * * @param deviceContext Contexto del dispositivo para aplicar el cambio.
 * @param stencilRef Valor de referencia para las operaciones de stencil (si estuvieran activas).
 * @param reset Si es true, establece el estado de profundidad a nullptr (usando los valores por defecto).
 */
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
		// Aplicar el estado personalizado configurado en init
		deviceContext.m_deviceContext->OMSetDepthStencilState(m_depthStencilState, stencilRef);
	}
	else {
		// Restaurar al estado de profundidad por defecto de DirectX 11
		deviceContext.m_deviceContext->OMSetDepthStencilState(nullptr, stencilRef);
	}
}

/**
 * @brief Libera la interfaz del Depth Stencil State de la memoria de video.
 * * Utiliza la macro SAFE_RELEASE para invalidar el puntero de la interfaz COM.
 */
void
DepthStencilState::destroy() {
	SAFE_RELEASE(m_depthStencilState);
}