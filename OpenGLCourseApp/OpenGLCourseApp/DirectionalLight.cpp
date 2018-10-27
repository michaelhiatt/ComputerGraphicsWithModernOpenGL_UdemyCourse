#include "DirectionalLight.h"



DirectionalLight::~DirectionalLight()
{
}


DirectionalLight::DirectionalLight() : Light()
{
	direction = glm::vec3(0.f, -1.f, 0.f);
	lightProj = glm::ortho(-5.0f, 5.f, -5.f, 5.f, 0.1f, 20.f);
}

DirectionalLight::DirectionalLight(GLfloat shadowWidth, GLfloat shadowHeight,
								GLfloat red, GLfloat green, GLfloat blue,
								GLfloat aIntensity, GLfloat dIntensity,
								GLfloat xDir, GLfloat yDir, GLfloat zDir) : Light(shadowWidth, shadowHeight, red, green, blue, aIntensity, dIntensity)
{
	direction = glm::vec3(xDir, yDir, zDir);
	lightProj = glm::ortho(-20.0f, 20.f, -20.f, 20.f, 0.1f, 100.f);
}

void DirectionalLight::UseLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
	GLuint diffuseIntensityLocation, GLuint directionLocation)
{
	glUniform3f(ambientColourLocation, colour.x, colour.y, colour.z);
	glUniform1f(ambientIntensityLocation, ambientIntensity);

	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity);
}

glm::mat4 DirectionalLight::CalculateLightTransform()
{
	return lightProj * glm::lookAt(-direction, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
}
