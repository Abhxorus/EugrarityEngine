/**
 * @file AudioSourceComponent.h
 * @brief Componente para emitir sonido usando DirectXTK.
 */
#pragma once
#include "Component.h" // Verifica que esta sea la ruta correcta a tu clase base Component
#include "Audio.h"     // El header de DirectXTK 
#include <string>
#include <memory>

class AudioSourceComponent : public Component {
public:
    AudioSourceComponent();
    ~AudioSourceComponent() override;

    // BUGFIX: Component es una clase abstracta (init/update/render/destroy
    // son virtuales puros). AudioSourceComponent no los sobreescribia, asi
    // que seguia siendo abstracta y `EU::MakeShared<AudioSourceComponent>()`
    // en BaseApp.cpp no podia compilar ("cannot instantiate abstract class").
    void init() override {}
    void update(float deltaTime) override;
    void render(DeviceContext& deviceContext) override {}
    void destroy() override;

    // Controles de reproduccion
    void play();
    void stop();
    void pause();
    void resume();
    bool isPlaying() const;

    // Propiedades del sonido
    void setVolume(float volume);
    void setLooping(bool loop);
    void setPitch(float pitch);

    // Carga de archivo (.wav)
    bool loadSound(const std::string& filePath);

private:
    // Objetos de DirectXTK
    // SoundEffect guarda la data del audio en memoria
    std::unique_ptr<DirectX::SoundEffect> m_soundEffect;
    // SoundEffectInstance controla la reproduccion individual
    std::unique_ptr<DirectX::SoundEffectInstance> m_soundInstance;

    std::string m_filePath;
    float m_volume = 1.0f;
    bool m_isLooping = false;
};