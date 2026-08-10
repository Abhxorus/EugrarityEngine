/**
 * @file BaseApp.cpp
 * @brief Implementa la logica de BaseApp dentro del subsistema Core.
 * @ingroup core
 */
#include "BaseApp.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include "ECS/AudioSourceComponent.h"
#include "EngineUtilities/Utilities/PlatformUtils.h" // <-- Neva utilidad de Windows
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>

namespace {
	bool isSerializedLightActorName(const std::string& actorName)
	{
		return actorName.rfind("Light Actor", 0) == 0;
	}

	void ensureDefaultLightComponent(const EU::TSharedPointer<Actor>& actor)
	{
		if (actor.isNull()) {
			return;
		}

		EU::TSharedPointer<LightComponent> lightComponent = actor->getComponent<LightComponent>();
		if (!lightComponent) {
			lightComponent = EU::MakeShared<LightComponent>();
			actor->addComponent(lightComponent);
		}

		LightData& light = lightComponent->getLightData();
		light.type = LightType::Directional;
		light.color = EU::Vector3(1.0f, 1.0f, 1.0f);
		light.intensity = 1.0f;
		light.direction = EU::Vector3(-0.20f, -1.0f, 1.0f);
		light.range = 12.0f;
		light.spotAngle = 0.0f;
		lightComponent->setCastShadow(true);
	}
}

