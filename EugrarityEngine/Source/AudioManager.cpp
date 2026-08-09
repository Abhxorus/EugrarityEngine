#include "AudioManager.h"

std::unique_ptr<DirectX::AudioEngine> AudioManager::m_engine = nullptr;

void AudioManager::init() {
    // Inicializa el motor de audio con los parámetros por defecto
    DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
    m_engine = std::make_unique<DirectX::AudioEngine>(eflags);
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