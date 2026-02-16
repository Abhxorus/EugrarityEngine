#include "BaseApp.h"
#include "Model3D.h"
#include "ECS/Actor.h"

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
		return 0;
	}
	if (FAILED(init()))
		return 0;
	// Main message loop
	MSG msg = {};
	LARGE_INTEGER freq, prev;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&prev);
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			LARGE_INTEGER curr;
			QueryPerformanceCounter(&curr);
			float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
			prev = curr;
			update(deltaTime);
			render();
		}
	}
	return (int)msg.wParam;
}

HRESULT
BaseApp::init() {
	HRESULT hr = S_OK;

	// Crear swapchain
	hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize SwpaChian. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear render target view
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear textura de depth stencil
	hr = m_depthStencil.init(m_device,
		m_window.m_width,
		m_window.m_height,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_BIND_DEPTH_STENCIL,
		1,
		0);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Crear el depth stencil view
	hr = m_depthStencilView.init(m_device,
		m_depthStencil,
		DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}


	// Crear el m_viewport
	hr = m_viewport.init(m_window);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Load Resources


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

	D3D11_INPUT_ELEMENT_DESC texcoord;
	texcoord.SemanticName = "TEXCOORD";
	texcoord.SemanticIndex = 0;
	texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
	texcoord.InputSlot = 0;
	texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
	texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	texcoord.InstanceDataStepRate = 0;
	Layout.push_back(texcoord);

	// Create the Shader Program
	hr = m_shaderProgram.init(m_device, "EugrarityEngine.fx", Layout);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	/*
	// Create vertex buffer
	SimpleVertex vertices[] =
	{
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	};
	*/
	unsigned int indices[] =
	{
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
	};
	/*
	// Integrar los vertices a meshcomponent
	for (unsigned int i = 0; i < 24; i++) {
		m_mesh.m_vertex.push_back(vertices[i]);
	}
	m_mesh.m_numVertex = 24;
	
	// Integrar los indices a meshcomponent
	for (unsigned int i = 0; i < 36; i++) {
		m_mesh.m_index.push_back(indices[i]);
	}
	m_mesh.m_numIndex = 36;

	// Create vertex buffer
	hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize VertexBuffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create index buffer
	hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);

	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize IndexBuffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	*/
	// Set primitive topology
	m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Cargar modelos
	{
		Model3D fbxResource("MiModelo", ModelType::FBX);
		// Verificamos si la carga fue exitosa antes de continuar
		if (fbxResource.load("Assets/tu_modelo.fbx")) {
			auto nuevoActor = EU::MakeShared<Actor>(m_device);
			nuevoActor->setMesh(m_device, fbxResource.GetMeshes());
			nuevoActor->setName("Modelo3D_Cargado");

			// --- MODIFICACIÓN IMPORTANTE: Ajuste de Transform ---
			// Los FBX suelen ser gigantes (x100). Los reducimos para verlos.
			auto transform = nuevoActor->getComponent<Transform>();
			if (transform) {
				transform->setTransform(
					EU::Vector3(0.0f, 0.0f, 0.0f),  // Posición: Un poco abajo y al frente (Z=10)
					EU::Vector3(180.0f, 0.0f, 135.0f),  // Rotación: Girado 180 grados (a veces vienen de espaldas)
					EU::Vector3(1.0f, 1.0f, 1.0f)  // Escala: Reducida al 1%
				);
			}
			// ---------------------------------------------------

			m_actors.push_back(nuevoActor);
		}
		else {
			// Si falla, imprimimos error pero no rompemos el programa inmediatamente
			ERROR("BaseApp", "init", "ERROR FATAL: No se encontro Assets/tu_modelo.fbx");
		}
	}
	// Create the constant buffers
	hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize ChangeOnResize Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize ChangesEveryFrame Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_textureCube.init(m_device, "Cracked2", ExtensionType::PNG);
	// Load the Texture
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize texture Cube. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create the sample state
	hr = m_samplerState.init(m_device);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize SamplerState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Initialize the world matrices
	m_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);


	// Initialize the projection matrix
	cbNeverChanges.mView = XMMatrixTranspose(m_View);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

	return S_OK;
}

void BaseApp::update(float deltaTime)
{
	// Update our time
	static float t = 0.0f;
	if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}
	// Actualizar la matriz de proyección y vista
	cbNeverChanges.mView = XMMatrixTranspose(m_View);
	m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
	m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

	// Modify the color
	m_vMeshColor.x = 1.0f;
	m_vMeshColor.y = 1.0f;
	m_vMeshColor.z = 1.0f;
	// Rotate cube around the origin
	m_World = XMMatrixRotationY(t);
	cb.mWorld = XMMatrixTranspose(m_World);
	cb.vMeshColor = m_vMeshColor;
	m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);

	for (auto& actor : m_actors) {
		if (actor) {
			actor->update(deltaTime, m_deviceContext);
		}
	}
}

void
BaseApp::render() {
	// Set Render Target View
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

	// Set Viewport
	m_viewport.render(m_deviceContext);

	// Set depth stencil view
	m_depthStencilView.render(m_deviceContext);

	// Set shader program
	m_shaderProgram.render(m_deviceContext);

	// Render the cube
	 // Asignar buffers Vertex e Index
	//m_vertexBuffer.render(m_deviceContext, 0, 1);
	//m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

	// Asignar buffers constantes
	m_cbNeverChanges.render(m_deviceContext, 0, 1);
	m_cbChangeOnResize.render(m_deviceContext, 1, 1);
	m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
	m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true);

	// Asignar textura y sampler
	m_textureCube.render(m_deviceContext, 0, 1);
	m_samplerState.render(m_deviceContext, 0, 1);
	//m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

	// Present our back buffer to our front buffer
	for (auto& actor : m_actors) {
		if (actor) {
			actor->render(m_deviceContext);
		}
	}
	m_swapChain.present();
}

void
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

	m_samplerState.destroy();
	m_textureCube.destroy();

	m_cbNeverChanges.destroy();
	m_cbChangeOnResize.destroy();
	m_cbChangesEveryFrame.destroy();
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
	m_shaderProgram.destroy();
	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapChain.destroy();
	m_backBuffer.destroy();
	m_deviceContext.destroy();
	m_device.destroy();
}

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	//  return true;
	switch (message)
	{
	case WM_CREATE:
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
	}
	return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}