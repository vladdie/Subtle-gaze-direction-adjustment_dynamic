#pragma once
//#include "ogldev_math_3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
//#include "key.h"
#include <glfw3.h>

#define M_PI       3.14159265358979323846
#define ToRadian(x) (float)(((x) * M_PI / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / M_PI))


class Camera
{
public:
	Camera(int WindowWidth, int WindowHeight);

    Camera(int WindowWidth, int WindowHeight, const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up);

    bool OnKeyboard();

    void OnMouse(int x, int y);

    void OnRender();

    const glm::vec3& GetPos() const
    {
        return m_pos;
    }

    const glm::vec3& GetTarget() const
    {
        return m_target;
    }

    const glm::vec3& GetUp() const
    {
        return m_up;
    }

	glm::mat4 getViewMatrix(glm::vec3 Target, glm::vec3 Up);
	glm::mat4 getProjectionMatrix();

	glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

private:

    void Init();
    void Update();



    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    glm::vec2 m_mousePos;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};

