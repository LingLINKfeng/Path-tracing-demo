#pragma once
#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "std_image/stb_image.h"
#include "Shader.h"
#include "Model.h"
#include <iostream>

#include "ImageAverage.h"
#include <opencv2/opencv.hpp>
#include <cstdlib> 
#include <ctime>
#include <stdlib.h>

#include "TBTool.h"
#include "ray.h"
#include "BVH.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -100.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.5f;
float yaw = 90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float fov = 40.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

int main()
{
	glfwInit();
	if (!glfwInit())
		return -1;

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (glewInit() != GLEW_OK)
		std::cout << "error!" << std::endl;
	stbi_set_flip_vertically_on_load(true);
	glEnable(GL_DEPTH_TEST);

	Shader testHit("Src/raytracing/hit.shader");

	Model light("Src/Model/cornell/light.obj");
	Model boxlight("Src/Model/cornell/box.obj");
	Model leftwall("Src/Model/cornell/leftwall.obj");
	Model rightwall("Src/Model/cornell/rightwall.obj");
	
	Model floor("Src/Model/cornell/floor.obj");

	Model TBD_faces("Src/Model/cornell/TBD_faces.obj");
	Model tallbox("Src/Model/cornell/tallbox2.obj");
	Model smallbox("Src/Model/cornell/smallbox2.obj");


	vector<triangle> triangleList;
	light.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f),true);

	leftwall.obj2BVH(triangleList, glm::vec3(0.63f, 0.065f, 0.05f));
	rightwall.obj2BVH(triangleList, glm::vec3(0.14f, 0.45f, 0.091f));
	floor.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));

	TBD_faces.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	tallbox.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	smallbox.obj2BVH(triangleList, glm::vec3(0.725f, 0.71f, 0.68f));
	
	BVH* firstBVH = recursionBVH(triangleList, 0, triangleList.size(), xAxis);
	float* bvh = NULL;
	float* tri = NULL;
	int bvhSize, triSize;
	createTexture(firstBVH, bvh, tri, bvhSize, triSize);
	Ray *myRay = new Ray(SCR_WIDTH, SCR_HEIGHT, testHit, bvh, tri, bvhSize, triSize,fov,cameraPos);
	int i = 0;
	int averageframes = 30;
	frame2Real f = frame2Real(SCR_WIDTH, SCR_HEIGHT, averageframes);
	int nums;
	while (!glfwWindowShouldClose(window))
	{
		glfwSetCursorPosCallback(window, mouse_callback);
		processInput(window);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		testHit.Bind();
		testHit.SetUniform1i("triangleNum", 36);
		myRay->Draw(testHit);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (i != 0) {
			std::cout << i << std::endl;
			f.frame2local();
		}
		if (i++ == averageframes) {
			break;
		}
	}
	f.saveFrameBuff("hello.bmp", SCR_WIDTH, SCR_HEIGHT);
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		cameraPos += cameraUp * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		cameraPos -= cameraUp * cameraSpeed;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float xoffset;
	float yoffset;
	glm::vec3 front;
	xoffset = xpos - lastX;
	yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}