HRESULT
BaseApp::awake() {
	HRESULT hr = S_OK;

	// Inicializacion de dlls y elementos externos al motor.
	m_sceneGraph.init();

	// Log Success Message
	MESSAGE("Main", "Awake", "Application awake successfully.");
	return hr;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
	// 1) Initialize Window
	if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
		ERROR("Main", "Run", "Failed to initialize window.");
		return 0;
	}
	// 2) Awake Application
	if (FAILED(awake())) {
		ERROR("Main", "Run", "Failed to awake application.");
		return 0;
	}
	// 3) Initialize Device and Device Context
	if (FAILED(init())) {
		ERROR("Main", "Run", "Failed to initialize device and device context.");
		return 0;
	}
	// 4) Initialize GUI
	m_gui.init(m_window, m_device, m_deviceContext);
	m_guiInitialized = true;

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

	AudioManager::init();

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
	m_d3dReady = true;

	// Load Resources -> Modelos, Texturas e Interfaz de usuario
	std::array<std::string, 6> faces = {
		"Skybox/cubemap_0.png",
		"Skybox/cubemap_1.png",
		"Skybox/cubemap_2.png",
		"Skybox/cubemap_3.png",
		"Skybox/cubemap_4.png",
		"Skybox/cubemap_5.png"
	};
	m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);
	HRESULT lightIconHr = m_lightIconTexture.init(m_device, "slate/icons/light-bulb", PNG);
	if (FAILED(lightIconHr)) {
		MESSAGE("Main", "InitDevice", "Light actor icon not found. Continuing with fallback light marker.");
	}

	// Set CyberGun Actor
	m_cyberGun = EU::MakeShared<Actor>(m_device);

	if (!m_cyberGun.isNull()) {
		// --- NUEVO: Selección de Modelo mediante el explorador de Windows ---
		std::string modelPath = PlatformUtils::OpenFileDialog("Modelos FBX\0*.fbx\0Todos los archivos\0*.*\0");

		if (modelPath.empty()) {
			ERROR("Main", "InitDevice", "No se selecciono ningun modelo FBX. Abortando inicializacion.");
			return E_FAIL;
		}

		m_model = new Model3D(modelPath, ModelType::FBX);
		if (!m_model || !m_model->load(modelPath)) {
			ERROR("Main", "InitDevice", "Failed to load FBX model.");
			return E_FAIL;
		}

		// Asegúrate de cambiar la extensión a .jpg y el flag a JPG si tus archivos no son PNG
		hr = m_AlbedoSRV.init(m_device, "C:/Users/Usuario/Documents/GitHub/EugrarityEngine/EugrarityEngine/bin/Assets/textures/E-45_col", JPG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", "Failed to initialize Albedo Texture.");
			return hr;
		}

		hr = m_NormalSRV.init(m_device, "C:/Users/Usuario/Documents/GitHub/EugrarityEngine/EugrarityEngine/bin/Assets/textures/E-45-nor_1", JPG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", "Failed to initialize Normal Texture.");
			return hr;
		}

		hr = m_MetallicSRV.init(m_device, "C:/Users/Usuario/Documents/GitHub/EugrarityEngine/EugrarityEngine/bin/Assets/textures/E-45_REF 1", JPG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", "Failed to initialize Metallic Texture.");
			return hr;
		}

		hr = m_RoughnessSRV.init(m_device, "C:/Users/Usuario/Documents/GitHub/EugrarityEngine/EugrarityEngine/bin/Assets/textures/E-45_REF 1", JPG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", "Failed to initialize Roughness Texture.");
			return hr;
		}

		hr = m_AOSRV.init(m_device, "C:/Users/Usuario/Documents/GitHub/EugrarityEngine/EugrarityEngine/bin/Assets/textures/E-45_col_2", JPG);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice", "Failed to initialize AO Texture.");
			return hr;
		}

		// Omitimos el Emissive si no hay una textura para ello en la carpeta
		m_EmissiveSRV.destroy();

		m_cyberGun->setName("Nave/Modelo Dinamico");
		m_actors.push_back(m_cyberGun);

		m_cyberGun->getComponent<Transform>()->setTransform(EU::Vector3(2.0f, -1.90f, 11.60f),
			EU::Vector3(-0.60f, 3.0f, -0.20f),
			EU::Vector3(1.0f, 1.0f, 1.0f));
	}
	else {
		ERROR("Main", "InitDevice", "Failed to create cyber Gun Actor.");
		return E_FAIL;
	}

	// Store the Actors in the Scene Graph
	for (auto& actor : m_actors) {
		m_sceneGraph.addEntity(actor.get());
	}

	LayoutBuilder builder;

	builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
		.Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	// Create the Shader Program
	hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Create the constant buffers
	hr = m_constantBuffer.init(m_device, sizeof(CBMain));
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize m_constantBuffer Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 500.0f);
	m_camera.setPosition(0.0f, 10.0f, -40.0f);
	m_camera.lookAt(EU::Vector3(0.0f, 3.0f, -6.0f), EU::Vector3(2.0f, -1.90f, 11.60f), EU::Vector3(0.0f, 1.0f, 0.0f));

	m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
	m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

	// Initialize the Skybox pass
	m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

	// Initialize default states (Rasterizer, DepthStencil)
	hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize default Rasterizer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize default DepthStencilState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	hr = m_defaultSampler.init(m_device);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize default SamplerState. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	m_pbrMaterial.setShader(&m_shaderProgram);
	m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_pbrMaterial.setSamplerState(&m_defaultSampler);
	m_pbrMaterial.setDomain(MaterialDomain::Opaque);
	m_pbrMaterial.setBlendMode(BlendMode::Opaque);

	m_transparentPbrMaterial.setShader(&m_shaderProgram);
	m_transparentPbrMaterial.setRasterizerState(&m_defaultRasterizer);
	m_transparentPbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
	m_transparentPbrMaterial.setSamplerState(&m_defaultSampler);
	m_transparentPbrMaterial.setDomain(MaterialDomain::Transparent);
	m_transparentPbrMaterial.setBlendMode(BlendMode::Alpha);

	auto configurePbrMaterial = [&](Material& material) {
		material.setShader(&m_shaderProgram);
		material.setRasterizerState(&m_defaultRasterizer);
		material.setDepthStencilState(&m_defaultDepthStencil);
		material.setSamplerState(&m_defaultSampler);
		material.setDomain(MaterialDomain::Opaque);
		material.setBlendMode(BlendMode::Opaque);
		};

	configurePbrMaterial(m_cyberGunPbrMaterial);

	m_cyberGunMaterial.setMaterial(&m_cyberGunPbrMaterial);
	m_cyberGunMaterial.setAlbedo(&m_AlbedoSRV);
	m_cyberGunMaterial.setNormal(&m_NormalSRV);
	m_cyberGunMaterial.setMetallic(&m_MetallicSRV);
	m_cyberGunMaterial.setRoughness(&m_RoughnessSRV);
	m_cyberGunMaterial.setAO(&m_AOSRV);
	if (m_EmissiveSRV.m_textureFromImg) {
		m_cyberGunMaterial.setEmissive(&m_EmissiveSRV);
	}
	m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_cyberGunMaterial.getParams().metallic = 1.0f;
	m_cyberGunMaterial.getParams().roughness = 1.0f;
	m_cyberGunMaterial.getParams().ao = 1.0f;
	m_cyberGunMaterial.getParams().normalScale = 1.0f;
	m_cyberGunMaterial.getParams().emissiveStrength = 1.0f;
	m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

	m_cyberGunRenderMesh.destroy();
	for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
		Submesh submesh{};
		hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize CyberGun vertex buffer. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}

		hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(hr)) {
			ERROR("Main", "InitDevice",
				("Failed to initialize CyberGun index buffer. HRESULT: " + std::to_string(hr)).c_str());
			return hr;
		}

		submesh.indexCount = meshComponent.m_numIndex;
		submesh.localTransform = meshComponent.m_localTransform;
		submesh.materialSlot = 0;
		m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
	}

	EU::TSharedPointer<MeshRendererComponent> meshRenderer = m_cyberGun->getComponent<MeshRendererComponent>();
	if (!meshRenderer) {
		meshRenderer = EU::MakeShared<MeshRendererComponent>();
		m_cyberGun->addComponent(meshRenderer);
	}
	meshRenderer->setMesh(&m_cyberGunRenderMesh);
	meshRenderer->setMaterialInstance(&m_cyberGunMaterial);
	meshRenderer->setVisible(true);
	meshRenderer->setCastShadow(true);

	EU::TSharedPointer<AudioSourceComponent> audioComponent = EU::MakeShared<AudioSourceComponent>();
	m_cyberGun->addComponent(audioComponent);

	// --- NUEVO: Selección de Audio mediante el explorador de Windows ---
	std::string audioPath = PlatformUtils::OpenFileDialog("Audio WAV\0*.wav\0Todos los archivos\0*.*\0");
	if (!audioPath.empty()) {
		if (audioComponent->loadSound(audioPath)) {
			audioComponent->setVolume(0.6f);
			audioComponent->setLooping(true);
			audioComponent->play();
		}
	}
	else {
		MESSAGE("Main", "InitDevice", "No se selecciono ningun audio. El actor no emitira sonido.");
	}

	m_directionalLightActor = EU::MakeShared<Actor>(m_device);
	if (!m_directionalLightActor.isNull()) {
		m_directionalLightActor->setName("Light Actor 1");
		EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
		if (!lightComponent) {
			lightComponent = EU::MakeShared<LightComponent>();
			m_directionalLightActor->addComponent(lightComponent);
		}

		lightComponent->getLightData().type = LightType::Directional;
		lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
		lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
		lightComponent->getLightData().intensity = 1.0f;
		lightComponent->getLightData().range = 12.0f;
		lightComponent->setCastShadow(true);

		EU::TSharedPointer<Transform> transform = m_directionalLightActor->getComponent<Transform>();
		if (transform) {
			transform->setTransform(EU::Vector3(0.0f, 3.0f, 0.0f),
				EU::Vector3(0.0f, 0.0f, 0.0f),
				EU::Vector3(1.0f, 1.0f, 1.0f));
		}

		m_actors.push_back(m_directionalLightActor);
		m_sceneGraph.addEntity(m_directionalLightActor.get());
	}

	loadScene(getDefaultScenePath());

	hr = m_editorViewportPass.init(m_device, 1280, 720);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	hr = m_renderPipeline.init(m_device, RendererType::Deferred);
	if (FAILED(hr)) {
		ERROR("Main", "InitDevice",
			("Failed to initialize RenderPipeline. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

void
BaseApp::update(float deltaTime) {
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
	// Update User Interface
	m_gui.update(m_viewport, m_window);
	m_camera.updateViewMatrix();
	if (m_gui.consumeCreateLightActorRequest()) {
		EU::TSharedPointer<Actor> lightActor = createLightActor();
		if (!lightActor.isNull()) {
			m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
		}
	}
	EU::TSharedPointer<Actor> selectedActor;
	if (m_gui.selectedActorIndex >= 0 &&
		m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
		selectedActor = m_actors[m_gui.selectedActorIndex];
	}
	bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);
	m_gui.drawViewportPanel(m_editorViewportPass.getSRV(), m_actors, m_camera, m_window, selectedActor, m_lightIconTexture.m_textureFromImg);
	m_gui.drawRenderDebugPanel(m_renderPipeline.getPreShadowSRV(), m_editorViewportPass.getSRV(), m_renderPipeline.getShadowMapSRV());
	m_gui.drawGBufferDebugPanel(m_renderPipeline.getGBufferAlbedoMetallicSRV(),
		m_renderPipeline.getGBufferNormalRoughnessSRV(),
		m_renderPipeline.getGBufferWorldAoSRV(),
		m_renderPipeline.getGBufferEmissiveAlphaSRV(),
		selectedActor);
	m_renderPipeline.setShadowFactorDebugEnabled(m_gui.m_visualizeDeferredShadowFactor);
	m_renderPipeline.setDeferredDebugViewMode(m_gui.m_deferredDebugViewMode);
	m_gui.outliner(m_actors);
	if (m_gui.selectedActorIndex >= 0 &&
		m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
		selectedActor = m_actors[m_gui.selectedActorIndex];
	}
	m_gui.inspectorGeneral(selectedActor);
	if (m_gui.consumeSaveSceneRequest()) {
		saveScene(getDefaultScenePath());
	}

	unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
	unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);

	const unsigned int kMinViewportSize = 64;

	if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
	if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

	// Si cambio el tamano solicitado, reinicia estabilidad
	if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight)
	{
		m_lastRequestedViewportWidth = desiredW;
		m_lastRequestedViewportHeight = desiredH;
		m_viewportResizeStableFrames = 0;
	}
	else
	{
		// El tamano ya no cambio este frame
		m_viewportResizeStableFrames++;
	}

	// Solo marcar resize cuando el tamano se haya mantenido estable
	const int kStableFramesRequired = 2;

	if (m_viewportResizeStableFrames >= kStableFramesRequired)
	{
		if (desiredW != m_editorViewportPass.getWidth() ||
			desiredH != m_editorViewportPass.getHeight())
		{
			m_editorViewportResizePending = true;
			m_pendingViewportWidth = desiredW;
			m_pendingViewportHeight = desiredH;
		}
	}

	XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
	XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
	m_constantBufferStruct.CameraPos = m_camera.getPosition();

	// Update Skybox Pass -> Solo necesita la vista sin traslacion + proyeccion para funcionar correctamente
	m_skybox.update(m_deviceContext, m_camera);

	// Update Actors
	m_sceneGraph.update(deltaTime, m_deviceContext);

	AudioManager::update();
}

