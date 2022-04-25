#pragma once
#include<GL/glew.h>
#include <GLFW/glfw3.h>
#include<glm/gtx/vector_angle.hpp>
#include<iostream>
class TBOTool {
public:
	unsigned int Id;
	TBOTool() {}
	TBOTool(float* img, int size, GLenum style) {
		// ���ɡ��󶨺ͳ�ʼ������������ð󶨵�Ϊ GL_TEXTURE_BUFFER�����������õ����������� 1MB
		unsigned int buf;
		glGenBuffers(1, &buf);
		glBindBuffer(GL_TEXTURE_BUFFER, buf);
		glBufferData(GL_TEXTURE_BUFFER, size * sizeof(float),
			img, GL_STATIC_DRAW);
		glCreateTextures(GL_TEXTURE_BUFFER, 1, &Id);
		
		switch (style) {
		case GL_RGB32F:
			glTextureBuffer(Id, GL_RGB32F, buf);
			
			break; 
		case GL_R32F:
			glTextureBuffer(Id, GL_R32F, buf);
			break; 
		}
		//glGenerateMipmap(GL_TEXTURE_BUFFER);
		//std::cout << Id << std::endl;
	}
	void bindTexture(int num) {
		switch (num) {
		case 0:
			glActiveTexture(GL_TEXTURE0);
			break;
		case 1:
			glActiveTexture(GL_TEXTURE1);
			break;
		case 2:
			glActiveTexture(GL_TEXTURE2);
			break;
		case 3:
			glActiveTexture(GL_TEXTURE3);
			break;
		case 4:
			glActiveTexture(GL_TEXTURE4);
			break;
		case 5:
			glActiveTexture(GL_TEXTURE5);
			break;
		case 6:
			glActiveTexture(GL_TEXTURE6);
			break;
		case 7:
			glActiveTexture(GL_TEXTURE7);
			break;
		default:
			std::cout << "�����֧��0~7" << std::endl;
		}
		glBindTexture(GL_TEXTURE_BUFFER, Id);
	}
	void SetWrap(GLint pname) {
		if (pname == GL_REPEAT || pname == GL_MIRRORED_REPEAT || pname == GL_CLAMP_TO_EDGE || pname == GL_CLAMP_TO_BORDER) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pname);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pname);
		}
		else {
			std::cout << "���벻�����û��Ʒ�ʽ" << std::endl;
		}
	}
	void SetWrapS(GLint pname) {
		if (pname == GL_REPEAT || pname == GL_MIRRORED_REPEAT || pname == GL_CLAMP_TO_EDGE || pname == GL_CLAMP_TO_BORDER) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pname);
		}
		else {
			std::cout << "���벻�����û��Ʒ�ʽ" << std::endl;
		}
	}
	void SetWrapT(GLint pname) {
		if (pname == GL_REPEAT || pname == GL_MIRRORED_REPEAT || pname == GL_CLAMP_TO_EDGE || pname == GL_CLAMP_TO_BORDER) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pname);
		}
		else {
			std::cout << "���벻�����û��Ʒ�ʽ" << std::endl;
		}
	}

	void SetFilterMag(GLint pname) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pname);
	}
	void SetFilterMin(GLint pname) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
};
