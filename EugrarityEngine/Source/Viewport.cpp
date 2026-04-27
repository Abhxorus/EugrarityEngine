#include "Viewport.h"
#include "Window.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa el Viewport utilizando las dimensiones de una ventana activa.
 * * Configura el área de dibujado para que coincida exactamente con el tamaño del área cliente
 * de la ventana proporcionada, estableciendo el rango de profundidad estándar de 0 a 1.
 * * @param window Referencia a la ventana de la cual se extraerán el ancho y el alto.
 * @return HRESULT S_OK si la inicialización fue exitosa, E_POINTER si la ventana no es válida
 * o E_INVALIDARG si las dimensiones son cero.
 */
HRESULT
Viewport::init(const Window& window) {
	if (!window.m_hWnd) {
		ERROR("Viewport", "init", "Window handle (m_hWnd) is nullptr");
		return E_POINTER;
	}
	if (window.m_width == 0 || window.m_height == 0) {
		ERROR("Viewport", "init", "Window dimensions are zero.");
		return E_INVALIDARG;
	}

	// Configuración de la estructura D3D11_VIEWPORT
	m_viewport.Width = static_cast<float>(window.m_width);
	m_viewport.Height = static_cast<float>(window.m_height);
	m_viewport.MinDepth = 0.0f; // Plano cercano (Z)
	m_viewport.MaxDepth = 1.0f; // Plano lejano (Z)
	m_viewport.TopLeftX = 0;    // Origen X en la esquina superior izquierda
	m_viewport.TopLeftY = 0;    // Origen Y en la esquina superior izquierda

	return S_OK;
}

/**
 * @brief Inicializa el Viewport con dimensiones manuales.
 * * Útil para configuraciones de renderizado personalizadas, como minimapas o
 * técnicas de render-to-texture donde el área de salida no depende de la ventana principal.
 * * @param width Ancho deseado del Viewport en píxeles.
 * @param height Alto deseado del Viewport en píxeles.
 * @return HRESULT S_OK si las dimensiones son válidas.
 */
HRESULT
Viewport::init(unsigned int width, unsigned int height) {
	if (width == 0 || height == 0) {
		ERROR("Viewport", "init", "Window dimensions are zero.");
		return E_INVALIDARG;
	}

	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return S_OK;
}

/**
 * @brief Establece el Viewport en la etapa del Rasterizador del pipeline.
 * * Vincula la configuración de visualización al contexto del dispositivo para que
 * DirectX sepa cómo escalar las coordenadas proyectadas a los píxeles de salida.
 * * @param deviceContext Contexto del dispositivo para aplicar el comando RSSetViewports.
 */
void Viewport::render(DeviceContext& deviceContext) {
	if (!deviceContext.m_deviceContext) {
		ERROR("Viewport", "render", "Device context is not set.");
		return;
	}
	// Aplicar el Viewport al Rasterizer Stage (RS)
	deviceContext.RSSetViewports(1, &m_viewport);
}