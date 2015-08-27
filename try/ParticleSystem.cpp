#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(void)
{
	LastUsedParticle = 0;
	m_center = glm::vec3(0.0);
	 m_radius = 1.0f;
	
	 ParticlesContainer = new Particle[MaxParticles];
	 
	/*for(int i=0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data         = new GLubyte[MaxParticles * 4];*/

}


ParticleSystem::~ParticleSystem(void)
{
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &vertex_buffer);

	glDeleteVertexArrays(1, &ParticleVertexArrayID);
	delete[] g_particule_position_size_data;

}





int ParticleSystem::FindUnusedParticle(){

	for(int i=LastUsedParticle; i<MaxParticles; i++){
		if (ParticlesContainer[i].life < 0){  //if <0 : dead and unused
			LastUsedParticle = i;
			return i;
		}
	}

	for(int i=0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void ParticleSystem::SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void ParticleSystem::particleGenerator(double delta){

	int newparticles = (int)(delta*100.0);//每秒1000个new particle
	if (newparticles > (int)(0.016f*1000.0))//don't make  too much new particle
		newparticles = (int)(0.016f*1000.0);

	for(int i=0; i<newparticles; i++)
	{
		//init partical position

		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 2 + rand() % 3;

		//double ang = glm::linearRand(0.0, (double)M_PI/2);
		ParticlesContainer[particleIndex].initAngle = glm::linearRand(0.0, (double)M_PI);
		ParticlesContainer[particleIndex].moveRadius = m_radius - 0.5f;
		ParticlesContainer[particleIndex].pos = m_center + glm::vec3(0.5, ParticlesContainer[particleIndex].moveRadius*cos(ParticlesContainer[particleIndex].initAngle ),ParticlesContainer[particleIndex].moveRadius*sin(ParticlesContainer[particleIndex].initAngle ));
		//ParticlesContainer[particleIndex].pos = m_center;
		//ParticlesContainer[particleIndex].fakePos = m_center + glm::vec3(0.0, m_radius*cos(ang),m_radius*sin(ang));
		ParticlesContainer[particleIndex].mass = 1.0;
		glm::vec3 maindir = glm::normalize( m_center - ParticlesContainer[particleIndex].pos);
		//glm::vec3 maindir = glm::normalize( m_center - ParticlesContainer[particleIndex].fakePos);
		maindir = glm::cross(maindir, glm::vec3(1,0,0));
		ParticlesContainer[particleIndex].speed = 10.0f * maindir;
		ParticlesContainer[particleIndex].speed.x = 1.0f;
		// partical color
		ParticlesContainer[particleIndex].r = rand() % 256;
		ParticlesContainer[particleIndex].g = rand() % 256;
		ParticlesContainer[particleIndex].b = rand() % 256;
		ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

		ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;

	}
}

//glm::vec3 ParticleSystem::ForceAccumulate(Particle p){
//	
//	glm::vec3 currentForceDirection = glm::normalize( m_center - p.pos );//direction
//	//glm::vec3 currentForceDirection = glm::normalize( m_center - p.fakePos );//direction
//	glm::vec3 circleForce = glm::dot(p.speed, p.speed)/m_radius * currentForceDirection;
//
//	glm::vec3 totalForce = circleForce;
//
//	
//	return totalForce;
//}
glm::vec3 ParticleSystem::ForceAccumulate(Particle p){
	
	glm::vec3 current2DPos = glm::vec3(0.0, p.pos.y, p.pos.z);
	glm::vec3 current2DSpeed = glm::vec3(0.0, p.speed.y, p.speed.z);
	glm::vec3 currentForceDirection = glm::normalize( m_center - current2DPos );//direction
	//glm::vec3 currentForceDirection = glm::normalize( m_center - p.fakePos );//direction
	//moveRadius += 0.00001;
	glm::vec3 circleForce = glm::dot(current2DSpeed, current2DSpeed)/m_radius * currentForceDirection;

	glm::vec3 totalForce = circleForce;

	
	return totalForce;
}


void ParticleSystem::Updator(glm::vec3 CameraPosition, double delta){
	// Simulate all particles

	 ParticlesCount = 0;
	for(int i=0; i<MaxParticles; i++)
	{
		//for each alive partical, decrease life, update attributes
		Particle& p = ParticlesContainer[i]; // shortcut
		glm::vec3 v_mid = p.speed;
		if(p.life > 0.0f)
		{
			p.life -= delta;

			//p.speed.y+=ForceAccumulate(p).y/p.mass * (float)delta * 1.0f;
			//p.speed.z+=ForceAccumulate(p).z/p.mass * (float)delta * 1.0f;
			p.speed.x+=0.05*delta;
			////p.pos+=p.speed*(float)delta;
			//p.pos.x+=delta;
			////p.pos.x+=delta*p.speed.x;
			//p.pos.y+=1*delta*p.speed.y;
			//p.pos.z+=1*delta*p.speed.z;

			p.moveRadius += 0.0035;
			p.initAngle+= 0.04;
			p.pos = m_center + glm::vec3(p.pos.x+0.5*delta*p.speed.x, p.moveRadius*cos(p.initAngle ),m_radius*sin(p.initAngle));

			

			if (p.life > 0.0f){

				p.cameradistance = glm::length2(p.pos - CameraPosition);

				// Fill the GPU buffer
				g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
				g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
				g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;

				g_particule_position_size_data[4*ParticlesCount+3] = p.size;

				g_particule_color_data[4*ParticlesCount+0] = p.r;
				g_particule_color_data[4*ParticlesCount+1] = p.g;
				g_particule_color_data[4*ParticlesCount+2] = p.b;
				g_particule_color_data[4*ParticlesCount+3] = p.a;

			}
			else{
				p.cameradistance = -1.0f;
				p.moveRadius = m_radius;
			}
			ParticlesCount++;
		}
	}
	
}

void ParticleSystem::generateBuffer(){
	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat particle_vertex_buffer_data[] = { 
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	};

	glGenVertexArrays(1, &ParticleVertexArrayID);
	glBindVertexArray(ParticleVertexArrayID);

	//particle vertex data
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_vertex_buffer_data), particle_vertex_buffer_data, GL_STATIC_DRAW);

	//particle position data
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//particle color data
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	glBindVertexArray(0);

}




