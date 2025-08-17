#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath,const char* geometryPath = nullptr) {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			if (geometryPath!=nullptr) {
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}


		}
		catch (std::ifstream::failure &e)
		{
			std::cout<< "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		
		unsigned int vertex, fragment,geometry;
		//顶点着色器
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex,1,&vShaderCode,NULL);//第二个参数是传递的源码字符串数量
		glCompileShader(vertex);
		//assert
		checkCompileErrors(vertex,"VERTEX");
		
		//片段着色器
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment,1,&fShaderCode,NULL);
		glCompileShader(fragment);
		//assert
		checkCompileErrors(fragment, "FRAGMENT");

		if (geometryPath!=nullptr) {
			const char* gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry,1,&gShaderCode,NULL);//编号，加载字符串个数，数据指针，数组长度
			glCompileShader(geometry);
			checkCompileErrors(geometry,"GEOMETRY");
		}
		
		//Shader program link
		ID = glCreateProgram();
		glAttachShader(ID,vertex);
		glAttachShader(ID,fragment);
		if (geometryPath != nullptr) { glAttachShader(ID, geometry); }
		glLinkProgram(ID);
		//assert
		checkCompileErrors(ID, "PROGRAM");
		//delete shader 合成完着色器程序就可以删了
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);

	};
	~Shader() =default;
	void use() { glUseProgram(ID); };
	//输入uniform变量前要先激活对应的着色器程序
	//因为指定了着色器程序，所以查询不需要激活
	void setBool(const std::string& name, bool value)const { glUniform1i(glGetUniformLocation(ID,name.c_str()),(int)value); };
	void setInt(const std::string& name, int value)const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); };
	void setFloat(const std::string& name, float value)const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); };
	void setMatrix4fv(const std::string& name, float* value)const { glUniformMatrix4fv(glGetUniformLocation(ID,name.c_str()),1,GL_FALSE,value); }//形参列表：第二参数是输入矩阵个数，第三个参数是是否将矩阵转置，第4个是矩阵数据
	void setMatrix4fv(const std::string& name, float* value,GLuint i)const { glUniformMatrix4fv(glGetUniformLocation(ID, (name+"[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, value); };
	void setMat4(const std::string& name, GLuint i,float* value)const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), i, GL_FALSE, value); }
	void setMatrix3fv(const std::string& name, float* value)const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value); }
	void setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID,name.c_str()),x,y,z); }
	void setVec3(const std::string& name, float * vector3 ) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()),1,vector3); }
	void setVec3(const std::string& name, float* vector3, GLuint i) const { glUniform3fv(glGetUniformLocation(ID, (name+"["+std::to_string(i)+"]").c_str()), 1, vector3); }
	void setVec2(const std::string& name, float * vector2) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, vector2); }
	void setVec2(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
	//设置uniform块绑定点
	void setBindingPoints(const std::string& name, unsigned int x) { glUniformBlockBinding(ID,glGetUniformBlockIndex(ID,name.c_str()),x); }

private:
	void checkCompileErrors(unsigned int shader,const std::string& type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};

#endif // !SHADER_H




