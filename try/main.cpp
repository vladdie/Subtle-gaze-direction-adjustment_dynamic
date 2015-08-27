// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

#include "shader.hpp"
#include "MeshLoader.h"
#include "oldCamera.h"
#include "ParticleSystem.h"
#include <iostream>
#include "MyGaze.h"

#define SCENEMODEL "sponza.obj"
#define OBJMODEL "sphere.obj"
#define WINDOW_WIDTH 1680
#define WINDOW_HEIGHT 1050

MyGaze* gazeData;

Mesh* mainMesh;
Mesh* objMesh[5];
oldCamera* mainCamera;
GLuint renderedTexture, AveLumID;
GLuint pass1Index, pass2Index, pass3Index, pass4Index;
GLuint blurFbo1,blurFbo2,FramebufferName3, tex1, tex2,renderedTexture3;
GLuint ParticleFramebuffer, renderedParticleTexture;
int defaultCue;
float litthreshold =0.8f, sum=0;
ParticleSystem* m_particles;

GLuint createTexture(int w,int h, bool depth)
{
	unsigned int textureId;
	glGenTextures(1,&textureId);
	glBindTexture(GL_TEXTURE_2D,textureId);
	if(depth){
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}else{
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	int i;
	i=glGetError();
	if(i!=0)
	{
		//		  std::cout << "Error happened while loading the texture: " << i << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D,0);
	return textureId;
}

void quadDraw(GLuint quad_vertexbuffer)
{
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);
}

float ScomputeLogAveLuminance()
{
	GLfloat *texData = new GLfloat[WINDOW_WIDTH*WINDOW_HEIGHT*3];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData);
	float sum = 0.0f;
	for( int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++ ) {
		float lum = glm::dot(glm::vec3(texData[i*3+0], texData[i*3+1], texData[i*3+2]),
			glm::vec3(0.2126f, 0.7152f, 0.0722f) );//0.21*r+0.71*g+0.072*b
		sum += logf( lum + 0.00001f );
	}
	//glUniform1f(AveLumID, expf( sum / (WINDOW_WIDTH*WINDOW_HEIGHT) ));
	return sum;
	delete [] texData;
}

bool setupFBO() {

#pragma region //放置经过水平+竖直高斯模糊后的texture
	blurFbo1 = 0;
	glGenFramebuffers(1, &blurFbo1);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo1);
	tex1 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);//second pass rendering-vertical
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex1, 0);
	//GLenum DrawBuffers2[1] = {GL_COLOR_ATTACHMENT0};
	//glDrawBuffers(1, DrawBuffers2); // "1" is the size of DrawBuffers
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //放置对cue obj的false color rendering texture
	blurFbo2 = 0;
	glGenFramebuffers(1, &blurFbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo2);
	tex2 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //放置对cue obj的false color rendering texture
	 FramebufferName3 = 0;
	glGenFramebuffers(1, &FramebufferName3);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName3);
	renderedTexture3 = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture3, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

#pragma region //放置对particle的rendering texture
	 ParticleFramebuffer = 0;
	glGenFramebuffers(1, &ParticleFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ParticleFramebuffer);
	renderedParticleTexture = createTexture(WINDOW_WIDTH, WINDOW_HEIGHT,false);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedParticleTexture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
#pragma endregion

	return true;
}

glm::vec2 calculateWindowSpacePos(glm::mat4 mP, glm::mat4 mV, glm::vec3 pointPos){
	glm::vec4 clipSpacePos = mP * mV * glm::vec4(pointPos, 1.0);
	//printf("clip Space value is:  %f,  %f, %f, %f \n", clipSpacePos.x, clipSpacePos.y, clipSpacePos.z, clipSpacePos.w);
	if(clipSpacePos.w != 0){
		glm::vec3 NDCSpacePos = glm::vec3( clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);
		//printf("NDC Space value is:  %f, %f, %f \n", NDCSpacePos.x, NDCSpacePos.y, NDCSpacePos.z);
		glm::vec2 viewSize = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::vec2 viewOffSet = glm::vec2(0.0);
		//glm::vec2 windowSpacePos = glm::vec2((NDCSpacePos.x+1.0)/2.0, (NDCSpacePos.y+1.0)/2.0) * glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT)+glm::vec2(0.0);
		glm::vec2 windowSpacePos = glm::vec2 ( ((NDCSpacePos.x + 1.0) / 2.0) * viewSize.x + viewOffSet.x, ((NDCSpacePos.y + 1.0) / 2.0) * viewSize.y + viewOffSet.y);
		return windowSpacePos;
	}
	return glm::vec2(1.0);
}

