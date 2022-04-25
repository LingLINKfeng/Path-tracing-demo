#pragma once
#include"Mesh.h"
#include"std_image/stb_image.h"
using namespace std;
enum xyz { xAxis, yAxis, zAxis };
class triangle {
public:
	Vertex a, b, c;
	bool isLight;
	glm::vec3 color;
	triangle(Vertex a1, Vertex b1, Vertex c1, glm::vec3 myColor = glm::vec3(0.0f, 0.0f, 0.0f), bool islight = false)
	{
		a = a1; b = b1; c = c1; color = myColor; isLight = islight;
	}
	triangle() {}
	glm::vec3 minTriangle() const
	{
		return glm::vec3(min(min(a.Position.x, b.Position.x), c.Position.x), min(min(a.Position.y, b.Position.y), c.Position.y), min(min(a.Position.z, b.Position.z), c.Position.z));
	}
	glm::vec3 maxTriangle() const
	{
		return glm::vec3(max(max(a.Position.x, b.Position.x), c.Position.x), max(max(a.Position.y, b.Position.y), c.Position.y), max(max(a.Position.z, b.Position.z), c.Position.z));
	}
	glm::vec3 maxVec3(glm::vec3 b)
	{
		return glm::vec3(max(this->maxTriangle().x, b.x), max(this->maxTriangle().y, b.y), max(this->maxTriangle().z, b.z));
	}
	glm::vec3 minVec3(glm::vec3 b)
	{
		return glm::vec3(min(this->minTriangle().x, b.x), min(this->minTriangle().y, b.y), min(this->minTriangle().z, b.z));
	}
};
class Model
{
public:
	Model(std::string const &path) {
		loadModel(path);
	}
	std::vector<MeshTexture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<glm::vec3> test_vertices;
	std::vector<GLuint> test_indices;

	void Draw(Shader &shader);
	void obj2BVH(vector<triangle> &trangleList, glm::vec3 color, bool islight = false)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh temp = meshes[i];
			unsigned int index = 0;//indices order
			unsigned int index0 = 0;
			while (index < temp.indices.size()) {
				test_vertices.emplace_back(temp.vertices[temp.indices[index0++]].Position);
				test_vertices.emplace_back(temp.vertices[temp.indices[index0++]].Position);
				test_vertices.emplace_back(temp.vertices[temp.indices[index0++]].Position);
				trangleList.push_back(triangle(temp.vertices[temp.indices[index++]],
					temp.vertices[temp.indices[index++]], temp.vertices[temp.indices[index++]], color, islight));
			}
		}
		
	}

private:
	void loadModel(std::string const &path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		std::string typeName);

};
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
