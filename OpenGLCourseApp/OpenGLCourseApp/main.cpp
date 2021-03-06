#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
#include "Model.h"

const float toRadians = 3.14159265f / 180.f;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0;

Window mainWindow;

std::vector<Mesh *> meshList;
std::vector<Shader> shaderList;
Shader directionalShadowShader;

Camera camera;

Texture brickTexture;
Texture dirtTexture;

Material shinyMaterial;
Material dullMaterial;

Model xWing;
Model blackHawk;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

// Vertex Shader
static const char *vShader = "Shaders/shader.vert";

// Fragment Shader
static const char *fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int *indices, unsigned int indiceCount, GLfloat *vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;

		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset;
		in1 += normalOffset;
		in2 += normalOffset;

		vertices[in0] += normal.x;
		vertices[in0 + 1] += normal.y;
		vertices[in0 + 2] += normal.z;

		vertices[in1] += normal.x;
		vertices[in1 + 1] += normal.y;
		vertices[in1 + 2] += normal.z;

		vertices[in2] += normal.x;
		vertices[in2 + 1] += normal.y;
		vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);

		vertices[nOffset] = vec.x;
		vertices[nOffset + 1] = vec.y;
		vertices[nOffset + 2] = vec.z;
	}
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	directionalShadowShader = Shader();
	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map.vert", "Shaders/directional_shadow_map.frag");

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
		// x	y	z		u	  v			nx	  ny	nz
		-1.f, -1.f, -0.6f,	0.f,  0.f,		0.f,  0.f, 0.f,
		0.f, -1.f, 1.f,		0.5f, 0.f,		0.f,  0.f, 0.f,
		1.f, -1.f, -0.6f,	1.f,  0.f,		0.f,  0.f, 0.f,
		0.f, 1.f, 0.f,		0.5f, 1.0f,		0.f,  0.f, 0.f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[]{

		-10.f, 0.f, -10.f,	0.f, 0.f,		0.f, -1.f, 0.f,
		10.f, 0.f, -10.f,	10.f, 0.f,		0.f, -1.f, 0.f,
		-10.f, 0.f, 10.f,	0.f, 10.f,		0.f, -1.f, 0.f,
		10.f, 0.f, 10.f,	10.f, 10.f,		0.f, -1.f, 0.f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void RenderScene()
{
	glm::mat4 model(1.f);
	model = glm::translate(model, glm::vec3(0.f, 0.f, -2.5f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, 4.f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, -2.f, 0.f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[2]->RenderMesh();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-10.f, -1.5f, 18.f));
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	xWing.RenderModel();

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-6.f, -1.f, 0.f));
	model = glm::rotate(model, 270.f * toRadians, glm::vec3(1.f, 0.f, 0.f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	blackHawk.RenderModel();
}

void DirectionalShadowMapPass(DirectionalLight *light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	directionalShadowShader.SetDirectionalLightTransform(&light->CalculateLightTransform());

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{
	shaderList[0].UseShader();
	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glViewport(0, 0, 1366, 768);

	// Clear the window
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, pointLightCount);
	shaderList[0].SetSpotLights(spotLights, spotLightCount);
	shaderList[0].SetDirectionalLightTransform(&mainLight.CalculateLightTransform());

	mainLight.GetShadowMap()->Read(GL_TEXTURE1);
	shaderList[0].SetTexture(0);
	shaderList[0].SetDirectionalShadowMap(1);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	//spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	RenderScene();
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), -90.f, 0.f, 5.f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();

	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();

	shinyMaterial = Material(1.f, 32.f);
	dullMaterial = Material(0.3f, 4.f);

	xWing = Model();
	xWing.LoadModel("Models/x-wing.obj");

	blackHawk = Model();
	blackHawk.LoadModel("Models/uh60.obj");

	mainLight = DirectionalLight(2048, 2048,
								1.f, 1.f, 1.f, 
								0.1f, 0.3f,
								3.f, -7.f, -4.f);
	
	pointLights[0] = PointLight(0.f, 0.f, 1.f,
								0.2, 0.3f,
								4.f, 0.f, -3.f,
								0.3f, 0.2f, 0.1f);
	pointLightCount++;

	pointLights[1] = PointLight(0.f, 1.f, 0.f,
								0.1, 0.1f,
								-4.f, 2.f, 0.f,
								0.3f, 0.1f, 0.1f);
	pointLightCount++;

	spotLights[0] = SpotLight(1.f, 1.f, 1.f,
							0.2, 2.0f,
							4.f, 0.f, 0.f,
							0.f, -1.f, 0.f,
							1.0f, 0.0f, 0.f,
							20.f);
	spotLightCount++;

	spotLights[1] = SpotLight(1.f, 1.f, 1.f,
							0.2, 1.0f,
							0.f, 1.5f, 0.f,
							0.f, -1.5f, 0.f,
							1.0f, 0.0f, 0.1f,
							20.f);
	spotLightCount++;

	glm::mat4 projection(1.f);
	projection = glm::perspective(45.f, mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.f);

	
	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get and handle user input events
		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		DirectionalShadowMapPass(&mainLight);
		RenderPass(projection, camera.calculateViewMatrix());

		glUseProgram(0);

		// Swap buffers on the main window
		mainWindow.swapBuffers();
	}

	return 0;
}