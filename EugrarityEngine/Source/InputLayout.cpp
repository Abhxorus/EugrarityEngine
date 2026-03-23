#include "InputLayout.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa el objeto Input Layout para definir el formato de los vértices.
 * * Crea el recurso en la GPU vinculando la descripción del layout con la firma del Vertex Shader.
 * DirectX utiliza los datos del shader para validar que los elementos de entrada coincidan
 * con los parámetros esperados por el pipeline.
 * * @param device Referencia al dispositivo de hardware para la creación del recurso.
 * @param Layout Vector con los descriptores de los elementos de entrada (Semánticas, formatos, slots).
 * @param VertexShaderData Puntero al Blob que contiene el código compilado del Vertex Shader asociado.
 * @return HRESULT S_OK si la creación fue exitosa, E_INVALIDARG si el layout está vacío o E_POINTER si los datos del shader son nulos.
 */
HRESULT
InputLayout::init(Device& device,
	std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
	ID3DBlob* VertexShaderData) {
	if (Layout.empty()) {
		ERROR("InputLayout", "init", "Layout vector is empty.");
		return E_INVALIDARG;
	}
	if (!VertexShaderData) {
		ERROR("InputLayout", "init", "VertexShaderData is nullptr.");
		return E_POINTER;
	}

	// Creación del Input Layout vinculando los descriptores con el buffer del shader
	HRESULT hr = device.CreateInputLayout(Layout.data(),
		static_cast<unsigned int>(Layout.size()),
		VertexShaderData->GetBufferPointer(),
		VertexShaderData->GetBufferSize(),
		&m_inputLayout);

	if (FAILED(hr)) {
		ERROR("InputLayout", "init",
			("Failed to create InputLayout. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

/**
 * @brief Método de actualización para lógica interna.
 * * Actualmente se mantiene vacío; se reserva para posibles cambios dinámicos o recreación de layouts.
 */
void
InputLayout::update() {
	// Método vacío
}

/**
 * @brief Establece este Input Layout en la etapa de Input Assembler (IA) del pipeline.
 * * Informa a la GPU cómo debe interpretar los flujos de datos provenientes de los Vertex Buffers.
 * * @param deviceContext Contexto del dispositivo para aplicar el estado.
 */
void
InputLayout::render(DeviceContext& deviceContext) {
	if (!m_inputLayout) {
		ERROR("InputLayout", "render", "InputLayout is nullptr");
		return;
	}

	// Vinculación al Input Assembler
	deviceContext.m_deviceContext->IASetInputLayout(m_inputLayout);
}

/**
 * @brief Libera la interfaz del Input Layout de la memoria de video.
 * * Utiliza la macro SAFE_RELEASE para invalidar de forma segura el puntero de la interfaz COM.
 */
void
InputLayout::destroy() {
	SAFE_RELEASE(m_inputLayout);
}