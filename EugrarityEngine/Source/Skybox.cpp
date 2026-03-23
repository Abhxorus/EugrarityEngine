#include "EngineUtilities/Utilities/Skybox.h"
#include "Device.h"
#include "DeviceContext.h"

/**
 * @brief Inicializa todos los componentes necesarios para el renderizado del Skybox.
 * * Configura la geometría del cubo unitario, crea el Actor de soporte, carga los shaders
 * de HLSL, y establece los estados de Pipeline (Rasterizer con Front-Culling y Depth con Less-Equal).
 * * @param device Referencia al dispositivo de hardware.
 * @param deviceContext Puntero al contexto del dispositivo (no se usa directamente aquí, pero se mantiene por firma).
 * @param cubemap Referencia a la textura de tipo Cubemap ya cargada.
 * @return HRESULT S_OK si todo se inicializó correctamente.
 */
HRESULT
Skybox::init(Device& device, DeviceContext* deviceContext, Texture& cubemap) {
	destroy();

	// Cargar el cubemap compartido
	m_skyboxTexture = cubemap;

	// 1) Definición de Geometría (Cubo unitario centrado en el origen)
	const SkyboxVertex vertices[] = {
			{-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1}, {+1,-1,-1}, // Cara trasera
			{-1,-1,+1}, {-1,+1,+1}, {+1,+1,+1}, {+1,-1,+1}, // Cara frontal
	};

	const unsigned int indices[] =
	{
		// back (-Z)
		0,1,2, 0,2,3,
		// front (+Z)
		4,6,5, 4,7,6,
		// left (-X)
		4,5,1, 4,1,0,
		// right (+X)
		3,2,6, 3,6,7,
		// top (+Y)
		1,5,6, 1,6,2,
		// bottom (-Y)
		4,0,3, 4,3,7
	};

	// Creación del Actor para el Skybox dentro del sistema ECS
	m_skybox = EU::MakeShared<Actor>(device);

	if (!m_skybox.isNull()) {
		std::vector<MeshComponent> skybox;
		// Instanciación del modelo 3D con los datos crudos definidos arriba
		m_cubeModel = new Model3D("Skybox", vertices, indices);
		skybox = m_cubeModel->GetMeshes();

		m_skybox->setMesh(device, skybox);
		m_skybox->setName("skybox");
	}
	else {
		ERROR("Skybox", "Init", "Failed to create Skybox Actor.");
		return E_FAIL;
	}

	// Definición del Layout de entrada (Solo posición para el Skybox)
	std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
	D3D11_INPUT_ELEMENT_DESC position;
	position.SemanticName = "POSITION";
	position.SemanticIndex = 0;
	position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	position.InputSlot = 0;
	position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	position.InstanceDataStepRate = 0;
	Layout.push_back(position);

	HRESULT hr = S_OK;

	// Inicialización del programa de Shaders (Skybox.hlsl)
	hr = m_shaderProgram.init(device, "Skybox.hlsl", Layout);
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Inicialización del Constant Buffer para la matriz View-Projection
	hr = m_constantBuffer.init(device, sizeof(CBSkybox));
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Inicialización del Sampler State para el filtrado del Cubemap
	hr = m_samplerState.init(device);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new SamplerState");
	}

	// Inicialización del Rasterizer: Cull Front porque estamos dentro del cubo
	hr = m_rasterizerState.init(device, D3D11_FILL_SOLID, D3D11_CULL_FRONT, false, true);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new RasterizerState");
	}

	// Inicialización del DepthStencil: Comparación Less-Equal y sin escritura de profundidad
	// para que el Skybox se dibuje "detrás" de cualquier objeto u opacidad.
	hr = m_depthStencilState.init(device, true,
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_LESS_EQUAL);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new DepthStencilState");
	}

	return S_OK;
}

/**
 * @brief Ejecuta el proceso de dibujado del Skybox.
 * * Utiliza una matriz de vista sin traslación para mantener el cielo centrado en la cámara.
 * Gestiona la vinculación de recursos en el slot 10 para evitar colisiones con texturas de objetos.
 * * @param deviceContext Contexto del dispositivo para comandos de GPU.
 * @param camera Referencia a la cámara para obtener matrices de transformación.
 */
void
Skybox::render(DeviceContext& deviceContext, Camera& camera) {
	// Verificación de seguridad antes de renderizar
	if (!m_cubeModel || !m_skyboxTexture.m_textureFromImg) return;

	// 1) Aplicar estados de renderizado específicos del Skybox
	m_rasterizerState.render(deviceContext);
	m_depthStencilState.render(deviceContext, 0, false);

	// 2) Preparar matrices: Se usa ViewNoTranslation para que el cielo no se aleje al mover la cámara
	XMMATRIX viewNoT = camera.GetViewNoTranslation();
	XMMATRIX vp = viewNoT * camera.getProj();
	CBSkybox cb{};
	cb.mviewProj = XMMatrixTranspose(vp);

	// Actualización del buffer constante en el slot 0
	m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
	m_constantBuffer.render(deviceContext, 0, 1);

	// 3) Vincular Shader y Sampler (Sampler en slot 10 según convención del motor)
	m_shaderProgram.render(deviceContext);
	m_samplerState.render(deviceContext, 10, 1);

	// 4) Vincular la textura Cubemap en el slot 10
	m_skyboxTexture.render(deviceContext, 10, 1);

	// 5) Renderizar la geometría del cubo (IA + DrawIndexed)
	m_skybox->renderForSkybox(deviceContext);

	// 6) Limpieza de estados: Desvincular SRV de los slots para evitar conflictos en el siguiente draw call
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV); // Limpiar slot 10 (Skybox)
	deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);  // Limpiar slot 0 (General)
}