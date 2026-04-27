#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"

class Device;
class DeviceContext;

/**
 * @class Skybox
 * @brief Clase encargada de la creación y renderizado de un cubo de entorno (Skybox).
 * * Gestiona su propio conjunto de estados de renderizado (shading, texturizado y profundidad)
 * para asegurar que el fondo se dibuje correctamente en relación con la cámara y el resto de la escena.
 */
class
	Skybox {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Skybox() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Skybox() = default;

	/**
	 * @brief Inicializa los recursos necesarios para el Skybox.
	 * * Crea el modelo de cubo, configura los Shaders específicos de Skybox, inicializa
	 * los buffers constantes y establece los estados de rasterización y profundidad necesarios.
	 * * @param device Referencia al dispositivo de hardware para la creación de recursos.
	 * @param deviceContext Puntero al contexto del dispositivo para operaciones iniciales.
	 * @param cubemap Referencia a la textura de tipo Cubemap que se proyectará en el Skybox.
	 * @return HRESULT S_OK si la inicialización fue exitosa, código de error en caso contrario.
	 */
	HRESULT
		init(Device& device, DeviceContext* deviceContext, Texture& cubemap);

	/**
	 * @brief Actualiza la lógica del Skybox.
	 * * Actualmente no realiza ninguna operación por frame, ya que su posición suele
	 * actualizarse directamente en el renderizado respecto a la cámara.
	 */
	void
		update(DeviceContext& deviceContext, Camera& camera);

	/**
	 * @brief Renderiza el Skybox en la escena.
	 * * Configura el pipeline con los estados de profundidad y rasterización específicos del Skybox,
	 * vincula la textura Cubemap y utiliza la matriz de vista de la cámara (sin traslación)
	 * para que el fondo parezca estar infinitamente lejos.
	 * * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
	 * @param camera Referencia a la cámara activa para obtener las matrices de View y Projection.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera los recursos asociados al Skybox.
	 * * Limpia buffers, estados de pipeline y referencias al modelo de cubo.
	 */
	void
		destroy() {}

private:
	ShaderProgram m_shaderProgram;      ///< Shader especializado para el renderizado de Cubemaps.
	Buffer m_constantBuffer;           ///< Buffer constante para enviar matrices (WVP) a la GPU.
	SamplerState m_samplerState;       ///< Estado de muestreo para la textura del cielo.
	RasterizerState m_rasterizerState; ///< Estado de rasterización (usualmente configurado para Front-face Culling).
	DepthStencilState m_depthStencilState; ///< Estado de profundidad (usualmente Less-Equal para dibujar al fondo).
	Texture m_skyboxTexture;           ///< Referencia a la textura Cubemap.
	Model3D* m_cubeModel = nullptr;    ///< Puntero al modelo geométrico del cubo.
	EU::TSharedPointer<Actor> m_skybox; ///< Actor que representa la entidad del Skybox en el ECS.

};