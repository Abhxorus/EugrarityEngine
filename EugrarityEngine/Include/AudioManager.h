#pragma once
#include "Audio.h"
#include <memory>

class AudioManager {
public:
    static void init();
    static void update();
    static void destroy();
    static DirectX::AudioEngine* getEngine();

private:
    static std::unique_ptr<DirectX::AudioEngine> m_engine;
};