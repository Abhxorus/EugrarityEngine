#include "AudioManager.h"
#include "Prerequisites.h"
#include <string>

std::unique_ptr<DirectX::AudioEngine> AudioManager::m_engine = nullptr;

void AudioManager::init() {
    // BUGFIX: el constructor de DirectX::AudioEngine puede lanzar excepcion
    // si no hay ningun dispositivo de audio disponible (comun en maquinas
    // virtuales, servidores CI, o si el usuario no tiene bocinas/audifonos
    // conectados). Sin este try/catch, esa excepcion tumbaba toda la
    // aplicacion en el arranque antes de llegar siquiera a crear la ventana.
    try {
        DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#if defined(_DEBUG)
        eflags = eflags | DirectX::AudioEngine_Debug;
#endif
        m_engine = std::make_unique<DirectX::AudioEngine>(eflags);
    }
    catch (const std::exception& e) {
        ERROR("AudioManager", "init", (std::string("No se pudo inicializar el motor de audio: ") + e.what()).c_str());
        m_engine.reset();
    }
}

void AudioManager::update() {
    if (m_engine && !m_engine->Update()) {
        // Entra aquí si ocurre un error crítico, por ejemplo, si desconectas los audífonos
        if (m_engine->IsCriticalError()) {
            // Aquí en el futuro se podría intentar reiniciar el motor
        }
    }
}

void AudioManager::destroy() {
    if (m_engine) {
        m_engine->Suspend();
    }
    m_engine.reset();
}

DirectX::AudioEngine* AudioManager::getEngine() {
    return m_engine.get();
}