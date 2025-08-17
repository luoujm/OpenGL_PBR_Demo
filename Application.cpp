// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include<iostream>
#include<map>
#include<thread>
#include<chrono>
#include<ratio>
#include<algorithm>
#include<vector>
#include<optional>
#include<functional>
#include<string>
#include<variant>
#include<future>
#include<random>
#include<assert.h>
#include<glad\glad.h>
#include<glfw3.h>
#include"Shader.h"
#include"Camera.h"
#include"stb_image.h"
#include "Model.h"
#include <glm\glm.hpp>//OpenGL数学函数库
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <tiffio.h>
//---全局常量---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const float PI = 3.14159265359f;
//---camera---
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
//---回调函数---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
void processInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.CameraMove(FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.CameraMove(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.CameraMove(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.CameraMove(RIGHT);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.CameraMove(UP);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.CameraMove(DOWN);

}
//---加载贴图---
void LoadCubeTexture(unsigned int& textureID, std::vector<std::string>& textures_faces) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	unsigned char* data;
	for (size_t i = 0; i < textures_faces.size(); i++) {
		data = stbi_load(textures_faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			GLenum format = GL_RED;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		}
		else { Print("Fail to load texture") }

		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
unsigned int loadTIFTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, BitsPerSample, SamplesPerPixel, SampleFormat;
	TIFF* tif = TIFFOpen(path, "r");
	if (!tif) {
		fprintf(stderr, "无法打开 TIFF 文件: %s\n", path);
		return NULL;
	}
	uint32_t w, h; // 使用 uint32_t 避免弃用警告
	uint16_t bps,spp;  // 存储 BitsPerSample
	if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w) ||
		!TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h) ||
		!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps) ||
		!TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp)) {
		fprintf(stderr, "无法获取 TIFF 标签\n");
		TIFFClose(tif);
		return NULL;
	}
	if (TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &SampleFormat)) {
		switch (SampleFormat) {
		case SAMPLEFORMAT_UINT:
			printf("(Unsigned integer, LDR)\n");
			break;
		case SAMPLEFORMAT_INT:
			printf("(Signed integer, LDR)\n");
			break;
		case SAMPLEFORMAT_IEEEFP:
			printf("(Floating-point, HDR)\n");
			break;
		case SAMPLEFORMAT_VOID:
			printf("(Undefined format)\n");
			break;
		default:
			printf("(Unknown format)\n");
			break;
		}
	}
	else {
		printf("SampleFormat not set, defaulting to unsigned integer (LDR)\n");
	}
	printf("TIFF Info: Width=%u, Height=%u, BitsPerSample=%u, SamplesPerPixel=%u\n ", w, h, bps, spp );
	width = w;
	height = h;
	BitsPerSample = bps;
	SamplesPerPixel = spp;
	// 检查 BitsPerSample 是否为 8 的倍数
	if (bps % 8 != 0) {
		fprintf(stderr, "不支持的 BitsPerSample: %u (必须是 8 的倍数)\n", bps);
		TIFFClose(tif);
		return NULL;
	}
	// 计算每像素字节数
	size_t bytes_per_pixel = spp * (bps / 8);
	size_t buffer_size = w * h * bytes_per_pixel;
	unsigned char* data = (unsigned char*)malloc(buffer_size);
	if (!data) {
		fprintf(stderr, "内存分配失败: %zu 字节\n", buffer_size);
		TIFFClose(tif);
		return NULL;
	}
	// 逐行读取数据
	for (uint32_t row = 0; row < h; row++) {
		if (TIFFReadScanline(tif, data + row * w * bytes_per_pixel, row, 0) < 0) {
			fprintf(stderr, "读取 TIFF 行 %u 失败\n", row);
			free(data);
			TIFFClose(tif);
			return NULL;
		}
	}
	// 读取 RGBA 数据
	/*if (!TIFFReadRGBAImage(tif, width, height, (uint32_t*)data, 0)) {
		fprintf(stderr, "读取 TIFF 数据失败\n");
		free(data);
		TIFFClose(tif);
		return NULL;
	}*/
	GLenum internal_format, format;
	if (SamplesPerPixel == 1) {
		internal_format = BitsPerSample == 8 ? GL_RED : GL_R16;
		format = GL_RED;
	}
	else if (SamplesPerPixel == 3) {
		internal_format = BitsPerSample == 8 ? GL_RGB : GL_RGB16;
		format = GL_RGB;
	}
	else if (SamplesPerPixel == 4) {
		internal_format = BitsPerSample == 8 ? GL_RGBA : GL_RGBA16;
		format = GL_RGBA;
	}
	else {
		fprintf(stderr, "不支持的 SamplesPerPixel: %u\n", SamplesPerPixel);
		TIFFClose(tif);
		return NULL;
	}

	GLenum type = BitsPerSample == 8 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);//输入到GL_RGB或GL_RGB16，OpenGL将自动把输入数据归一化了
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	TIFFClose(tif);
	return textureID;
}
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents,isHDR;
	isHDR = stbi_is_hdr(path);
	if (isHDR) {
		float* floatData = stbi_loadf(path, &width, &height, &nrComponents, 0); // final 4 Force RGBA if not add alpha 1,0表示根据原始通道数加载
		if (floatData) {
			GLenum format, internal_format;
			if (nrComponents == 1){
				format = GL_RED;
			internal_format = GL_R32F;}
			else if (nrComponents == 3){
				format = GL_RGB;
			internal_format = GL_RGB32F;}
			else if (nrComponents == 4){
				format = GL_RGBA;
			internal_format = GL_RGB32F;}
			printf("%s is HDR have%d",path,nrComponents);
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_FLOAT, floatData);//倒数第二个参数表示数据格式（单通道数据格式）
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			stbi_image_free(floatData);
		}
		else {
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(floatData);
		}
	}
	else {
		unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}
