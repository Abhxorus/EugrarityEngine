#include "RenderTargetView.h"
#include "Device.h"
#include "Texture.h"
#include "DeviceContext.h"
#include "DepthStencilView.h"

/**
 * @brief Inicializa una Render Target View (RTV) diseñada específicamente para el Back Buffer.
 * * Configura la vista utilizando por defecto dimensiones de textura con multisampleo (TEXTURE2DMS).
 * * @param device Referencia al dispositivo de hardware para crear el recurso.
 * @param backBuffer Referencia a la textura que servirá como buffer de salida.
 * @param Format Formato de datos de color (ej. DXGI_FORMAT_R8G8B8A8_UNORM).
 * @return HRESULT S_OK si la creación fue exitosa, o un código de error específico si fallan los punteros o el formato.
 */
HRESULT
RenderTargetView::init(Device& device, Texture& backBuffer, DXGI_FORMAT Format) {
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!backBuffer.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	// Configuración de la descripción para la vista de destino de renderizado
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	// Creación física de la RTV en la GPU
	HRESULT hr = device.m_device->CreateRenderTargetView(backBuffer.m_texture,
		&desc,
		&m_renderTargetView);
	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

/**
 * @brief Inicializa una Render Target View con parámetros de dimensión personalizados.
 * * Permite mayor flexibilidad para texturas que no son necesariamente el Back Buffer (ej. Render-to-Texture).
 * * @param device Referencia al dispositivo de hardware.
 * @param inTex Textura de entrada sobre la cual se creará la vista.
 * @param ViewDimension Dimensión del recurso (Texture2D, Texture2DMS, etc.).
 * @param Format Formato de color de la vista.
 * @return HRESULT Resultado de la operación de creación en DirectX.
 */
HRESULT
RenderTargetView::init(Device& device,
	Texture& inTex,
	D3D11_RTV_DIMENSION ViewDimension,
	DXGI_FORMAT Format) {
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!inTex.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = ViewDimension;

	HRESULT hr = device.m_device->CreateRenderTargetView(inTex.m_texture,
		&desc,
		&m_renderTargetView);

	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

/**
 * @brief Prepara el Render Target para el dibujado, limpiando el color y vinculando el Depth Stencil.
 * * Realiza un Clear de la superficie con el color especificado y establece la RTV y la DSV en
 * la etapa de Output Merger del pipeline.
 * * @param deviceContext Contexto del dispositivo para ejecutar comandos de limpieza y vinculación.
 * @param depthStencilView Vista de profundidad y stencil que se usará en conjunto.
 * @param numViews Cantidad de vistas a vincular (usualmente 1).
 * @param ClearColor Arreglo de 4 flotantes (RGBA) que define el color de limpieza.
 */
void
RenderTargetView::render(DeviceContext& deviceContext,
	DepthStencilView& depthStencilView,
	unsigned int numViews,
	const float ClearColor[4]) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}

	// Limpiar la vista con el color sólido proporcionado
	deviceContext.m_deviceContext->ClearRenderTargetView(m_renderTargetView, ClearColor);

	// Configurar el Output Merger con la RTV y la DSV correspondientes
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		depthStencilView.m_depthStencilView);
}

/**
 * @brief Vincula la Render Target View al pipeline sin realizar limpieza ni usar buffer de profundidad.
 * * Útil para pasadas de renderizado que no requieren pruebas de profundidad o donde la limpieza
 * se manejó externamente.
 * * @param deviceContext Contexto del dispositivo.
 * @param numViews Cantidad de vistas a vincular.
 */
void
RenderTargetView::render(DeviceContext& deviceContext, unsigned int numViews) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}

	// Configurar solo el Render Target en el Output Merger (Depth Stencil se establece a nullptr)
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		nullptr);
}

/**
 * @brief Libera la interfaz de la Render Target View de la memoria de video.
 * * Utiliza la macro SAFE_RELEASE para asegurar que el puntero COM se libere correctamente.
 */
void RenderTargetView::destroy() {
	SAFE_RELEASE(m_renderTargetView);
}