void keyEventHandle(){
	if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS)
	{
		litthreshold -=0.001;
		if (litthreshold <= 0.0)
		{
			litthreshold = 0.0;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS)
	{
		litthreshold +=0.001;
		if (litthreshold >= 1)
		{
			litthreshold = 1;
		}
	}
}

glm::vec2 displayGazeData(){
	gazeData->update();
	//glm::vec2 rightEyePosition = gazeData->getRightEyePcenter();
	//rightEyePosition.x = WINDOW_WIDTH * rightEyePosition.x;
	//rightEyePosition.y = WINDOW_HEIGHT * rightEyePosition.y;
	//glm::vec2 rightEyePosition = gazeData->getPoint2dRaw();
	glm::vec2 rightEyePosition = gazeData->getPoint2dAvg();
	rightEyePosition.y = WINDOW_HEIGHT - rightEyePosition.y;
	//printf("Right eye center position is: %f, %f \n", rightEyePosition.x, rightEyePosition.y);

	return rightEyePosition;

}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //	glfwWindowHint(GLFW_SAMPLES, 8);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "SPONZA", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	//// Ensure we can capture the escape key being pressed below
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	//glfwSetCursorPos(window, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	gazeData = new MyGaze(WINDOW_WIDTH, WINDOW_HEIGHT);
	

	m_particles = new ParticleSystem();
	Texture* particleTextureObj = new Texture();
	GLuint ParticleProgramID = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );
	GLuint particleTexture = particleTextureObj->loadDDS("particle.DDS");
	m_particles->initParticle(ParticleProgramID);
	m_particles->generateBuffer();
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShadingRTT.vertexshader", "StandardShadingRTT.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint fadeInCue = glGetUniformLocation(programID, "fadeIn");
	GLuint attractprCueID = glGetUniformLocation(programID, "attractor");
	GLuint transparencyID = glGetUniformLocation(programID, "transparency");
	GLuint ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");
	GLuint NormalTextureID  = glGetUniformLocation(programID, "NormalTextureSampler");
	GLuint colorTextureID  = glGetUniformLocation(programID, "myTextureSampler");
	GLuint colorSwitchID  = glGetUniformLocation(programID, "colorSwitch");
	// Read our .obj file
	mainMesh = new Mesh();
	bool mainMeshLoadSuccess = mainMesh->LoadMesh(SCENEMODEL);

	glm::vec3 objPos[5];
	objPos[0] = glm::vec3(-30,30,10);
	objPos[1] = glm::vec3(30,30,10);
	objPos[2] = glm::vec3(-13,45,10);
	objPos[3] = glm::vec3(30,60,10);
	objPos[4] = glm::vec3(0,45,10);
	for(int i = 0; i <5; i++)
	{
		objMesh[i] = new Mesh();
		objMesh[i]->centerPos = objPos[i];
		objMesh[i]->LoadMesh(OBJMODEL);
	}
	int defaultCue = rand() % 5; 
	objMesh[defaultCue]->attractor = true;
	mainCamera = new oldCamera();


	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");


	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// ---------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	//// Alternative : Depth texture. Slower, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, WINDOW_WIDTH, WINDOW_HEIGHT, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	//// Depth texture alternative : 
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);


	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	setupFBO();


	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint falseColor_programID = LoadShaders( "falseColor.vertexshader", "falseColor.fragmentshader" );
	GLuint falseMatrixID = glGetUniformLocation(falseColor_programID, "MVP");
	
	GLuint quad_programID = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader" );
	GLuint texID = glGetUniformLocation(quad_programID, "renderedTexture");
	GLuint bur1texID = glGetUniformLocation(quad_programID, "BlurTex1");
	GLuint bur2texID = glGetUniformLocation(quad_programID, "BlurTex2");
	GLuint falseColortexID = glGetUniformLocation(quad_programID, "falseColorTexture");
	GLuint particleRendertexID = glGetUniformLocation(quad_programID, "particleRenderedTexture");
	AveLumID = glGetUniformLocation(quad_programID, "AveLum");
	GLuint 	litthresholdID = glGetUniformLocation(quad_programID, "litthreshold");
	pass1Index = glGetSubroutineIndex( quad_programID, GL_FRAGMENT_SHADER, "pass1");
	pass2Index = glGetSubroutineIndex( quad_programID, GL_FRAGMENT_SHADER, "pass2");
	pass3Index = glGetSubroutineIndex( quad_programID, GL_FRAGMENT_SHADER, "pass3");
	pass4Index = glGetSubroutineIndex( quad_programID, GL_FRAGMENT_SHADER, "pass4");
	GLuint HaloColorSwitchID  = glGetUniformLocation(quad_programID, "colorSwitch");

	ScomputeLogAveLuminance();
	glm::vec3 lightPos = glm::vec3(0,60,0);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	float countUnknownTime = 0;
	float transparency = 0;
	bool fadeOut = false;
	bool particleEffect = false;

	glBindVertexArray(0);

	double lastTime = glfwGetTime();

	double eyeXPos_old, eyeYPos_old;//mouse position on the screen space
	glfwGetCursorPos(window, &eyeXPos_old, &eyeYPos_old);
	eyeYPos_old =  WINDOW_HEIGHT-eyeYPos_old;
	/*glm::vec2 tempPos = displayGazeData();
	eyeXPos_old = tempPos.x;
	eyeYPos_old = tempPos.y;*/
	bool modulation = true;// default: target object needs to be modulated

	do{

		
		keyEventHandle();

#pragma region //设置闪烁效果
		if(transparency >= 0.2 ){
			fadeOut = true;
		}

		if(fadeOut)
		{
			transparency -= 0.007;
			if(transparency <= 0.0 )
			{
				fadeOut = false;
			}
		}else{
			transparency += 0.007;
		}
#pragma endregion

		mainCamera->computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = mainCamera->getProjectionMatrix();
		glm::mat4 ViewMatrix = mainCamera->getViewMatrix();

#pragma region 		//白色的球，显示正常的粒子特效 ParticleFramebuffer + renderedParticleTexture
		glBindFramebuffer(GL_FRAMEBUFFER, ParticleFramebuffer);//enable the framebuffer
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;
		glUseProgram(ParticleProgramID);
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
		glm::vec3 particleTransition;
		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor){
				
				particleTransition = objMesh[i]->centerPos;
				particleTransition.y = particleTransition.y - 4;
			}
		}
		glm::mat4 ParticleModelMatrix = glm::translate(glm::mat4(1.0),particleTransition);
		ParticleModelMatrix = glm::scale(ParticleModelMatrix, glm::vec3(1.25,1.25,1.25));
		//glm::mat4 ParticleModelMatrix = glm::mat4(1.0);
		ParticleModelMatrix = ParticleModelMatrix * glm::rotate(90.0f, glm::vec3(0,0,1));
		glm::mat4 ParticleMVP = ProjectionMatrix * ViewMatrix * ParticleModelMatrix;
		if(particleEffect){
			m_particles->particleGenerator(delta);
			m_particles->Updator( CameraPosition, delta);
			m_particles->SortParticles();
			glUniform1i(m_particles->particleFalseTextureID, false);
			m_particles->updateRender(ParticleProgramID,particleTexture,ViewMatrix,ParticleMVP);
		}
		glUseProgram(0);
		glUseProgram(programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(VertexArrayID);
		glUniform1f(transparencyID, transparency);
		glm::mat4 objModelMatrix;
		glm::mat4 MVP2;
		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor){
				objModelMatrix = glm::scale(glm::translate(glm::mat4(1.0), objMesh[i]->centerPos), glm::vec3(1,1,1));
				MVP2 = ProjectionMatrix * ViewMatrix * objModelMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &objModelMatrix[0][0]);
				glUniform1i(fadeInCue, true);
				objMesh[i]->Render();
			}
		}
		glBindVertexArray(0);
		glUseProgram(0);
