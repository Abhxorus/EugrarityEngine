/**
 * @file RenderScene.h
 * @brief Declara la API de RenderScene dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Contenedor temporal con los elementos visibles de un frame.
 *
 * `RenderScene` funciona como estructura intermedia entre el `SceneGraph` y el
 * renderer. Agrupa objetos por tipo de cola, luces direccionales y skybox activo.
 */
class
	RenderScene {
public:
	/**
	 * @brief Limpia todas las colecciones para preparar un nuevo frame.
	 */
	void clear();

public:
	std::vector<RenderObject> opaqueObjects;       ///< Objetos opacos listos para renderizar.
	std::vector<RenderObject> transparentObjects;  ///< Objetos transparentes ordenables por distancia.
	std::vector<LightData> directionalLights;      ///< Luces direccionales activas en la escena.
	Skybox* skybox = nullptr;                      ///< Skybox activo para el frame actual.
};