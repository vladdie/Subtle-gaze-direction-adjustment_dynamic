#include "Camera.h"

const static float STEP_SCALE = 0.1f;
const static float EDGE_STEP = 0.5f;
const static int MARGIN = 10;

extern GLFWwindow* window;

Camera::Camera(int WindowWidth, int WindowHeight, const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up)
{
    m_windowWidth  = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = glm::normalize(Target);

    m_up = glm::normalize(Up);

    Init();
}


void Camera::Init()
{
    glm::vec3 HTarget(m_target.x, 0.0, m_target.z);
    HTarget = glm::normalize(HTarget);
    
    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - ToDegree(asin(HTarget.z));
        }
        else
        {
            m_AngleH = 180.0f + ToDegree(asin(HTarget.z));
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = ToDegree(asin(-HTarget.z));
        }
        else
        {
            m_AngleH = 90.0f + ToDegree(asin(-HTarget.z));
        }
    }
    
    m_AngleV = -ToDegree(asin(m_target.y));

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge  = false;
    m_OnRightEdge = false;
    m_mousePos.x  = m_windowWidth / 2;
    m_mousePos.y  = m_windowHeight / 2;

   // glutWarpPointer(m_mousePos.x, m_mousePos.y);
}


bool Camera::OnKeyboard()
{
    bool Ret = false;

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		m_pos.y += STEP_SCALE;
		 Ret = true;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
	   m_pos.y -= STEP_SCALE;
	    Ret = true;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		glm::vec3 Right = glm::normalize(glm::cross(m_up, m_target));
        Right *= STEP_SCALE;
        m_pos += Right;
        Ret = true;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		glm::vec3 Left = glm::normalize(glm::cross(m_target, m_up)); 
        Left *= STEP_SCALE;
        m_pos += Left;
        Ret = true;
	}

    return Ret;
}


void Camera::OnMouse(int x, int y)
{
    const int DeltaX = x - m_mousePos.x;
    const int DeltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_AngleH += (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 20.0f;

    if (DeltaX == 0) {
        if (x <= MARGIN) {
        //    m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) {
        //    m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();
}


void Camera::OnRender()
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge) {
        m_AngleH -= EDGE_STEP;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) {
        m_AngleH += EDGE_STEP;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= EDGE_STEP;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
           m_AngleV += EDGE_STEP;
           ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}

void Camera::Update()
{
    const glm::vec3 Vaxis(0.0f, 1.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    glm::vec3 View(1.0f, 0.0f, 0.0f);
    View = glm::normalize(glm::rotate(View, m_AngleH, Vaxis));

    // Rotate the view vector by the vertical angle around the horizontal axis
    glm::vec3 Haxis = glm::normalize(glm::cross(Vaxis, View));
	View = glm::rotate(View, m_AngleV, Haxis);
       
    m_target = glm::normalize(View);

    m_up = glm::normalize(glm::cross(m_target, Haxis));
}

glm::mat4 Camera::getViewMatrix(glm::vec3 Target, glm::vec3 Up){
	
	glm::vec3 N = glm::normalize(Target);
    glm::vec3 U = glm::normalize(Up);
    U = glm::cross(U, N);
    glm::vec3 V = glm::cross(N, U);

    ViewMatrix[0][0] = U.x;   ViewMatrix[0][1] = U.y;   ViewMatrix[0][2] = U.z;   ViewMatrix[0][3] = 0.0f;
    ViewMatrix[1][0] = V.x;   ViewMatrix[1][1] = V.y;   ViewMatrix[1][2] = V.z;   ViewMatrix[1][3] = 0.0f;
    ViewMatrix[2][0] = N.x;   ViewMatrix[2][1] = N.y;   ViewMatrix[2][2] = N.z;   ViewMatrix[2][3] = 0.0f;
    ViewMatrix[3][0] = 0.0f;  ViewMatrix[3][1] = 0.0f;  ViewMatrix[3][2] = 0.0f;  ViewMatrix[3][3] = 1.0f;
	return ViewMatrix;
}

glm::mat4 Camera::getProjectionMatrix(){
	ProjectionMatrix = glm::perspective(60.0f, 4.0f / 3.0f, 1.0f, 1000.0f);
	return ProjectionMatrix;
}