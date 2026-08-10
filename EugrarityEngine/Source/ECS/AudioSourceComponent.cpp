/**
 * @file AudioSourceComponent.cpp
 * @brief Implementa AudioSourceComponent (subsistema ECS) usando DirectXTK Audio.
 */
#include "ECS/AudioSourceComponent.h"
#include "AudioManager.h"
#include "Prerequisites.h"
#include <Windows.h>
#include <exception>

namespace {
	// DirectXTK::SoundEffect pide el path como wchar_t*; el resto del motor
	// maneja rutas como std::string, asi que convertimos aqui.
	std::wstring ToWideString(const std::string& narrow) {
		if (narrow.empty()) {
			return std::wstring();
		}
		int required = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
		if (required <= 0) {
			return std::wstring();
		}
		std::wstring wide(static_cast<size_t>(required) - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, wide.data(), required);
		return wide;
	}
}

AudioSourceComponent::AudioSourceComponent()
	: Component(ComponentType::AUDIO) {
}

AudioSourceComponent::~AudioSourceComponent() {
	destroy();
}

void
AudioSourceComponent::update(float deltaTime) {
	(void)deltaTime;
	// Nada que actualizar por frame por ahora (el motor de audio en si se
	// actualiza una sola vez por frame de forma centralizada en
	// AudioManager::update(), llamado desde BaseApp::update()). Este hook
	// queda listo para cosas como atenuacion por distancia/3D audio si
	// mas adelante enganchas AudioSourceComponent al Transform del actor.
}

void
AudioSourceComponent::destroy() {
	if (m_soundInstance) {
		m_soundInstance->Stop();
		m_soundInstance.reset();
	}
	m_soundEffect.reset();
}

bool
AudioSourceComponent::loadSound(const std::string& filePath) {
	DirectX::AudioEngine* engine = AudioManager::getEngine();
	if (!engine) {
		ERROR("AudioSourceComponent", "loadSound", "AudioManager engine is null. Llama a AudioManager::init() antes de cargar sonidos.");
		return false;
	}

	// Soltar cualquier instancia/efecto previo antes de cargar uno nuevo.
	if (m_soundInstance) {
		m_soundInstance->Stop();
		m_soundInstance.reset();
	}
	m_soundEffect.reset();

	std::wstring widePath = ToWideString(filePath);
	if (widePath.empty()) {
		ERROR("AudioSourceComponent", "loadSound", ("Ruta de archivo invalida: " + filePath).c_str());
		return false;
	}

	try {
		m_soundEffect = std::make_unique<DirectX::SoundEffect>(engine, widePath.c_str());
	}
	catch (const std::exception& e) {
		ERROR("AudioSourceComponent", "loadSound",
			("No se pudo cargar '" + filePath + "': " + e.what()).c_str());
		m_soundEffect.reset();
		return false;
	}

	m_soundInstance = m_soundEffect->CreateInstance();
	if (!m_soundInstance) {
		ERROR("AudioSourceComponent", "loadSound", ("CreateInstance devolvio null para: " + filePath).c_str());
		m_soundEffect.reset();
		return false;
	}

	m_filePath = filePath;
	m_soundInstance->SetVolume(m_volume);
	MESSAGE("AudioSourceComponent", "loadSound", ("Sonido cargado: " + filePath).c_str());
	return true;
}

void
AudioSourceComponent::play() {
	if (!m_soundInstance) {
		ERROR("AudioSourceComponent", "play", "No hay ningun sonido cargado (llama a loadSound primero).");
		return;
	}
	m_soundInstance->Play(m_isLooping);
}

void
AudioSourceComponent::stop() {
	if (m_soundInstance) {
		m_soundInstance->Stop();
	}
}

void
AudioSourceComponent::pause() {
	if (m_soundInstance) {
		m_soundInstance->Pause();
	}
}

void
AudioSourceComponent::resume() {
	if (m_soundInstance) {
		m_soundInstance->Resume();
	}
}

bool
AudioSourceComponent::isPlaying() const {
	return m_soundInstance && m_soundInstance->GetState() == DirectX::SoundState::PLAYING;
}

void
AudioSourceComponent::setVolume(float volume) {
	m_volume = volume;
	if (m_soundInstance) {
		m_soundInstance->SetVolume(m_volume);
	}
}

void
AudioSourceComponent::setLooping(bool loop) {
	m_isLooping = loop;
	// DirectXTK no permite cambiar el loop de una instancia ya reproduciendo;
	// el flag se aplica en la siguiente llamada a play(m_isLooping).
}

void
AudioSourceComponent::setPitch(float pitch) {
	if (m_soundInstance) {
		// DirectXTK espera el pitch en semitonos normalizados, rango [-1, 1].
		m_soundInstance->SetPitch(pitch);
	}
}