void
BaseApp::render() {
	handleEditorViewportResize();

	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	m_renderScene.clear();
	m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
	m_renderScene.skybox = &m_skybox;
	m_renderPipeline.render(
		m_deviceContext,
		m_camera,
		m_renderScene,
		m_editorViewportPass
	);

	// 2) Volver al backbuffer principal
	m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
	m_viewport.render(m_deviceContext);
	m_depthStencilView.render(m_deviceContext);

	// 4) GUI
	m_gui.render();

	m_swapChain.present();
}

void
BaseApp::destroy() {
	if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
	m_sceneGraph.destroy();
	m_editorViewportPass.destroy();
	m_renderPipeline.destroy();
	m_cyberGunRenderMesh.destroy();
	m_AlbedoSRV.destroy();
	m_MetallicSRV.destroy();
	m_NormalSRV.destroy();
	m_RoughnessSRV.destroy();
	m_AOSRV.destroy();
	m_EmissiveSRV.destroy();
	m_lightIconTexture.destroy();
	m_defaultRasterizer.destroy();
	m_defaultDepthStencil.destroy();
	m_defaultSampler.destroy();
	m_shaderProgram.destroy();
	m_depthStencil.destroy();
	m_depthStencilView.destroy();
	m_renderTargetView.destroy();
	m_swapChain.destroy();
	m_backBuffer.destroy();

	if (m_guiInitialized) {
		m_gui.destroy();
		m_guiInitialized = false;
	}

	delete m_model;
	m_model = nullptr;

	m_deviceContext.destroy();
	m_device.destroy();
	AudioManager::destroy();
}

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
	case WM_SIZE:
	{
		// Evita recrear cuando esta minimizada
		if (wParam == SIZE_MINIMIZED) return 0;

		unsigned int newW = LOWORD(lParam);
		unsigned int newH = HIWORD(lParam);
		if (newW == 0 || newH == 0) return 0;

		// Recupera tu instancia BaseApp
		BaseApp* app = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (app) app->onResize(newW, newH);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void BaseApp::onResize(unsigned int newW, unsigned int newH)
{
	if (!m_d3dReady) {
		// Aun asi puedes actualizar el tamano logico de la ventana
		m_window.m_width = (int)newW;
		m_window.m_height = (int)newH;
		return;
	}

	if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
	if (newW == 0 || newH == 0) return;

	m_window.m_width = (int)newW;
	m_window.m_height = (int)newH;
	// 2) Desbindea targets actuales (clave antes de destruir)
	ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	// 3) Libera recursos dependientes del tamano (RTV/DSV/Depth/BackBuffer)
	m_renderTargetView.destroy();
	m_depthStencilView.destroy();
	m_depthStencil.destroy();
	m_backBuffer.destroy();

	// 4) Resize swapchain
	HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
	if (FAILED(hr)) return;

	// 5) Re-obten backbuffer
	hr = m_swapChain.getBackBuffer(m_backBuffer);
	if (FAILED(hr)) return;

	// 6) Re-crea RTV
	hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
	if (FAILED(hr)) return;

	// 7) Re-crea Depth/DSV 
	hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
	if (FAILED(hr)) return;

	hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
	if (FAILED(hr)) return;

	// 8) Viewport
	m_viewport.init(m_window);

	// 9) Camara (aspect ratio)  
	m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void BaseApp::handleEditorViewportResize()
{
	if (!m_editorViewportResizePending)
		return;

	// Desbindear antes de tocar recursos
	m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	m_deviceContext.m_deviceContext->PSSetShaderResources(
		0,
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
		nullSRVs
	);

	// Crear pass temporal nuevo
	EditorViewportPass newPass;
	HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
	if (FAILED(hr))
	{
		// Si falla, conserva el pass actual
		m_editorViewportResizePending = false;
		return;
	}

	// Intercambio seguro: el pass viejo queda en newPass y se destruye al salir
	m_editorViewportPass.swap(newPass);
	m_renderPipeline.resize(m_device, m_pendingViewportWidth, m_pendingViewportHeight);

	m_editorViewportResizePending = false;
}

std::string BaseApp::getDefaultScenePath() const
{
	CreateDirectoryA("Saved", nullptr);
	return "Saved/DefaultScene.wvscene";
}

EU::TSharedPointer<Actor> BaseApp::createLightActor(const std::string& name)
{
	EU::TSharedPointer<Actor> lightActor = EU::MakeShared<Actor>(m_device);
	if (lightActor.isNull()) {
		ERROR("Main", "createLightActor", "Failed to create Light Actor.");
		return lightActor;
	}

	size_t lightActorCount = 0;
	for (const auto& actor : m_actors) {
		if (!actor.isNull() && !actor->getComponent<LightComponent>().isNull()) {
			++lightActorCount;
		}
	}

	lightActor->setName(name.empty() ? "Light Actor " + std::to_string(lightActorCount + 1) : name);

	EU::TSharedPointer<LightComponent> lightComponent = lightActor->getComponent<LightComponent>();
	if (!lightComponent) {
		lightComponent = EU::MakeShared<LightComponent>();
		lightActor->addComponent(lightComponent);
	}

	lightComponent->getLightData().type = LightType::Point;
	lightComponent->getLightData().direction = EU::Vector3(-0.20f, -1.0f, 1.0f);
	lightComponent->getLightData().color = EU::Vector3(1.0f, 1.0f, 1.0f);
	lightComponent->getLightData().intensity = 1.0f;
	lightComponent->getLightData().range = 12.0f;
	lightComponent->setCastShadow(false);

	EU::TSharedPointer<Transform> transform = lightActor->getComponent<Transform>();
	if (transform) {
		const float lightOffset = static_cast<float>(lightActorCount) * 2.0f;
		transform->setTransform(EU::Vector3(lightOffset, 3.0f, 0.0f),
			EU::Vector3(0.0f, 0.0f, 0.0f),
			EU::Vector3(1.0f, 1.0f, 1.0f));
	}

	m_actors.push_back(lightActor);
	m_sceneGraph.addEntity(lightActor.get());
	return lightActor;
}

bool BaseApp::saveScene(const std::string& path)
{
	std::ofstream stream(path, std::ios::trunc);
	if (!stream.is_open()) {
		ERROR("Main", "saveScene", ("Failed to open scene file for writing: " + path).c_str());
		return false;
	}

	stream << "WVSCENE 1\n";
	stream << "ACTOR_COUNT " << m_actors.size() << "\n";

	for (size_t actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex) {
		const EU::TSharedPointer<Actor>& actor = m_actors[actorIndex];
		if (actor.isNull()) {
			continue;
		}

		stream << "ACTOR " << actorIndex << " " << std::quoted(actor->getName()) << "\n";

		EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
		if (transform) {
			const EU::Vector3& position = transform->getPosition();
			const EU::Vector3& rotation = transform->getRotation();
			const EU::Vector3& scale = transform->getScale();
			stream << "POSITION " << position.x << " " << position.y << " " << position.z << "\n";
			stream << "ROTATION " << rotation.x << " " << rotation.y << " " << rotation.z << "\n";
			stream << "SCALE " << scale.x << " " << scale.y << " " << scale.z << "\n";
		}

		EU::TSharedPointer<MeshRendererComponent> meshRenderer = actor->getComponent<MeshRendererComponent>();
		if (meshRenderer) {
			stream << "VISIBLE " << (meshRenderer->isVisible() ? 1 : 0) << "\n";
			stream << "CAST_SHADOW " << (meshRenderer->canCastShadow() ? 1 : 0) << "\n";

			const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
			stream << "MATERIAL_COUNT " << materials.size() << "\n";
			for (size_t i = 0; i < materials.size(); ++i) {
				MaterialInstance* materialInstance = materials[i];
				if (!materialInstance) {
					stream << "MATERIAL " << i << " 0 0 1 1 1 1 0 1 1 1 0.5\n";
					continue;
				}

				Material* material = materialInstance->getMaterial();
				const MaterialParams& params = materialInstance->getParams();
				const int domain = material ? static_cast<int>(material->getDomain()) : 0;
				const int blendMode = material ? static_cast<int>(material->getBlendMode()) : 0;

				stream << "MATERIAL " << i << " "
					<< domain << " "
					<< blendMode << " "
					<< params.baseColor.x << " "
					<< params.baseColor.y << " "
					<< params.baseColor.z << " "
					<< params.baseColor.w << " "
					<< params.metallic << " "
					<< params.roughness << " "
					<< params.ao << " "
					<< params.normalScale << " "
					<< params.alphaCutoff << "\n";
			}
		}

		EU::TSharedPointer<LightComponent> lightComponent = actor->getComponent<LightComponent>();
		if (lightComponent) {
			const LightData& light = lightComponent->getLightData();
			stream << "LIGHT_COMPONENT "
				<< static_cast<int>(light.type) << " "
				<< light.color.x << " "
				<< light.color.y << " "
				<< light.color.z << " "
				<< light.intensity << " "
				<< light.direction.x << " "
				<< light.direction.y << " "
				<< light.direction.z << " "
				<< light.range << " "
				<< light.spotAngle << " "
				<< (lightComponent->canCastShadow() ? 1 : 0) << "\n";
		}

		stream << "END_ACTOR\n";
	}

	stream << "LIGHT "
		<< m_constantBufferStruct.LightDir.x << " "
		<< m_constantBufferStruct.LightDir.y << " "
		<< m_constantBufferStruct.LightDir.z << " "
		<< m_constantBufferStruct.LightColor.x << " "
		<< m_constantBufferStruct.LightColor.y << " "
		<< m_constantBufferStruct.LightColor.z << "\n";

	stream << "END_SCENE\n";
	const std::wstring pathW(path.begin(), path.end());
	MESSAGE("Main", "saveScene", L"Saved scene to '" << pathW << L"'")
		return true;
}

bool BaseApp::loadScene(const std::string& path)
{
	std::ifstream stream(path);
	if (!stream.is_open()) {
		return false;
	}

	std::string token;
	stream >> token;
	if (token != "WVSCENE") {
		return false;
	}

	int version = 0;
	stream >> version;
	if (version != 1) {
		return false;
	}

	EU::TSharedPointer<Actor> currentActor;
	while (stream >> token) {
		if (token == "ACTOR_COUNT") {
			size_t ignoredCount = 0;
			stream >> ignoredCount;
		}
		else if (token == "ACTOR") {
			size_t actorIndex = 0;
			std::string actorName;
			stream >> actorIndex >> std::quoted(actorName);
			currentActor = EU::TSharedPointer<Actor>();
			while (actorIndex >= m_actors.size()) {
				EU::TSharedPointer<Actor> newActor = EU::MakeShared<Actor>(m_device);
				if (newActor.isNull()) {
					break;
				}
				newActor->setName("Actor " + std::to_string(m_actors.size() + 1));
				m_actors.push_back(newActor);
				m_sceneGraph.addEntity(newActor.get());
			}
			if (actorIndex < m_actors.size()) {
				currentActor = m_actors[actorIndex];
			}
			if (!currentActor.isNull()) {
				currentActor->setName(actorName);
				if (isSerializedLightActorName(actorName)) {
					ensureDefaultLightComponent(currentActor);
				}
			}
		}
		else if (token == "POSITION" && !currentActor.isNull()) {
			float x = 0.0f, y = 0.0f, z = 0.0f;
			stream >> x >> y >> z;
			EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
			if (transform) {
				transform->setPosition(EU::Vector3(x, y, z));
			}
		}
		else if (token == "ROTATION" && !currentActor.isNull()) {
			float x = 0.0f, y = 0.0f, z = 0.0f;
			stream >> x >> y >> z;
			EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
			if (transform) {
				transform->setRotation(EU::Vector3(x, y, z));
			}
		}
		else if (token == "SCALE" && !currentActor.isNull()) {
			float x = 1.0f, y = 1.0f, z = 1.0f;
			stream >> x >> y >> z;
			EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
			if (transform) {
				transform->setScale(EU::Vector3(x, y, z));
			}
		}
		else if (token == "VISIBLE" && !currentActor.isNull()) {
			int value = 1;
			stream >> value;
			EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
			if (meshRenderer) {
				meshRenderer->setVisible(value != 0);
			}
		}
		else if (token == "CAST_SHADOW" && !currentActor.isNull()) {
			int value = 1;
			stream >> value;
			EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
			if (meshRenderer) {
				meshRenderer->setCastShadow(value != 0);
			}
		}
		else if (token == "MATERIAL_COUNT") {
			size_t ignoredCount = 0;
			stream >> ignoredCount;
		}
		else if (token == "MATERIAL" && !currentActor.isNull()) {
			size_t materialIndex = 0;
			int domain = 0;
			int blendMode = 0;
			MaterialParams params{};
			stream >> materialIndex
				>> domain
				>> blendMode
				>> params.baseColor.x
				>> params.baseColor.y
				>> params.baseColor.z
				>> params.baseColor.w
				>> params.metallic
				>> params.roughness
				>> params.ao
				>> params.normalScale
				>> params.alphaCutoff;

			EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
			if (meshRenderer) {
				const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
				if (materialIndex < materials.size() && materials[materialIndex]) {
					materials[materialIndex]->getParams() = params;
					Material* material = materials[materialIndex]->getMaterial();
					if (material) {
						material->setDomain(static_cast<MaterialDomain>(domain));
						material->setBlendMode(static_cast<BlendMode>(blendMode));
					}
				}
			}
		}
		else if (token == "LIGHT_COMPONENT" && !currentActor.isNull()) {
			int type = 0;
			int castShadow = 0;
			LightData light{};
			stream >> type
				>> light.color.x
				>> light.color.y
				>> light.color.z
				>> light.intensity
				>> light.direction.x
				>> light.direction.y
				>> light.direction.z
				>> light.range
				>> light.spotAngle
				>> castShadow;

			if (type < static_cast<int>(LightType::Directional) || type > static_cast<int>(LightType::Spot)) {
				type = static_cast<int>(LightType::Point);
			}
			light.type = static_cast<LightType>(type);

			EU::TSharedPointer<LightComponent> lightComponent = currentActor->getComponent<LightComponent>();
			if (!lightComponent) {
				lightComponent = EU::MakeShared<LightComponent>();
				currentActor->addComponent(lightComponent);
			}
			lightComponent->getLightData() = light;
			lightComponent->setCastShadow(castShadow != 0);
		}
		else if (token == "LIGHT") {
			stream >> m_constantBufferStruct.LightDir.x
				>> m_constantBufferStruct.LightDir.y
				>> m_constantBufferStruct.LightDir.z
				>> m_constantBufferStruct.LightColor.x
				>> m_constantBufferStruct.LightColor.y
				>> m_constantBufferStruct.LightColor.z;

			if (!m_directionalLightActor.isNull()) {
				EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
				if (lightComponent) {
					lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
					lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
				}
			}
		}
		else if (token == "END_ACTOR") {
			currentActor = EU::TSharedPointer<Actor>();
		}
		else if (token == "END_SCENE") {
			break;
		}
	}

	const std::wstring pathW(path.begin(), path.end());
	MESSAGE("Main", "loadScene", L"Loaded scene from '" << pathW << L"'")
		return true;
}