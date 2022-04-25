#pragma once
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include"Shader.h"

#include"TBTool.h"
#include<glm/gtx/vector_angle.hpp>
#include<time.h>
#define M_PI 3.141592653589793f
struct Raytest {
	glm::vec3 ori;
	glm::vec3 dir;
	glm::vec3 invDir;
	glm::vec3 dirIsNeg;
};
struct Triangle {
	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 aNormal;
	glm::vec3 bNormal;
	glm::vec3 cNormal;
	glm::vec3 color;
	bool isLight;
};
struct Inter {
	glm::vec3 normal;
	glm::vec3 position;
	float distanceI;
	bool isIntersect;
	glm::vec3 color;
	bool isLight;
	float pdf;
};
class Ray
{
	unsigned int VAO;
	unsigned int VBO, VBO1;
	GLuint TBO, buf;
	float* vertices;
	int width;
	int height;
	int bvhSize;
	int triSize;
	//Camera camera;
	Shader shader;
	TBOTool texture1;
	TBOTool texture2;
	TBOTool texture3;
	float *randFloat;
	float camera_fov;
	glm::vec3 camera_position;
	

public:
	float*  interList;
	int m = 0;
	inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }
	Ray(unsigned int width, unsigned int height,
		Shader shader, float* bvh, float* tri, int bvhSize, int triSize, float fov, glm::vec3 position)
		:height(height), width(width), shader(shader), bvhSize(bvhSize), triSize(triSize), camera_fov(fov),camera_position(position)
	{
		randFloat = new float[width * height * 4 * 7];
		vertices = new float[width * height * 5];
		float scale = tan(deg2rad(camera_fov * 0.5));
		float imageAspectRatio = width / (float)height;
		for (uint32_t j = 0; j < height; ++j) {
			for (uint32_t i = 0; i < width; ++i) {
				// generate primary ray direction
				float x = (2 * (i + 0.5) / (float)width - 1) *
					imageAspectRatio * scale;
				float y = (2 * (j + 0.5) / (float)height - 1) * scale;
				glm::vec3 dir = normalize(glm::vec3(-x, y, 1));
				for (int t = 0; t < 3; t++) {
					vertices[m + t] = dir[t];
				}
				vertices[m + 3] = (float)i;
				vertices[m + 4] = (float)j;
				m += 5;
			}
		}

		setUpTray(bvh, tri);
	}
	void Random() {
		srand(time(0));  //设置时间种子
		for (int i = 0; i < width * height * 4 * 7; i++) {
			randFloat[i] = (float)rand() / RAND_MAX;//生成区间r~l的随机数 
		}
		texture3 = TBOTool(randFloat, height*width * 4 * 7, GL_R32F);
	}
	void setUpTray(float* bvh, float* tri) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		texture1 = TBOTool(tri, triSize, GL_RGB32F);
		texture2 = TBOTool(bvh, bvhSize, GL_R32F);
		Random();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, height*width * sizeof(float) * 5, vertices, GL_STATIC_DRAW);
		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	void Draw(Shader &shader) {
		// draw mesh
		/*texture2.bindTexture(0);
		shader.SetUniform1i("bvh", 0);*/
		texture1.bindTexture(1);
		shader.SetUniform1i("tria", 1);
		texture3.bindTexture(2);		
		Random();
		shader.SetUniform1i("rand", 2);

		glBindVertexArray(VAO);
		glUniform2f(glGetUniformLocation(shader.m_RendererID, "screen"), (float)width, (float)height); // 手动设置;
		glUniform3f(glGetUniformLocation(shader.m_RendererID, "eye"), camera_position.x, camera_position.y, camera_position.z);
		glDrawArrays(GL_POINTS, 0, height*width);
		glBindVertexArray(0);
	}
	void CPUhitTest(float* tri) {
		interList=new float[m / 5];
		Triangle testTri;
		Raytest raytemp;
		Inter inter;
		glm::vec3 S1, S2, S;
		glm::vec3 E1, E2;
		float det, u, v, t_tmp = 0;
		for (int j = 0; j < m; j+=5) {
			raytemp.ori = camera_position;
			raytemp.dir = glm::vec3(vertices[j], vertices[j  + 1], vertices[j + 2]);
			inter.isIntersect = false;
			for (int i = 0; i < 36; i++)
			{
				testTri.a = glm::vec3(tri[i * 24], tri[i * 24 + 1], tri[i * 24 + 2]);
				testTri.b = glm::vec3(tri[i * 24 + 3], tri[i * 24 + 4], tri[i * 24 + 5]);
				testTri.c = glm::vec3(tri[i * 24 + 6], tri[i * 24 + 7], tri[i * 24 + 8]);
				testTri.aNormal = glm::vec3(tri[i * 24 + 9], tri[i * 24 + 10], tri[i * 24 + 11]);
				testTri.bNormal = glm::vec3(tri[i * 24 + 12], tri[i * 24 + 13], tri[i * 24 + 14]);
				testTri.cNormal = glm::vec3(tri[i * 24 + 15], tri[i * 24 + 16], tri[i * 24 + 17]);
				testTri.color = glm::vec3(tri[i * 24 + 18], tri[i * 24 + 19], tri[i * 24 + 20]);
				testTri.isLight = (int)tri[i * 24 + 21];
				//std::cout << testTri.a.x <<","<< testTri.a.y <<","<< testTri.a.z << std::endl;
				glm::vec3 normal = cross(testTri.c - testTri.a, testTri.b - testTri.a);
				E1 = testTri.b - testTri.a;
				E2 = testTri.c - testTri.a;
				S = raytemp.ori - testTri.a;
				S1 = glm::cross(-raytemp.dir, E2);
				S2 = glm::cross(S, E1);
				det = dot(E1, S1);
				if (dot(raytemp.dir, normal) >=0) {
					continue;
				}
				if (abs(dot(normalize(E1), normalize(S1))) < 0.01)
					continue;
				u = dot(S, S1) / det;
				v = dot(-raytemp.dir, S2) / det;
				t_tmp = -dot(E2, S2) / det;
				if (u < 0 || u > 1)
					continue;
				if (v < 0 || u + v > 1)
					continue;
				if (t_tmp < 0)
					continue;
				inter.isIntersect = true;
				break;
			}
			if (inter.isIntersect == true)
				interList[j/5]=1;
			else
				interList[j/5]=0;
			
		}
	}

};