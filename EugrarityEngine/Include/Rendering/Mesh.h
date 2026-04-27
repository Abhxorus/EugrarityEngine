/**
 * @file Mesh.h
 * @brief Declara la API de Mesh dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

 /**
  * @struct Submesh
  * @brief Describe una porcion renderizable de una malla con sus buffers asociados.
  */
struct
	Submesh {
	Buffer vertexBuffer;          ///< Buffer de vertices de la submalla.
	Buffer indexBuffer;           ///< Buffer de indices de la submalla.
	unsigned int indexCount = 0;  ///< Numero de indices a dibujar.
	unsigned int startIndex = 0;  ///< Offset inicial dentro del index buffer.
	unsigned int materialSlot = 0;///< Slot de material esperado por el renderer.
};

/**
 * @class Mesh
 * @brief Agrupa una coleccion de submallas listas para ser renderizadas.
 */
class
	Mesh {
public:
	std::vector<Submesh>& getSubmeshes() { return m_submeshes; }
	const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }

	/**
	 * @brief Libera todos los buffers asociados a las submallas.
	 */
	void
		destroy() {
		for (Submesh& submesh : m_submeshes) {
			submesh.vertexBuffer.destroy();
			submesh.indexBuffer.destroy();
		}
		m_submeshes.clear();
	}

private:
	std::vector<Submesh> m_submeshes;
};