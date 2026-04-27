#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum ModelType
 * @brief Define los formatos de modelo 3D soportados por el motor.
 */
enum ModelType {
	OBJ, ///< Formato Wavefront OBJ.
	FBX  ///< Formato Autodesk FBX.
};

/**
 * @class Model3D
 * @brief Clase encargada de la carga, gestión y procesamiento de recursos de mallas 3D.
 * * Esta clase hereda de IResource y permite cargar modelos tanto de archivos externos
 * (usando el SDK de FBX) como de datos en memoria (para primitivas como el Skybox).
 */
class Model3D : public IResource {
public:
	/**
	 * @brief Constructor para cargar un modelo desde un archivo.
	 * @param name Nombre o ruta del recurso.
	 * @param modelType Tipo de formato del archivo (OBJ o FBX).
	 */
	Model3D(const std::string& name, ModelType modelType)
		: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
		load(name);
	}

	/**
	 * @brief Constructor especializado para la creación de Skyboxes.
	 * @param name Nombre del recurso.
	 * @param vertices Arreglo de vértices de tipo SkyboxVertex.
	 * @param indices Arreglo de índices para la construcción de caras.
	 */
	Model3D(const std::string& name,
		const SkyboxVertex vertices[],
		const unsigned int indices[]) : IResource(name) {
		MeshComponent mesh;
		mesh.m_skyVertex.assign(vertices, vertices + 8);
		mesh.m_index.assign(indices, indices + 36);
		mesh.m_numIndex = mesh.m_index.size();
		SetType(ResourceType::Model3D);
		m_meshes.push_back(mesh);
	}

	/**
	 * @brief Destructor por defecto.
	 */
	~Model3D() = default;

	/**
	 * @brief Carga el modelo desde la ruta especificada.
	 * @param path Ruta del archivo en el disco.
	 * @return true si la carga fue exitosa, false en caso contrario.
	 */
	bool load(const std::string& path) override;

	/**
	 * @brief Inicializa los componentes internos del modelo.
	 * @return true si la inicialización fue correcta.
	 */
	bool init() override;

	/**
	 * @brief Libera la memoria y recursos asociados al modelo.
	 */
	void unload() override;

	/**
	 * @brief Obtiene el tamaño aproximado del modelo en memoria.
	 * @return Tamaño en bytes.
	 */
	size_t getSizeInBytes() const override;

	/**
	 * @brief Devuelve la lista de mallas (MeshComponents) que componen el modelo.
	 * @return Referencia constante al vector de mallas.
	 */
	const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER*/

	/**
	 * @brief Inicializa el SDK Manager de FBX necesario para la importación.
	 * @return true si el manager se creó correctamente.
	 */
	bool InitializeFBXManager();

	/**
	 * @brief Importa y procesa un archivo de formato FBX.
	 * @param filePath Ruta completa al archivo .fbx.
	 * @return Vector con los componentes de malla extraídos.
	 */
	std::vector<MeshComponent> LoadFBXModel(const std::string& filePath);

	/**
	 * @brief Recorre de forma recursiva los nodos del escenario FBX.
	 * @param node Puntero al nodo actual del SDK de FBX.
	 */
	void ProcessFBXNode(FbxNode* node);

	/**
	 * @brief Extrae la información geométrica (vértices, normales, UVs) de un nodo FBX.
	 * @param node Nodo que contiene el atributo de malla.
	 */
	void ProcessFBXMesh(FbxNode* node);

	/**
	 * @brief Extrae la información de materiales y nombres de texturas asociadas.
	 * @param material Puntero al material de superficie de FBX.
	 */
	void ProcessFBXMaterials(FbxSurfaceMaterial* material);

	/**
	 * @brief Obtiene los nombres de los archivos de textura encontrados en el modelo.
	 * @return Vector de strings con las rutas/nombres de texturas.
	 */
	std::vector<std::string> GetTextureFileNames() const { return textureFileNames; }

private:
	FbxManager* lSdkManager; ///< Puntero al gestor de memoria del SDK FBX.
	FbxScene* lScene;       ///< Puntero a la escena cargada en memoria.
	std::vector<std::string> textureFileNames; ///< Lista interna de nombres de texturas.

public:
	ModelType m_modelType; ///< Tipo de modelo (formato).
	std::vector<MeshComponent> m_meshes; ///< Contenedor de las mallas procesadas del modelo.
};