#include "Mesh.h"
#include <fstream>
#include <sstream>
#include <SDL/SDL_log.h>
#include <rapidjson/document.h>
#include "src/Renderer.h"
#include "src/VertexArray.h"
#include "src/Vendor/Math.h"
#include "src/VertexBufferLayout.h"
#include "src/VertexBuffer.h"
#include "src/IndexBuffer.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Load(const std::string &fileName, Renderer *renderer)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("File not found: Mesh %s", fileName.c_str());
        return false;
    }
	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		SDL_Log("Mesh %s is not valid json", fileName.c_str());
		return false;
	}

	int ver = doc["version"].GetInt();

	// Check the version
	if (ver != 1)
	{
		SDL_Log("Mesh %s not version 1", fileName.c_str());
		return false;
	}

	m_shaderName = doc["shader"].GetString();

	// Skip the vertex format/shader for now
	// (This is changed in a later chapter's code)
	size_t vertSize = 8;

	// Load textures
	const rapidjson::Value &textures = doc["textures"];
	if (!textures.IsArray() || textures.Size() < 1)
	{
		SDL_Log("Mesh %s has no textures, there should be at least one", fileName.c_str());
		return false;
	}

	m_specPower = static_cast<float>(doc["specularPower"].GetDouble());

	for (rapidjson::SizeType i = 0; i < textures.Size(); i++)
	{
		// Is this texture already loaded?
		std::string texName = textures[i].GetString();
		Texture *t = renderer->GetTexture(texName);
		if (t == nullptr)
		{
			// Try loading the texture
			t = renderer->GetTexture(texName);
			if (t == nullptr)
			{
				// If it's still null, just use the default texture
				t = renderer->GetTexture("Assets/Default.png");
			}
		}
		m_textures.emplace_back(t);
	}

	// Load in the vertices
	const rapidjson::Value &vertsJson = doc["vertices"];
	if (!vertsJson.IsArray() || vertsJson.Size() < 1)
	{
		SDL_Log("Mesh %s has no vertices", fileName.c_str());
		return false;
	}

	std::vector<float> vertices;
	vertices.reserve(vertsJson.Size() * vertSize);
	m_radius = 0.0f;
	for (rapidjson::SizeType i = 0; i < vertsJson.Size(); i++)
	{
		// For now, just assume we have 8 elements
		const rapidjson::Value &vert = vertsJson[i];
		if (!vert.IsArray() || vert.Size() != 8)
		{
			SDL_Log("Unexpected vertex format for %s", fileName.c_str());
			return false;
		}

		Vector3 pos(vert[0].GetDouble(), vert[1].GetDouble(), vert[2].GetDouble());
		m_radius = Math::Max(m_radius, pos.LengthSq());

		// Add the floats
		for (rapidjson::SizeType i = 0; i < vert.Size(); i++)
		{
			vertices.emplace_back(static_cast<float>(vert[i].GetDouble()));
		}
	}

	// We were computing length squared earlier
	m_radius = Math::Sqrt(m_radius);

	// Load in the indices
	const rapidjson::Value &indJson = doc["indices"];
	if (!indJson.IsArray() || indJson.Size() < 1)
	{
		SDL_Log("Mesh %s has no indices", fileName.c_str());
		return false;
	}

	std::vector<unsigned int> indices;
	indices.reserve(indJson.Size() * 3);
	for (rapidjson::SizeType i = 0; i < indJson.Size(); i++)
	{
		const rapidjson::Value &ind = indJson[i];
		if (!ind.IsArray() || ind.Size() != 3)
		{
			SDL_Log("Invalid indices for %s", fileName.c_str());
			return false;
		}

		indices.emplace_back(ind[0].GetUint());
		indices.emplace_back(ind[1].GetUint());
		indices.emplace_back(ind[2].GetUint());
	}

	// Now create a vertex array
	//m_vertexArray = new VertexArray(vertices.data(), static_cast<unsigned>(vertices.size()) / vertSize,
	//	indices.data(), static_cast<unsigned>(indices.size()));

	VertexBufferLayout layout;
	layout.Push<float>(3); // position
	layout.Push<float>(3); // normal
	layout.Push<float>(2); // texture

	// Create and bind vertex buffer
	m_vertexBuffer = new VertexBuffer(vertices.data(), static_cast<unsigned>(vertices.size())/vertSize * layout.GetStride());
	m_vertexArray = new VertexArray();

	m_vertexArray->AddBuffer(*m_vertexBuffer, layout); // bind vao and add buffer

	m_indexBuffer = new IndexBuffer(indices.data(), indices.size());

	return true;
}

void Mesh::Unload()
{
	delete m_vertexArray;
	m_vertexArray = nullptr;

	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}

Texture *Mesh::GetTexture(size_t index)
{
	if (index < m_textures.size())
	{
		return m_textures[index];
	}
	else
	{
		return nullptr;
	}
}