void ParticleSystem::updateRender(GLuint particleProgramID, GLuint particleTexture,glm::mat4 viewMatrix, glm::mat4 ViewProjectionMatrix){

	glUseProgram(particleProgramID);
	glBindVertexArray(ParticleVertexArrayID);

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, particleTexture);
	glUniform1i(particleTextureID, 2);

	glUniform3f(CameraRight_worldspace_ID, viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	glUniform3f(CameraUp_worldspace_ID, viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//一个buffer存储粒子的中心。 size : x + y + z + size = 4
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,0,(void*)0);                        


	//一个buffer存储粒子的颜色 size : r + g + b + a = 4
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,0,(void*)0); 

	
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
	/*glPointSize(5.0f);
	glDrawArrays(GL_POINTS,0,ParticlesCount);*/

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}


void ParticleSystem::initParticle(GLuint particleProgramID){

	//加载shader和纹理
	/*particleProgramID = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );
	particleTexture = loadDDS("particle.DDS");*/

	//创建粒子集，初始化
	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data         = new GLubyte[MaxParticles * 4];

	for(int i=0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	// Vertex shader
	CameraRight_worldspace_ID  = glGetUniformLocation(particleProgramID, "CameraRight_worldspace");
	CameraUp_worldspace_ID  = glGetUniformLocation(particleProgramID, "CameraUp_worldspace");
	ViewProjMatrixID = glGetUniformLocation(particleProgramID, "VP");

	// fragment shader
	particleTextureID  = glGetUniformLocation(particleProgramID, "myTextureSampler");
	particleFalseTextureID  = glGetUniformLocation(particleProgramID, "falseColorRender");
	TurnOffParticleID  = glGetUniformLocation(particleProgramID, "turnOffParticle");
}

void ParticleSystem::StopParticles(){

	int newparticles = 0;//每秒1000个new particle
	LastUsedParticle = 0;
	for(int i=0; i<MaxParticles; i++)
	{
		//init partical position
		ParticlesContainer[i].life = -1.0f;

		ParticlesContainer[i].initAngle = 0;
		ParticlesContainer[i].moveRadius = 0;
		ParticlesContainer[i].pos =glm::vec3(0.0);
		ParticlesContainer[i].cameradistance = -1.0f;
		ParticlesContainer[i].speed = glm::vec3(0.0);
		// partical color
		ParticlesContainer[i].r = 0;
		ParticlesContainer[i].g = 0;
		ParticlesContainer[i].b = 0;
		ParticlesContainer[i].a = 1.0;

		ParticlesContainer[i].size = 0;

	}
	memset(g_particule_position_size_data, 0, sizeof(g_particule_position_size_data)); 
	memset(g_particule_color_data, 0, sizeof(g_particule_color_data)); 
	
}