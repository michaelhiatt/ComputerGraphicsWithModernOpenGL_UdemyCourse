#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"


const float toRadians = 3.14159265f / 180.f;

Window mainWindow;

std::vector<Mesh *> meshList;
std::vector<Shader> shaderList;


// Vertex Shader
static const char *vShader = "Shaders/shader.vert";

// Fragment Shader
static const char *fShader = "Shaders/shader.frag";


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		-1.f, -1.f, 0.f,
		0.f, -1.f, 1.f,
		1.f, -1.f, 0.f,
		0.f, 1.f, 0.f
	};

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 12, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 12, 12);
	meshList.push_back(obj2);
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	GLuint uniformProjection = 0, uniformModel = 0;
	glm::mat4 projection(1.f);
	projection = glm::perspective(45.f, mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.f);


	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		// Get and handle user input events
		glfwPollEvents();


		// Clear the window
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();

		glm::mat4 model(1.f);
		model = glm::translate(model, glm::vec3(0.f, 0.f, -2.5f));
		//model = glm::rotate(model, currentAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

		meshList[0]->RenderMesh();

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, 1.f, -2.5f));
		//model = glm::rotate(model, currentAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		
		meshList[1]->RenderMesh();

		glUseProgram(0);

		// Swap buffers on the main window
		mainWindow.swapBuffers();
	}

	return 0;
}