#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <algorithm>
#include <GL/glew.h>
//#include "shader.h"
//#include "texture.h"

#define MaxParticles 1000
#define M_PI 3.14f

struct Particle{
	float mass;
	glm::vec3 pos; //position
	glm::vec3 speed;  //velocity
	glm::vec3 fakePos;
	double initAngle;
	float moveRadius;

	unsigned char r,g,b,a; // Color
	float size;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; 

	 Particle()
    {
        size = 0;
        life = -1.0f;
		cameradistance = -1.0f;
		mass = 1.0f;
		pos = glm::vec3(0.0);
		fakePos = glm::vec3(0.0);
        speed = glm::vec3(0.0);
		initAngle = 0.0;
		moveRadius = 0.0;
    }

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};


class ParticleSystem
{
public:
	ParticleSystem(void);
	~ParticleSystem(void);

	//创建一个大的粒子容器
	 Particle* ParticlesContainer;
	int particleNum;
	//float delta;
	int LastUsedParticle;
	//glm::vec3 CameraPosition;
	//GLuint particleProgramID, particleTexture, particleTextureID;
	GLuint particleTextureID, particleFalseTextureID,TurnOffParticleID;
	GLuint ViewProjMatrixID, CameraRight_worldspace_ID,CameraUp_worldspace_ID;


	int FindUnusedParticle();
	void SortParticles();
	void particleGenerator(double delta);
	void generateBuffer();
	void Updator(glm::vec3 CameraPosition, double delta);
	glm::vec3 ForceAccumulate(Particle p);
	void updateRender(GLuint particleProgramID, GLuint particleTexture, glm::mat4 viewMatrix, glm::mat4 ViewProjectionMatrix);
	void initParticle(GLuint particleProgramID);

	void StopParticles();
//private:
	/*double delta;*/
	glm::vec3 m_center;
	float m_radius;
	GLuint ParticleVertexArrayID;
	GLuint vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	int ParticlesCount;
	 GLfloat* g_particule_position_size_data;
	 GLubyte* g_particule_color_data;

};

