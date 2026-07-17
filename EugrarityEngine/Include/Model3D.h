/**
 * @file Model3D.h
 * @brief Declara la API de Model3D dentro del subsistema Core.
 * @ingroup core
 */
#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

enum
	ModelType {
	OBJ,
	FBX
};

class
	Model3D : public IResource {
public:
	Model3D(const std::string& name, ModelType modelType)
		: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
	}

	Model3D(const std::string& name,
		const SkyboxVertex vertices[],
		const unsigned int indices[]) : IResource(name)
	{
		MeshComponent mesh;

		// Llenamos m_vertex con SimpleVertex para que Buffer::init no falle
		mesh.m_vertex.resize(8);
		for (int i = 0; i < 8; ++i) {
			mesh.m_vertex[i].Position = EU::Vector3(vertices[i].x, vertices[i].y, vertices[i].z);
			// Rellenamos el resto con ceros porque el Skybox no los usa
			mesh.m_vertex[i].Normal = EU::Vector3(0.0f, 0.0f, 0.0f);
			mesh.m_vertex[i].Tangent = EU::Vector3(0.0f, 0.0f, 0.0f);
			mesh.m_vertex[i].Bitangent = EU::Vector3(0.0f, 0.0f, 0.0f);
			mesh.m_vertex[i].TextureCoordinate = EU::Vector2(0.0f, 0.0f);
		}

		mesh.m_index.assign(indices, indices + 36);
		mesh.m_numVertex = 8;
		mesh.m_numIndex = 36;

		m_meshes.push_back(mesh);
	}

	~Model3D() override;

	bool
		load(const std::string& path) override;

	bool
		init() override;

	void
		unload() override;

	size_t
		getSizeInBytes() const override;

	const std::vector<MeshComponent>&
		GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER*/
	bool
		InitializeFBXManager();

	std::vector<MeshComponent>
		LoadFBXModel(const std::string& filePath);

	std::vector<MeshComponent>
		LoadOBJModel(const std::string& filePath);

	void
		ProcessFBXNode(FbxNode* node);

	void
		ProcessFBXMesh(FbxNode* node);

	void
		ProcessFBXMaterials(FbxSurfaceMaterial* material);

	std::vector<std::string>
		GetTextureFileNames() const { return textureFileNames; }

private:
	std::string GetBinaryCachePath() const;
	bool IsBinaryCacheUpToDate(const std::string& sourcePath, const std::string& cachePath) const;
	bool LoadBinaryCache(const std::string& cachePath);
	bool SaveBinaryCache(const std::string& cachePath) const;

private:
	FbxManager* lSdkManager;
	FbxScene* lScene;
	FbxAMatrix m_fbxModelRootInverse;
	std::vector<std::string> textureFileNames;
public:
	ModelType m_modelType;
	std::vector<MeshComponent> m_meshes;
};