#pragma endregion		

//
#pragma region 		//场景+闪烁的白色球 FramebufferName + renderedTexture
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);
		glBindVertexArray(VertexArrayID);
		glm::mat4 SceneModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(0.2,0.2,0.2));
		SceneModelMatrix = SceneModelMatrix * glm::rotate(-90.0f, glm::vec3(0,1,0));
		//glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(0.1,0.1,0.1));
		//glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * SceneModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &SceneModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniform1i(fadeInCue, false);
		glUniform1i(attractprCueID, false);
		glUniform1i(colorTextureID, 0);
		glUniform1i(NormalTextureID, 1);
		glUniform1i(colorSwitchID, fadeOut);
		mainMesh->Render();
	
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(transparencyID, transparency);

		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor && modulation){
				glUniform1i(attractprCueID, true);
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &objModelMatrix[0][0]);
				glUniform1i(fadeInCue, objMesh[i]->fadeIn);
				objMesh[i]->Render();
			}
		}
		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
#pragma endregion


#pragma region //saccadic data analysis, decide whether or not shut down the modulation
		double eyeXPos_current, eyeYPos_current;//mouse position on the screen space
		glfwGetCursorPos(window, &eyeXPos_current, &eyeYPos_current);
		eyeYPos_current =  WINDOW_HEIGHT-eyeYPos_current;
		objMesh[defaultCue]->screenPos = calculateWindowSpacePos(ProjectionMatrix, ViewMatrix, objMesh[defaultCue]->centerPos);
		//printf("cue object position is: %f, %f \n", objMesh[defaultCue]->screenPos.x, objMesh[defaultCue]->screenPos.y);
		//printf("Mouse position is: %f, %f \n", eyeXPos_current, eyeYPos_current);
		/*glm::vec2 eyetempPos = displayGazeData();
		  eyeXPos_current = eyetempPos.x;
		  eyeYPos_current = eyetempPos.y;*/
		glm::vec2 saccadicVelocity = glm::vec2(eyeXPos_current-eyeXPos_old, eyeYPos_current-eyeYPos_old) ;
		glm::vec2 FixToROC = glm::vec2(objMesh[defaultCue]->screenPos.x-eyeXPos_old, objMesh[defaultCue]->screenPos.y-eyeYPos_old) ;
		float vLength = glm::length(saccadicVelocity);
		float wLength = glm::length(FixToROC);
		float theta;
		if (vLength != 0)
		{
			
			float costheta = glm::dot(saccadicVelocity, FixToROC)/(vLength * wLength);
			theta = acos(costheta) * 180.0 / 3.14;
			if (theta <= 20)//smaller than 10 degree. move to the target
			{
				modulation = false;
				particleEffect = false;
			}
		}
		/*if (!(gazeData ->isFix()))
		{

			float costheta = glm::dot(saccadicVelocity, FixToROC)/(vLength * wLength);
			theta = acos(costheta) * 180.0 / 3.14;
			if (theta <= 20)
			{
				modulation = false;
			}
		}*/
		
