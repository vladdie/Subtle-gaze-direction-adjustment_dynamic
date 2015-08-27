#ifndef MESH_H
#define	MESH_H

#include <map>
#include <vector>
#include <GL/glew.h>
#include <Importer.hpp>      // C++ importer interface
#include <scene.h>       // Output data structure
#include <postprocess.h> // Post processing flags

#include "ogldev_util.h"
#include "ogldev_math_3d.h"
#include "TextureMesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;
	//Vector3f m_tangent;

    Vertex() {}

	 Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal)//, const Vector3f& Tangent)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
       // m_tangent = Tangent;
                
    }
};

struct VertexWithTangent
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;
	Vector3f m_tangent;

    VertexWithTangent() {}

	 VertexWithTangent(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal, const Vector3f& Tangent)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
        m_tangent = Tangent;
                
    }
};


class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render();

	bool attractor;
	bool fadeIn;


	glm::vec3 centerPos;
	glm::vec2 screenPos;

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void Init(const std::vector<Vertex>& Vertices,
                  const std::vector<unsigned int>& Indices);


        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Texture*> m_Textures;
	std::vector<Texture*> m_NormalTextures;
};


#endif	/* MESH_H */


