#include "ECS/Actor.h"
#include "MeshComponent.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Constructor de la clase Actor.
 * * Inicializa los componentes por defecto (Transform y MeshComponent), configura el buffer constante
 * para la matriz de transformación del modelo y prepara el estado del sampler.
 * * @param device Referencia al dispositivo de DirectX para la creación de recursos.
 */
Actor::Actor(Device& device) {
	// Setup Default Components
	EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
	addComponent(transform);
	EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
	addComponent(meshComponent);

	HRESULT hr;
	std::string classNameType = "Actor -> " + m_name;

	// Inicialización del Constant Buffer para cambios por frame (Matrices de mundo)
	hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
	if (FAILED(hr)) {
		ERROR("Actor", classNameType.c_str(), "Failed to create new CBChangesEveryFrame");
	}

	// Llamada al despertar de la entidad (lógica inicial)
	awake();

	// Inicialización del estado del sampler para el filtrado de texturas
	hr = m_sampler.init(device);
	if (FAILED(hr)) {
		ERROR("Actor", classNameType.c_str(), "Failed to create new SamplerState");
	}
}

/**
 * @brief Actualiza la lógica del actor y sus componentes.
 * * Itera sobre todos los componentes adjuntos para ejecutar su update y actualiza
 * el buffer constante del modelo con la matriz de transformación actual del Transform.
 * * @param deltaTime Tiempo transcurrido desde el último frame.
 * @param deviceContext Contexto del dispositivo para actualizar los datos del buffer en la GPU.
 */
void Actor::update(float deltaTime, DeviceContext& deviceContext) {
	for (auto& component : m_components) {
		if (component) {
			component->update(deltaTime);
		}
	}

	// Preparar la matriz de mundo (transpuesta para HLSL) y el color base
	m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);
	m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Enviar datos actualizados a la GPU
	m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

/**
 * @brief Renderiza el actor y sus mallas asociadas.
 * * Configura la topología, recorre cada malla del actor vinculando sus respectivos
 * Vertex y Index Buffers, y gestiona la asignación de texturas al Pixel Shader.
 * * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
 */
void Actor::render(DeviceContext& deviceContext) {
	m_sampler.render(deviceContext, 0, 1);
	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int i = 0; i < m_meshes.size(); i++) {
		m_vertexBuffers[i].render(deviceContext, 0, 1);
		m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		m_modelBuffer.render(deviceContext, 2, 1, true);

		// Limpiar texturas previas por seguridad para evitar fugas de estado
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

		// Asignar textura principal si el actor tiene texturas cargadas
		if (!m_textures.empty()) {
			m_textures[0].render(deviceContext, 0, 1);
		}

		deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
	}
}

/**
 * @brief Renderizado simplificado diseñado para objetos tipo Skybox.
 * * Omite el setup de materiales y texturas complejas, centrándose solo en la geometría.
 * * @param deviceContext Contexto del dispositivo para el renderizado.
 */
void Actor::renderForSkybox(DeviceContext& deviceContext) {
	deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (unsigned int i = 0; i < m_meshes.size(); i++) {
		m_vertexBuffers[i].render(deviceContext, 0, 1);
		m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);
		deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
	}
}

/**
 * @brief Libera todos los recursos de GPU asociados al actor.
 * * Destruye buffers de vértices, índices, texturas y estados de sampler de forma segura.
 */
void Actor::destroy() {
	for (auto& vertexBuffer : m_vertexBuffers) {
		vertexBuffer.destroy();
	}
	for (auto& indexBuffer : m_indexBuffers) {
		indexBuffer.destroy();
	}
	for (auto& tex : m_textures) {
		tex.destroy();
	}
	m_modelBuffer.destroy();
	m_sampler.destroy();
}

/**
 * @brief Asigna un conjunto de mallas al actor y crea sus recursos de GPU correspondientes.
 * * Para cada MeshComponent proporcionado, inicializa y almacena un Vertex Buffer y un Index Buffer.
 * * @param device Referencia al dispositivo para la creación de los buffers.
 * @param meshes Vector de MeshComponent que contienen la información geométrica bruta.
 */
void Actor::setMesh(Device& device, std::vector<MeshComponent> meshes) {
	m_meshes = meshes;
	HRESULT hr;
	for (auto& mesh : m_meshes) {
		// Inicialización del Vertex Buffer
		Buffer vertexBuffer;
		hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Actor", "setMesh", "Failed to create new vertexBuffer");
		}
		else {
			m_vertexBuffers.push_back(vertexBuffer);
		}

		// Inicialización del Index Buffer
		Buffer indexBuffer;
		hr = indexBuffer.init(device, mesh, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Actor", "setMesh", "Failed to create new indexBuffer");
		}
		else {
			m_indexBuffers.push_back(indexBuffer);
		}
	}
}