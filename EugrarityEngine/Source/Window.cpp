#include "Window.h"
#include "Device.h"
#include "BaseApp.h"

/**
 * @brief Inicializa la ventana de la aplicación utilizando la API de Win32.
 * * Registra la clase de ventana, calcula las dimensiones reales (incluyendo bordes)
 * y muestra la ventana en pantalla. También configura las dimensiones iniciales
 * del área cliente para el motor.
 * * @param hInstance Instancia actual de la aplicación proporcionada por Windows.
 * @param nCmdShow Estado de visualización de la ventana (Minimizado, Maximizado, etc.).
 * @param wndproc Puntero a la función de procedimiento de ventana (WndProc) para procesar mensajes.
 * @return HRESULT S_OK si la ventana se creó con éxito, E_FAIL si falló el registro o la creación.
 */
HRESULT
Window::init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc) {
	// Almacenar la instancia de la aplicación
	m_hInst = hInstance;

	// Configuración y registro de la clase de ventana (WNDCLASSEX)
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; // Redibujar si cambia el ancho o alto
	wcex.lpfnWndProc = wndproc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInst;
	wcex.hIcon = LoadIcon(m_hInst, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

	if (!RegisterClassEx(&wcex)) {
		return E_FAIL;
	}

	// Definir el tamaño deseado del área cliente (donde se renderiza el juego)
	RECT rc = { 0, 0, 1200, 950 };
	m_rect = rc;

	// Ajustar el rectángulo para incluir bordes y barras de título sin reducir el área cliente
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// Creación física de la ventana
	m_hWnd = CreateWindow("TutorialWindowClass",
		m_windowName.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!m_hWnd) {
		MessageBox(nullptr, "CreateWindow failed!", "Error", MB_OK);
		ERROR("Window", "init", "CHECK FOR CreateWindow()");
		return E_FAIL;
	}

	// Mostrar y actualizar la ventana por primera vez
	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	// Obtener las dimensiones finales del área cliente para el Viewport de DirectX
	GetClientRect(m_hWnd, &m_rect);
	m_width = m_rect.right - m_rect.left;
	m_height = m_rect.bottom - m_rect.top;

	return S_OK;
}

/**
 * @brief Método de actualización para lógica específica de la ventana.
 * * Actualmente no realiza ninguna operación por frame.
 */
void
Window::update() {
}

/**
 * @brief Método para renderizado específico de la ventana (GDI).
 * * En este motor, el dibujo lo maneja DirectX, por lo que este método queda libre.
 */
void
Window::render() {
}

/**
 * @brief Limpieza de recursos de la ventana.
 * * La destrucción del HWND suele ser manejada por los mensajes de sistema de Windows (WM_DESTROY).
 */
void
Window::destroy() {
}