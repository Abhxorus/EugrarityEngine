#include "DepthStencilView.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"

/**
 * @brief Inicializa la vista de profundidad y stencil (Depth Stencil View).
 * * Configura la vista para ser utilizada con texturas que soportan multisampleo (TEXTURE2DMS),
 * permitiendo al hardware realizar pruebas de visibilidad y descarte de píxeles.
 * * @param device Referencia al dispositivo de hardware para crear el recurso.
 * @param depthStencil Referencia a la textura que contiene el buffer de profundidad.
 * @param format Formato DXGI para la profundidad (ej. DXGI_FORMAT_D24_UNORM_S8_UINT).
 * @return HRESULT S_OK si la creación fue exitosa, E_FAIL si la textura es nula.
 */
HRESULT
DepthStencilView::init(Device& device, Texture& depthStencil, DXGI_FORMAT format) {
	if (!device.m_device) {
		ERROR("DepthStencilView", "init", "Device is null.");
	}
	if (!depthStencil.m_texture) {
		ERROR("DepthStencilView", "init", "Texture is null.");
		return E_FAIL;
	}

	// Configuración de la descripción para la vista de profundidad/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	memset(&descDSV, 0, sizeof(descDSV));
	descDSV.Format = format;
	// Se asume el uso de multisampleo para coincidir con el Render Target estándar
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDSV.Texture2D.MipSlice = 0;

	// Creación física de la DSV en la GPU
	HRESULT hr = device.m_device->CreateDepthStencilView(depthStencil.m_texture,
		&descDSV,
		&m_depthStencilView);

	if (FAILED(hr)) {
		ERROR("DepthStencilView", "init",
			("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

/**
 * @brief Realiza la limpieza del buffer de profundidad y stencil.
 * * Establece el valor de profundidad a 1.0f (el punto más alejado) y el stencil a 0.
 * Este paso es crucial al inicio de cada frame para evitar que restos del frame anterior
 * afecten las pruebas de visibilidad actuales.
 * * @param deviceContext Contexto del dispositivo para ejecutar el comando de limpieza.
 */
void
DepthStencilView::render(DeviceContext& deviceContext) {
	if (!deviceContext.m_deviceContext) {
		ERROR("DepthStencilView", "render", "Device context is null.");
		return;
	}

	// Limpia tanto el canal de profundidad como el de stencil simultáneamente
	deviceContext.m_deviceContext->ClearDepthStencilView(m_depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, // Profundidad máxima
		0);    // Valor de stencil inicial
}

/**
 * @brief Libera la interfaz de la Depth Stencil View de la memoria de video.
 * * Utiliza la macro SAFE_RELEASE para invalidar el puntero de la interfaz COM.
 */
void
DepthStencilView::destroy() {
	SAFE_RELEASE(m_depthStencilView);
}