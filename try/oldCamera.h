#pragma once
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glfw3.h>

class oldCamera
{
public:
	oldCamera(void);
	~oldCamera(void);

	void computeMatricesFromInputs();
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::mat4 setViewMatrix(glm::vec3 pos, glm::vec3 dir);
	glm::vec3 getEyePosition();

	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::vec3 up;
	glm::vec3 position;
	glm::vec3 target;

private:
	float horizontalAngle;
	float verticalAngle;
	float initialFoV;
	float speed;
	float mouseSpeed;
};

