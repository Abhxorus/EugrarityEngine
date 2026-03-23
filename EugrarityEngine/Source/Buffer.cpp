#include "Buffer.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa un buffer de datos geométricos (Vértices o Índices).
 * * Configura la descripción del buffer y los datos iniciales basándose en el bindFlag proporcionado.
 * * @param device Referencia al dispositivo de hardware.
 * @param mesh Estructura que contiene los vectores de vértices e índices.
 * @param bindFlag Bandera de DirectX que indica si es un buffer de vértices o índices.
 * @return HRESULT S_OK si el buffer se creó correctamente, código de error en caso de datos vacíos o fallo de creación.
 */
HRESULT
Buffer::init(Device& device, const MeshComponent& mesh, unsigned int bindFlag) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	if ((bindFlag & D3D11_BIND_VERTEX_BUFFER) && mesh.m_vertex.empty()) {
		ERROR("Buffer", "init", "Vertex buffer is empty");
		return E_INVALIDARG;
	}
	if ((bindFlag & D3D11_BIND_INDEX_BUFFER) && mesh.m_index.empty()) {
		ERROR("Buffer", "init", "Index buffer is empty");
		return E_INVALIDARG;
	}

	D3D11_BUFFER_DESC desc = {};
	D3D11_SUBRESOURCE_DATA data = {};

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	m_bindFlag = bindFlag;

	if (bindFlag & D3D11_BIND_VERTEX_BUFFER) {
		m_stride = sizeof(SimpleVertex);
		desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_vertex.size());
		desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
		data.pSysMem = mesh.m_vertex.data();
	}
	else if (bindFlag & D3D11_BIND_INDEX_BUFFER) {
		m_stride = sizeof(unsigned int);
		desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_index.size());
		desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
		data.pSysMem = mesh.m_index.data();
	}

	return createBuffer(device, desc, &data);
}

/**
 * @brief Inicializa un Constant Buffer (Buffer Constante).
 * * Crea un buffer vacío con el tamaño especificado, optimizado para ser actualizado
 * frecuentemente desde la CPU para enviar datos como matrices o colores a los shaders.
 * * @param device Referencia al dispositivo de hardware.
 * @param ByteWidth Tamaño total del buffer en bytes (debe ser múltiplo de 16).
 * @return HRESULT S_OK si la creación fue exitosa.
 */
HRESULT
Buffer::init(Device& device, unsigned int ByteWidth) {
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	if (ByteWidth == 0) {
		ERROR("Buffer", "init", "ByteWidth is zero");
		return E_INVALIDARG;
	}
	m_stride = ByteWidth;

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = ByteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_bindFlag = desc.BindFlags;

	return createBuffer(device, desc, nullptr);
}

/**
 * @brief Actualiza el contenido del buffer en la GPU.
 * * Utiliza UpdateSubresource para copiar datos desde la memoria del sistema a la memoria de video.
 * * @param deviceContext Contexto del dispositivo para ejecutar la copia.
 * @param pDstResource Recurso de destino (usualmente m_buffer).
 * @param DstSubresource Índice del subrecurso.
 * @param pDstBox Caja que define la región a actualizar (nullptr para todo el buffer).
 * @param pSrcData Puntero a los nuevos datos en la CPU.
 * @param SrcRowPitch Tamaño de una fila de datos.
 * @param SrcDepthPitch Tamaño de una profundidad de datos.
 */
void
Buffer::update(DeviceContext& deviceContext,
	ID3D11Resource* pDstResource,
	unsigned int DstSubresource,
	const D3D11_BOX* pDstBox,
	const void* pSrcData,
	unsigned int SrcRowPitch,
	unsigned int SrcDepthPitch) {
	if (!m_buffer) {
		ERROR("ShaderProgram", "update", "m_buffer is null.");
		return;
	}
	if (!pSrcData) {
		ERROR("ShaderProgram", "update", "pSrcData is null.");
		return;
	}
	deviceContext.m_deviceContext->UpdateSubresource(m_buffer,
		DstSubresource,
		pDstBox,
		pSrcData,
		SrcRowPitch,
		SrcDepthPitch);
}

/**
 * @brief Vincula el buffer al pipeline de renderizado según su tipo.
 * * Gestiona automáticamente si el buffer debe ir al Input Assembler (vértices/índices)
 * o a las etapas de Shaders (constantes).
 * * @param deviceContext Contexto del dispositivo.
 * @param StartSlot Slot de inicio para la vinculación.
 * @param NumBuffers Cantidad de buffers a vincular.
 * @param setPixelShader Si es true y es un Constant Buffer, también lo vincula al Pixel Shader.
 * @param format Formato de datos (principalmente para Index Buffers).
 */
void
Buffer::render(DeviceContext& deviceContext,
	unsigned int StartSlot,
	unsigned int NumBuffers,
	bool setPixelShader,
	DXGI_FORMAT format) {
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_buffer) {
		ERROR("Buffer", "render", "m_buffer is null.");
		return;
	}

	switch (m_bindFlag) {
	case D3D11_BIND_VERTEX_BUFFER:
		deviceContext.m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, &m_buffer, &m_stride, &m_offset);
		break;
	case D3D11_BIND_CONSTANT_BUFFER:
		deviceContext.m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		if (setPixelShader) {
			deviceContext.m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		}
		break;
	case D3D11_BIND_INDEX_BUFFER:
		deviceContext.m_deviceContext->IASetIndexBuffer(m_buffer, format, m_offset);
		break;
	default:
		ERROR("Buffer", "render", "Unsupported BindFlag");
		break;
	}
}

/**
 * @brief Libera el recurso del buffer de la memoria de video.
 */
void
Buffer::destroy() {
	SAFE_RELEASE(m_buffer);
}

/**
 * @brief Función interna para la creación física del buffer en DirectX 11.
 * * @param device Referencia al dispositivo.
 * @param desc Descripción técnica del buffer.
 * @param initData Puntero a los datos iniciales (opcional).
 * @return HRESULT Resultado de la creación.
 */
HRESULT
Buffer::createBuffer(Device& device,
	D3D11_BUFFER_DESC& desc,
	D3D11_SUBRESOURCE_DATA* initData) {
	if (!device.m_device) {
		ERROR("Buffer", "createBuffer", "Device is nullptr");
		return E_POINTER;
	}

	HRESULT hr = device.CreateBuffer(&desc, initData, &m_buffer);
	if (FAILED(hr)) {
		ERROR("Buffer", "createBuffer", "Failed to create buffer");
		return hr;
	}
	return S_OK;
}