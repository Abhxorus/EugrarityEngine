#include "SamplerState.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa el estado del muestreador (Sampler State) con una configuración estándar.
 * * Configura el filtrado lineal para Minificación, Magnificación y Mipmaps (D3D11_FILTER_MIN_MAG_MIP_LINEAR)
 * y establece el modo de direccionamiento en WRAP para las coordenadas U, V y W, permitiendo que
 * las texturas se repitan infinitamente.
 * * @param device Referencia al dispositivo de hardware para crear el recurso.
 * @return HRESULT S_OK si la creación fue exitosa, E_POINTER si el dispositivo es nulo.
 */
HRESULT
SamplerState::init(Device& device) {
    if (!device.m_device) {
        ERROR("SamplerState", "init", "Device is nullptr");
        return E_POINTER;
    }

    // Configuración de la descripción del Sampler
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Filtrado lineal para suavizado
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // Repetir textura en eje U
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // Repetir textura en eje V
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // Repetir textura en eje W
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // No usar función de comparación
    sampDesc.MinLOD = 0;                               // Nivel mínimo de detalle (Mipmap)
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;               // Nivel máximo de detalle permitido

    // Creación física del Sampler State en la GPU
    HRESULT hr = device.CreateSamplerState(&sampDesc, &m_sampler);
    if (FAILED(hr)) {
        ERROR("SamplerState", "init", "Failed to create SamplerState");
        return hr;
    }

    return S_OK;
}

/**
 * @brief Método de actualización para lógica interna.
 * * Actualmente no realiza ninguna operación ya que los samplers suelen ser estáticos.
 */
void
SamplerState::update() {
    // No hay lógica de actualización para un sampler en este caso.
}

/**
 * @brief Vincula el Sampler State a la etapa del Pixel Shader.
 * * Establece los muestreadores en los slots especificados para que el shader pueda
 * procesar las texturas aplicadas a los objetos.
 * * @param deviceContext Contexto del dispositivo para aplicar el estado.
 * @param StartSlot Slot inicial de vinculación (ej. Slot 0 para texturas base o 10 para Skybox).
 * @param NumSamplers Cantidad de samplers a vincular.
 */
void
SamplerState::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumSamplers) {
    if (!m_sampler) {
        ERROR("SamplerState", "render", "SamplerState is nullptr");
        return;
    }

    // Vinculación al Pixel Shader (PS)
    deviceContext.PSSetSamplers(StartSlot, NumSamplers, &m_sampler);
}

/**
 * @brief Libera el recurso del Sampler State de la memoria de video.
 * * Utiliza la macro SAFE_RELEASE para invalidar el puntero de la interfaz COM.
 */
void
SamplerState::destroy() {
    if (m_sampler) {
        SAFE_RELEASE(m_sampler);
    }
}