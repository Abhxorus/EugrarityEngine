#include "SwapChain.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"
#include "Window.h"

/**
 * @brief Inicializa el Swap Chain y los componentes principales de DirectX 11.
 * * Este método realiza el setup completo del motor gráfico:
 * 1. Crea el Dispositivo (Device) y el Contexto (DeviceContext) probando diferentes drivers.
 * 2. Verifica el soporte de MSAA (Multisample Anti-Aliasing).
 * 3. Navega por la jerarquía DXGI (Device -> Adapter -> Factory).
 * 4. Crea la cadena de intercambio (Swap Chain) vinculada a la ventana.
 * 5. Extrae el Back Buffer para su uso posterior en Render Targets.
 * * @param device Referencia al objeto Device que almacenará el ID3D11Device creado.
 * @param deviceContext Referencia al contexto que almacenará el ID3D11DeviceContext.
 * @param backBuffer Referencia a la textura donde se extraerá el buffer de dibujo del Swap Chain.
 * @param window Objeto ventana que contiene el HWND y dimensiones para el renderizado.
 * @return HRESULT S_OK si todo se inicializó correctamente, o código de error detallado.
 */
HRESULT
SwapChain::init(Device& device,
    DeviceContext& deviceContext,
    Texture& backBuffer,
    Window window) {
    // Verificación de validez de la ventana
    if (!window.m_hWnd) {
        ERROR("SwapChain", "init", "Invalid window handle. (m_hWnd is nullptr)");
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // Configuración de banderas de creación (Modo Debug para desarrollo)
    unsigned int createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Lista de tipos de controladores a probar (Hardware -> Software)
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    unsigned int numDriverTypes = ARRAYSIZE(driverTypes);

    // Niveles de características de DirectX soportados
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    unsigned int numFeatureLevels = ARRAYSIZE(featureLevels);

    // Bucle para intentar crear el dispositivo con el mejor controlador disponible
    for (unsigned int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr,
            driverType,
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &device.m_device,
            &m_featureLevel,
            &deviceContext.m_deviceContext);

        if (SUCCEEDED(hr)) {
            MESSAGE("SwapChain", "init", "Device created successfully.");
            break;
        }
    }

    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to create D3D11 device. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Verificación de niveles de calidad para MSAA
    m_sampleCount = 1; // Configuración actual: Sin multisampleo
    hr = device.m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
        m_sampleCount,
        &m_qualityLevels);
    if (FAILED(hr) || m_qualityLevels == 0) {
        ERROR("SwapChain", "init",
            ("MSAA not supported or invalid quality level. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Configuración de la descripción del Swap Chain
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.BufferCount = 1;                                 // Un solo buffer de vuelta
    sd.BufferDesc.Width = window.m_width;
    sd.BufferDesc.Height = window.m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // Formato RGBA estándar
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // Usar buffer como salida de renderizado
    sd.OutputWindow = window.m_hWnd;
    sd.Windowed = TRUE;                                 // Modo ventana
    sd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;         // Modo secuencial para presentación
    sd.SampleDesc.Count = 1;                            // Muestreo simple
    sd.SampleDesc.Quality = 0;

    // Obtener la Factory de DXGI navegando por la jerarquía de interfaces COM
    hr = device.m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgiDevice);
    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to query IDXGIDevice. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    hr = m_dxgiDevice->GetAdapter(&m_dxgiAdapter);
    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to get IDXGIAdapter. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    hr = m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(&m_dxgiFactory));
    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to get IDXGIFactory. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Creación final del Swap Chain a través de la Factory
    hr = m_dxgiFactory->CreateSwapChain(device.m_device, &sd, &m_swapChain);

    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to create swap chain. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Extracción del Back Buffer para vincularlo a las texturas del motor
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) {
        ERROR("SwapChain", "init",
            ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

/**
 * @brief Libera de forma segura todos los recursos de DXGI y el Swap Chain.
 */
void
SwapChain::destroy() {
    if (m_swapChain) {
        SAFE_RELEASE(m_swapChain);
    }
    if (m_dxgiDevice) {
        SAFE_RELEASE(m_dxgiDevice);
    }
    if (m_dxgiAdapter) {
        SAFE_RELEASE(m_dxgiAdapter);
    }
    if (m_dxgiFactory) {
        SAFE_RELEASE(m_dxgiFactory);
    }
}

/**
 * @brief Intercambia los buffers frontal y trasero para mostrar el frame renderizado.
 * * Llama al método Present de la interfaz IDXGISwapChain.
 */
void
SwapChain::present() {
    if (m_swapChain) {
        HRESULT hr = m_swapChain->Present(0, 0);
        if (FAILED(hr)) {
            ERROR("SwapChain", "present",
                ("Failed to present swap chain. HRESULT: " + std::to_string(hr)).c_str());
        }
    }
    else {
        ERROR("SwapChain", "present", "Swap chain is not initialized.");
    }
}