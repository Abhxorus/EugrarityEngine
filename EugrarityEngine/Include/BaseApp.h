#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "ECS/Actor.h"
#include "EngineUtilities\GUI/GUI.h"
#include "SceneGraph\SceneGraph.h"
#include "EngineUtilities\Utilities\Camera.h"

extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class
	BaseApp {
public:
	BaseApp() = default;
	~BaseApp() { destroy(); }

	HRESULT
		awake();

	int
		run(HINSTANCE hInst, int nCmdShow);

	HRESULT
		init();

	void
		update(float deltaTime);

	void
		render();

	void
		destroy();

private:
	static LRESULT CALLBACK
		WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Window                              m_window;
	Device															m_device;
	DeviceContext												m_deviceContext;
	SwapChain                           m_swapChain;
	Texture                             m_backBuffer;
	RenderTargetView									  m_renderTargetView;
	Texture                             m_depthStencil;
	DepthStencilView									  m_depthStencilView;
	Viewport                            m_viewport;
	ShaderProgram												m_shaderProgram;
	Buffer															m_cbNeverChanges;
	Buffer															m_cbChangeOnResize;
	Texture 														m_cyberGunAlbedo;
	Texture															m_skyboxTex;

	Camera															m_camera;

	SceneGraph													m_sceneGraph;
	std::vector<EU::TSharedPointer<Actor>> m_actors;
	EU::TSharedPointer<Actor> m_cyberGun;


	Model3D* m_model;

	CBChangeOnResize										cbChangesOnResize;
	CBNeverChanges											cbNeverChanges;
	GUI																m_gui;
};