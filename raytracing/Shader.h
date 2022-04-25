#pragma once
#pragma once
#include<GL/glew.h>
#include<string>
#include<unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
struct  ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmenShader;
};
class Shader {
public:
	void SetUniformf(const std::string& name, float v0);
	void SetUniform1i(const std::string& name, int v0);
	void SetUniformVec3(const std::string& name, glm::vec3 v0);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string & name, const glm::mat4 & matrix);
	std::string m_FilePath;
	Shader(const std::string& filepath);
	~Shader();
	unsigned int Getid() { return m_RendererID; };
	void Bind()const;
	void Unbind()const;
	unsigned int m_RendererID;
private:
	std::unordered_map<std::string, int> m_UniformLocationCache;
	int GetUniformLocation(const std::string& name);
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CreatShader(const std::string& vertaxShader, const std::string& fragmentShader);
	GLuint CompileShader(GLuint type, const std::string&source);

};