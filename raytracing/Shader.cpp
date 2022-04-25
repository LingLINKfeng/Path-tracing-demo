#include<iostream>
#include<string>
#include <fstream>
#include<sstream>
#include "Shader.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Shader::Shader(const std::string & filepath) :
	m_FilePath(filepath), m_RendererID(0)
{
	ShaderProgramSource source = ParseShader(filepath);
	m_RendererID = CreatShader(source.VertexSource, source.FragmenShader);
}

Shader::~Shader()
{

}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	std::string line;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos) //返回一个名为 string::npos 的特殊值，说明查找没有匹配
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	//std::cout << ss[1].str() << std::endl;
	return{ ss[0].str(),ss[1].str() };
}

unsigned int Shader::CreatShader(const std::string & vertaxShader, const std::string & fragmentShader)
{
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertaxShader);
	GLuint fg = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(shaderProgramID, vs);
	glAttachShader(shaderProgramID, fg);

	int Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	glLinkProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

GLuint Shader::CompileShader(GLuint type, const std::string & source)
{
	GLuint id = glCreateShader(type);
	const char*src = source.c_str();
	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);

	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(id, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", id, InfoLog);
		exit(1);
	}
	return id;
}


void Shader::SetUniformf(const std::string & name, float v0)
{
	glUniform1f(GetUniformLocation(name), v0);
}



void Shader::SetUniform1i(const std::string & name, int v0)
{
	glUniform1i(GetUniformLocation(name), v0);
}

void Shader::SetUniformVec3(const std::string & name, glm::vec3 v0)
{
	glUniform3f(GetUniformLocation(name), v0.x, v0.y, v0.z);
}

void Shader::SetUniform3f(const std::string & name, float v0, float v1, float v2)
{
	glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
	glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}
void Shader::SetUniformMat4f(const std::string & name, const glm::mat4 & matrix)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

int Shader::GetUniformLocation(const std::string&name) {
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
		return m_UniformLocationCache[name];
	}
	int location = glGetUniformLocation(m_RendererID, name.c_str());

	if (location == -1)
		std::cout << "Warning uniform" << name << "does ont exist" << std::endl;
	else
		m_UniformLocationCache[name] = location;
	return location;
}
void Shader::Bind() const
{
	glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
	glUseProgram(0);
}