#pragma endregion



#pragma region //cue的false color rendering 黑色背景，白色球 FramebufferName3+renderedTexture3
		
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName3);

		glUseProgram(falseColor_programID);
		glBindVertexArray(VertexArrayID);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f,0.1f,0.1f, 0.0f);
		
		for(int i = 0; i<5; i++){
			if(objMesh[i]->attractor){
				double eyeXPos_current, eyeYPos_current;//mouse position on the screen space
				glfwGetCursorPos(window, &eyeXPos_current, &eyeYPos_current);
				eyeYPos_current =  WINDOW_HEIGHT-eyeYPos_current;
				//printf("mouse position is:  %f,  %f \n", eyeXPos_current, eyeYPos_current);
				objMesh[i]->screenPos = calculateWindowSpacePos(ProjectionMatrix, ViewMatrix, objMesh[i]->centerPos);
				glm::vec2 distToObj = glm::vec2(eyeXPos_current-objMesh[i]->screenPos.x, eyeYPos_current-objMesh[i]->screenPos.y);
				
				/*glm::vec2 eyetempPos = displayGazeData();
				glm::vec2 distToObj = glm::vec2(eyetempPos.x-objMesh[i]->screenPos.x, eyetempPos.y-objMesh[i]->screenPos.y);*/
				//判断眼睛是否触碰到物体
				float dist_squared = glm::dot(distToObj, distToObj);
				printf("dist_squared is:  %f \n", dist_squared);
				if(dist_squared < 20000 && (vLength == 0)){
					objMesh[i]->attractor = false;
					objMesh[i]->fadeIn = false;
					defaultCue = (defaultCue + (rand() % 5 + 1)) % 5; 
					objMesh[defaultCue]->attractor = true;
					countUnknownTime = 0;
					particleEffect = false;
					m_particles->StopParticles();
				}else{
					countUnknownTime += 1;
				}
				if(countUnknownTime >= 100){
					objMesh[i]->fadeIn = true;
				}
				//if(countUnknownTime >= 250 && (vLength == 0)){
				//	particleEffect = true;
				//	//m_particles->StopParticles();
				//}
				if(dist_squared <= 160000 && dist_squared > 20000 && (vLength == 0)){
					modulation = true;
					particleEffect = false;
				}
				if(dist_squared  > 160000 && (vLength == 0)){
					modulation = true;
					particleEffect = true;
	
				}
				glUniformMatrix4fv(falseMatrixID, 1, GL_FALSE, &MVP2[0][0]);
				if(modulation){
					objMesh[i]->Render();
				}
				
			}
		}
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glUseProgram(0);
		
