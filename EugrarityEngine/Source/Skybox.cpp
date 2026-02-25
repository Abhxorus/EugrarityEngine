#include "EngineUtilities/Utilities/Skybox.h"
#include "Device.h"
#include "DeviceContext.h"


HRESULT
Skybox::init(Device& device, DeviceContext* deviceContext, Texture& cubemap) {
	destroy();
	// Cargar el cubemap
	m_skyboxTexture = cubemap;

	// 1) Geometría (cubo)
	 // Cubo unitario centrado en origen. (tamaño no importa si quitas traslación)
	const SkyboxVertex vertices[] = {
		{-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1}, {+1,-1,-1}, // back
		{-1,-1,+1}, {-1,+1,+1}, {+1,+1,+1}, {+1,-1,+1}, // front
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

	// Load Model
	m_skybox = EU::MakeShared<Actor>(device);

	if (!m_skybox.isNull()) {
		// Crear vertex buffer y index buffer para el skybox
		std::vector<MeshComponent> skybox;
		m_cubeModel = new Model3D("Skybox", vertices, indices);

		skybox = m_cubeModel->GetMeshes();

		// No texture loading

		m_skybox->setMesh(device, skybox);
		m_skybox->setName("CyberGun");
	}
	else {
		ERROR("Skybox", "Init", "Failed to create Skybox Actor.");
		return E_FAIL;
	}


	// Define the input layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
	D3D11_INPUT_ELEMENT_DESC position;
	position.SemanticName = "POSITION";
	position.SemanticIndex = 0;
	position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	position.InputSlot = 0;
	position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
	position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	position.InstanceDataStepRate = 0;
	Layout.push_back(position);

	HRESULT hr = S_OK;
	// Create the Shader Program
	hr = m_shaderProgram.init(device, "Skybox.fx", Layout);

	// Create the constant buffers
	hr = m_constantBuffer.init(device, sizeof(CBSkybox));  // View
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Init SamplerState
	hr = m_samplerState.init(device);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new SamplerState");
	}

	// Init Rasterizer
	//hr = m_rasterizerState.init(device, true, false);
	//if (FAILED(hr)) {
	//	ERROR("Skybox", "init", "Failed to create new RasterizerState");
	//}

	// Init DepthStencilState
	//hr = m_depthStencilState.init(device, true, false);
	//if (FAILED(hr)) {
	//	ERROR("Skybox", "init", "Failed to create new DepthStencilState");
	//}


	return E_NOTIMPL;
}

void
Skybox::render(DeviceContext& deviceContext, Camera& camera) {
	// Set rasterizer state
	//m_rasterizerState.render(deviceContext);

	// Set depth stencil state
	//m_depthStencilState.render(deviceContext);
	// Render the cube model with the cubemap texture


	// View sin traslación
	XMMATRIX view = camera.getView();
	view = XMMatrixTranspose(camera.GetViewNoTranslation());
	XMMATRIX vp = view * camera.getProj();

	// Set constant buffer
	CBSkybox cb{};
	cb.mviewProj = XMMatrixTranspose(vp);
	m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
	m_constantBuffer.render(deviceContext, 0, 1);

	// Set shader program
	m_shaderProgram.render(deviceContext);

	// Set sampler state
	m_samplerState.render(deviceContext, 0, 1);

	// Draw
	deviceContext.DrawIndexed(m_cubeModel->m_meshes[0].m_index.size(), 0, 0);

	// Set cubemap texture
	m_skyboxTexture.render(deviceContext, 0, 1);
}