//---封装函数---
void inline GeneCubeMapFBO(unsigned int&captureFBO,unsigned int&CubeMap, unsigned int& depthCubeMap,unsigned int width,unsigned int height) {
	glGenTextures(1, &CubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
	glGenFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CubeMap, 0);
	glGenTextures(1, &depthCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void inline ChangeCubeMapFBOLOD(unsigned int& captureFBO, unsigned int& CubeMap, unsigned int& depthCubeMap, unsigned int lod) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CubeMap, lod);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, lod);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	
}

void GeneCubeTextureViewProjectionMatrix(std::vector<glm::mat4>& ptjTransforms, glm::vec3 Pos, GLfloat aspect, GLfloat near, GLfloat far)
{
	glm::mat4 Proj = glm::perspective(glm::radians(90.0f), aspect, near, far);
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));//立方体纹理V坐标朝下
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	ptjTransforms.push_back(Proj *
		glm::lookAt(Pos, Pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
}
//---渲染通道---
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere() {
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = static_cast<unsigned int>(indices.size());

		std::vector<float> data;
		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);//注意是strip
}
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
int main() {
	//-------
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	std::vector<glm::mat4>cubeMatrices;
	cubeMatrices.reserve(6);
	/*std::vector<std::string> faces{
		"./skybox/right.jpg",
		"./skybox/left.jpg",
		"./skybox/top.jpg",
		"./skybox/bottom.jpg",
		"./skybox/front.jpg",
		"./skybox/back.jpg"
	};*/
	//-------


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBR_window", NULL, NULL);
	if (window == 0) {
		Print("Fail to Create GLFW_window")
			glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		Print("Fail to Initialize GLAD")
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//没有对象渲染的像素被该默认颜色填充,后面用到延迟渲染堆叠光照画面最好是清零
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//---switch---
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	stbi_set_flip_vertically_on_load(true);
	//glEnable(GL_BLEND);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	//---shader---
	Shader PBRshader("./shader/PBRVER.txt", "./shader/PBRFRAG.txt");
	Shader equirectangularToCubemapshader("./shader/equirectangular_to_cubemapVER.txt", "./shader/equirectangular_to_cubemapFRAG.txt", "./shader/equirectangular_to_cubemapGEO.txt");
	Shader irradianceshader("./shader/irradiance_convolutionVER.txt", "./shader/irradiance_convolutionFRAG.txt","./shader/irradiance_convolutionGEO.txt");
	Shader Skyshader("./shader/skyboxVER.txt", "./shader/skyboxFRAG.txt");
	Shader prefiltershader("./shader/prefilterVER.txt", "./shader/prefilterFRAG.txt", "./shader/prefilterGEO.txt");
	Shader brdfshader("./shader/BRDFVER.txt", "./shader/BRDFFRAG.txt");
	//---shader settings---
	PBRshader.use();
	PBRshader.setInt("material.texture_albedoMap1", 0);
	PBRshader.setInt("material.texture_normalMap1", 1);
	PBRshader.setInt("material.texture_metallicMap1", 2);
	PBRshader.setInt("material.texture_roughnessMap1", 3);
	PBRshader.setInt("material.texture_aoMap1", 4);
	PBRshader.setInt("irradianceMap", 5);
	PBRshader.setInt("prefilterMap", 6);
	PBRshader.setInt("brdfLUT", 7);
	PBRshader.setInt("material.texture_depth1", 8);
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	PBRshader.setMatrix4fv("projection", glm::value_ptr(projection));
	equirectangularToCubemapshader.use();
	equirectangularToCubemapshader.setInt("equirectangularMap", 0);
	irradianceshader.use();
	irradianceshader.setInt("environmentMap",0);
	Skyshader.use();
	Skyshader.setMatrix4fv("projection", glm::value_ptr(projection));
	Skyshader.setInt("environmentMap",0);
	//---load material textures---
	 unsigned int albedo = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_albedo.tif");
	 unsigned int normal = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_normal.tif");
	 unsigned int metallic = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_metallic.tif");
	 unsigned int roughness = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_roughness.tif");
	 unsigned int ao = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_ao.tif");
	 unsigned int depth = loadTIFTexture("./PBR Material/TCom_Plastic_SpaceBlanketFolds_2K_height.tif");
	// unsigned int albedo = loadTexture("./PBR Material/white/White_Marble_006_basecolor.png");
	// unsigned int normal = loadTexture("./PBR Material/white/White_Marble_006_normal.png");
	// unsigned int metallic = loadTexture("./PBR Material/white/White_Marble_006_height.png");
	// unsigned int roughness = loadTexture("./PBR Material/white/White_Marble_006_roughness.png");
	// unsigned int ao = loadTexture("./PBR Material/white/White_Marble_006_ambientOcclusion.png");
	 unsigned int hdrTexture = loadTexture("./HDR/lebombo_2k.hdr");
	// data
	// ------
	glm::vec3 lightPositions[4] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;
	unsigned int cubemap;
	unsigned int rendertocubemapFBO;
	unsigned int skybox;
	unsigned int irradianceMap;
	unsigned int irradianceFBO;
	unsigned int prefilterMap;
	unsigned int prefilterFBO;
	unsigned int depthCubeMap1;
	unsigned int depthCubeMap2;
	unsigned int depthCubeMap3;
	//---equirectangular_to_cubemap---
	GeneCubeMapFBO(rendertocubemapFBO,cubemap, depthCubeMap1,2048,2048);
	GeneCubeTextureViewProjectionMatrix(cubeMatrices, glm::vec3(0),1, 0.1f, 10.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, rendertocubemapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 2048, 2048);
	equirectangularToCubemapshader.use();
	equirectangularToCubemapshader.setMat4("cubeMatrices", 6, &cubeMatrices[0][0][0]);
	/*for (int i = 0; i < 6; i++) {
		printf("Matrix %d:\n", i);
		for (int j = 0; j < 4; j++) {
			printf("%f %f %f %f\n", captureViews[i][j][0], captureViews[i][j][1], captureViews[i][j][2], captureViews[i][j][3]);
		}
	}*/
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	renderCube();
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	GeneCubeMapFBO(irradianceFBO, irradianceMap, depthCubeMap2, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, irradianceFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 32, 32);
	irradianceshader.use();
	irradianceshader.setMat4("cubeMatrices", 6, &cubeMatrices[0][0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP,cubemap);
	renderCube();
	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	GeneCubeMapFBO(prefilterFBO,prefilterMap,depthCubeMap3,128,128);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap3);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindFramebuffer(GL_FRAMEBUFFER, prefilterFBO);
	prefiltershader.use();
	prefiltershader.setInt("environmentMap",0);
	prefiltershader.setMat4("cubeMatrices", 6, &cubeMatrices[0][0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glViewport(0, 0, mipWidth, mipHeight);
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefiltershader.setFloat("roughness", roughness);
		ChangeCubeMapFBOLOD(prefilterFBO,prefilterMap,depthCubeMap3,mip);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}

	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	unsigned int brdfLUTTexture;
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenTextures(1, &brdfLUTTexture);
	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
	glViewport(0, 0, 512, 512);
	brdfshader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//---render loop---
	while (!glfwWindowShouldClose(window))
	{

		
		//---refresh camera---
		camera.Refresh(static_cast<float>(glfwGetTime()));
		view = camera.GetViewMatrix();
		
		//---default fbo---
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PBRshader.use();
		PBRshader.setMatrix4fv("view", glm::value_ptr(view));
		PBRshader.setVec3("camPos", glm::value_ptr(camera.cameraPos));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedo);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metallic);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughness);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, ao);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, depth);
		glm::mat4 model = glm::mat4(1.0f);
		for (unsigned int i = 0; i < 4; ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 1.0) * 5.0, 0.0, 0.0);
			//newPos = lightPositions[i];
			PBRshader.setVec3("lightPositions", glm::value_ptr(newPos),i);
			PBRshader.setVec3("lightColors", glm::value_ptr(lightColors[i]),i);


		}
		for (int row = 0; row < nrRows; ++row)
		{
			for (int col = 0; col < nrColumns; ++col)
			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(float)(col - (nrColumns / 2)) * spacing,
					(float)(row - (nrRows / 2)) * spacing,
					0.0f
				));
				PBRshader.setMatrix4fv("model", glm::value_ptr(model));
				glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
				PBRshader.setMatrix3fv("normalMatrix",glm::value_ptr(normalMatrix));
				renderSphere();
			}
		}
		//---sky box---
		{
			Skyshader.use();
			Skyshader.setMatrix4fv("view", glm::value_ptr(view));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP,cubemap);
			glDepthFunc(GL_LEQUAL);
			renderCube();
			glDepthFunc(GL_LESS);
		}
		processInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}