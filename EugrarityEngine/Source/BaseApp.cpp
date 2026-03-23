#include "BaseApp.h"
#include "ResourceManager.h"

/**
 * @brief Realiza la activación inicial de los sistemas de la aplicación.
 * * Inicializa el grafo de escena y registra el estado inicial en el log.
 * * @return HRESULT S_OK si la activación fue exitosa.
 */
HRESULT BaseApp::awake() {
	HRESULT hr = S_OK;
	m_sceneGraph.init();
	MESSAGE("Main", "Awake", "Application awake successfully.");
	return hr;
}

/**
 * @brief Ciclo principal de la aplicación.
 * * Se encarga de la creación de la ventana, la inicialización de los recursos de DirectX,
 * el setup de la GUI y el manejo del bucle de mensajes de Windows (Pump Message).
 * Calcula el deltaTime para asegurar una actualización consistente.
 * * @param hInst Instancia de la aplicación.
 * @param nCmdShow Estado de visualización de la ventana.
 * @return int Código de salida de la aplicación.
 */
int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
		ERROR("Main", "Run", "Failed to initialize window.");
		return 0;
	}
	if (FAILED(awake())) {
		ERROR("Main", "Run", "Failed to awake application.");
		return 0;
	}
	if (FAILED(init())) {
		ERROR("Main", "Run", "Failed to initialize device and device context.");
		return 0;
	}
	m_gui.init(m_window, m_device, m_deviceContext);

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

/**
 * @brief Inicializa los recursos de renderizado y carga los activos iniciales.
 * * Configura el SwapChain, RenderTarget, DepthStencil, Viewport, Shaders y buffers constantes.
 * También realiza la carga de prueba del modelo "Monkami" y su respectiva textura.
 * * @return HRESULT S_OK si todos los recursos se crearon correctamente, código de error en caso contrario.
 */
HRESULT BaseApp::init() {
	HRESULT hr = S_OK;

	hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
	if (FAILED(hr)) return hr;

	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) return hr;

	hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 1, 0);
	if (FAILED(hr)) return hr;

	hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	if (FAILED(hr)) return hr;

	hr = m_viewport.init(m_window);
	if (FAILED(hr)) return hr;

	// CARGAR MODELO Y TEXTURAS
	{
		Model3D fbxResource("Monkami", ModelType::FBX);
		if (fbxResource.load("Assets/tu_modelo.fbx")) {
			auto nuevoActor = EU::MakeShared<Actor>(m_device);
			nuevoActor->setMesh(m_device, fbxResource.GetMeshes());
			nuevoActor->setName("Monkami");

			// Cargar Textura
			std::vector<Texture> monkamiTextures;
			Texture texturaCampana;

			HRESULT hrTex = texturaCampana.init(m_device, "Assets/HadaCampana_Campana.png", ExtensionType::PNG);

			if (SUCCEEDED(hrTex)) {
				monkamiTextures.push_back(texturaCampana);
				nuevoActor->setTextures(monkamiTextures);
			}
			else {
				ERROR("BaseApp", "init", "No se pudo cargar la textura HadaCampana_Campana.png");
			}

			auto transform = nuevoActor->getComponent<Transform>();
			if (transform) {
				transform->setTransform(
					EU::Vector3(0.0f, 0.0f, 5.0f),
					EU::Vector3(0.0f, 180.0f, 0.0f),
					EU::Vector3(1.0f, 1.0f, 1.0f)
				);
			}
			m_actors.push_back(nuevoActor);
			m_sceneGraph.addEntity(nuevoActor.get());
		}
		else {
			ERROR("BaseApp", "init", "No se encontro tu_modelo.fbx en Assets.");
		}
	}

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

	D3D11_INPUT_ELEMENT_DESC texcoord;
	texcoord.SemanticName = "TEXCOORD";
	texcoord.SemanticIndex = 0;
	texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
	texcoord.InputSlot = 0;
	texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	texcoord.InstanceDataStepRate = 0;
	Layout.push_back(texcoord);

	hr = m_shaderProgram.init(m_device, "EugrarityEngine.fx", Layout);
	if (FAILED(hr)) return hr;

	hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
	if (FAILED(hr)) return hr;

	hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
	if (FAILED(hr)) return hr;

	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
	m_camera.setPosition(0.0f, 3.0f, -6.0f);

	cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());

	return S_OK;
}

/**
 * @brief Actualiza la lógica de la aplicación en cada frame.
 * * Actualiza la GUI, la cámara, los buffers constantes y el grafo de escena.
 * Gestiona el tiempo transcurrido (t) para animaciones o efectos.
 * * @param deltaTime Tiempo transcurrido desde el último frame en segundos.
 */
void BaseApp::update(float deltaTime) {
	static float t = 0.0f;
	if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
		t += (float)XM_PI * 0.0125f;
	}
	else {
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0) dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	m_gui.update(m_viewport, m_window);

	if (!m_actors.empty()) {
		m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
		m_gui.editTransform(m_camera.getView(), m_camera.getProj(), m_actors[m_gui.selectedActorIndex]);
	}
	m_gui.outliner(m_actors);

	m_camera.updateViewMatrix();
	cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
	m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
	m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

	m_sceneGraph.update(deltaTime, m_deviceContext);
}

/**
 * @brief Ejecuta los comandos de renderizado para el frame actual.
 * * Limpia el render target, establece el viewport y dibuja la escena,
 * buffers constantes y la interfaz de usuario. Finalmente presenta el frame.
 */
void BaseApp::render() {
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
	m_viewport.render(m_deviceContext);
	m_depthStencilView.render(m_deviceContext);
	m_shaderProgram.render(m_deviceContext);

	m_cbNeverChanges.render(m_deviceContext, 0, 1);
	m_cbChangeOnResize.render(m_deviceContext, 1, 1);

	m_sceneGraph.render(m_deviceContext);
	m_gui.render();
	m_swapChain.present();
}

/**
 * @brief Libera todos los recursos cargados y destruye los sistemas de la aplicación.
 * * Llama a los métodos de destrucción de cada componente de hardware y software
 * de manera ordenada para evitar fugas de memoria.
 */
void BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_cbNeverChanges.destroy();
	m_cbChangeOnResize.destroy();
	m_shaderProgram.destroy();
	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapChain.destroy();
	m_backBuffer.destroy();
	m_gui.destroy();
	m_deviceContext.destroy();
	m_device.destroy();
}

/**
 * @brief Procedimiento de ventana para el manejo de mensajes de Windows.
 * * Gestiona eventos de creación, dibujado, destrucción e integra los eventos de ImGui.
 * * @param hWnd Handle de la ventana.
 * @param message Identificador del mensaje.
 * @param wParam Información adicional del mensaje.
 * @param lParam Información adicional del mensaje.
 * @return LRESULT Resultado del procesamiento del mensaje.
 */
LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

	switch (message) {
	case WM_CREATE: {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
	}
				  return 0;
	case WM_PAINT: {
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