#pragma endregion		

#pragma region	// Renderpass1抽取白球的边缘 blurFbo1+tex1 红色（调制色）的球儿
		
		glBindFramebuffer(GL_FRAMEBUFFER, blurFbo1);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(quad_programID);
		glBindVertexArray(VertexArrayID);
		glUniform1f(AveLumID, expf( sum / (WINDOW_WIDTH*WINDOW_HEIGHT) ));
		glUniform1f(litthresholdID, litthreshold);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture3);
		glUniform1i(falseColortexID, 0);
		glUniform1i(HaloColorSwitchID, fadeOut);
		quadDraw(quad_vertexbuffer);
#pragma endregion


#pragma region //particle的false color rendering 蓝色背景 黑色粒子 FramebufferName3+renderedTexture3
		
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName3);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
		glClearColor(0.2f,0.1f,0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		glUseProgram(ParticleProgramID);
		/*m_particles->particleGenerator(delta);
		m_particles->Updator( CameraPosition, delta);
		m_particles->SortParticles();*/
		if(particleEffect){
			glUniform1i(m_particles->particleFalseTextureID, true);
			m_particles->updateRender(ParticleProgramID,particleTexture,ViewMatrix,ParticleMVP);
		}
		
		glUseProgram(0);

#pragma endregion		

//
//
#pragma region	// Renderpass2 blurFbo2+tex2 纵向高斯模糊 blurFbo2+tex2
		
		glBindFramebuffer(GL_FRAMEBUFFER, blurFbo2);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(quad_programID);
		glBindVertexArray(VertexArrayID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glUniform1i(bur1texID, 0);
		quadDraw(quad_vertexbuffer);

#pragma endregion


#pragma region	// Renderpass3 横向高斯模糊 blurFbo1+tex1 橘红（调制色）的模糊小球
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass3Index);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFbo1);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex2);
		glUniform1i(bur2texID, 0);
		quadDraw(quad_vertexbuffer);

#pragma endregion
//
////
		//render to screen
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass4Index);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		glUniform1i(texID, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glUniform1i(bur1texID, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, renderedTexture3);
		glUniform1i(falseColortexID, 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, renderedParticleTexture);
		glUniform1i(particleRendertexID, 3);
		
		quadDraw(quad_vertexbuffer);

		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glUseProgram(0);

		eyeXPos_old = eyeXPos_current;
		eyeYPos_old = eyeYPos_current;

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader

	glDeleteProgram(programID);
//	glDeleteTextures(1, &TextureID);

	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &depthrenderbuffer);
	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

