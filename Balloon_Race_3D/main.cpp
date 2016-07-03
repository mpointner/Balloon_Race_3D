#include <iostream>
#include <string>
#include <sstream> // for: char**
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Resources/SceneObject.hpp"
#include "Resources/Camera.hpp"
#include "Scene/Geometry.hpp"
#include "Scene/Actor1.hpp"
#include "Scene/Ring.hpp"
#include "Scene/Stadium.hpp"
#include "Scene/Speed.hpp"
#include <PhysX\PxPhysicsAPI.h>
#include "Resources\RenderBuffer.h"
#include "Resources\Const.hpp"
#include "Scene\Rocket.hpp"
#include "Resources\FrustumG.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H  

//-------Loading PhysX libraries (32bit only)----------//

#ifdef _DEBUG //If in 'Debug' load libraries for debug mode 
#pragma comment(lib, "PhysX3DEBUG_x86.lib")				//Always be needed  
#pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")		//Always be needed
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")		//PhysX extended library 
#pragma comment(lib, "PhysXVisualDebuggerSDKDEBUG.lib") //For PVD only 

#else //Else load libraries for 'Release' mode
#pragma comment(lib, "PhysX3_x86.lib")	
#pragma comment(lib, "PhysX3Common_x86.lib") 
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysXVisualDebuggerSDK.lib")
#endif

using namespace std;
using namespace cgue;
using namespace glm;
using namespace physx;

void init(GLFWwindow* window);
void cleanup();
void draw(Shader* shader, mat4x4 view, mat4x4 proj, mat4x4 camera_model);
void update(float time_delta);
void handleInput(GLFWwindow* window, float time_delta);
void handleInputJOISTICK(GLFWwindow* window, float time_delta, int numberOfJoistick);
void printVec(string name, glm::vec4 vec);
void OnShutdown();
mat4x4 pxMatToGlm(PxMat44 pxMat);
void RenderQuad();
void shoot(int actorID);

Shader* renderShader;
Shader* shadowShader;
Shader* combineShader;
Shader* blurShader;
Shader* hdrShader;

GLuint FramebufferName = 0;
GLuint hdrFBO;
GLuint colorBuffers[2];
GLuint pingpongFBO[2];
GLuint pingpongColorbuffers[2];
GLuint combineFBO;
GLuint combineBuffer;

int ringCount = 1; // TODO can be made const at the end
const int numberOfActors = 2;
const int absolutNumberOfRockets = 10;
int currentNumberOfRockets = 0;// TODO how does this work correctly
const int numberOfSpeed = 1;

// TODO actor1 to actor
Actor1** actors; 
Ring** ring;
Rocket** rockets;
bool activeRockets[absolutNumberOfRockets];
bool rocketHit[absolutNumberOfRockets];
bool rocketDeleted[absolutNumberOfRockets];
bool speedCollected[numberOfActors];
Stadium* world;
Camera** camera;
PxRigidStatic** activatedCollisionRigidStatic;
PxRigidActor** activatedCollisionRigidActor;
PxRigidActor** addCollisionRigidActor;
PxRigidActor** activatedCheckpointRigidActorFront;
Ring** activeRing;
Speed** speedCube;

FrustumG* frustumG;

float RING_HEIGHT_HIGH = 2.0f;
float RING_HEIGHT_MEDIUM = 6.0f;
float RING_HEIGHT_LOW = 10.0f;

int width;
int height;

float MOVESPEED = 20.0 * 1.2f;// 40.0;
float ROTATESPEED = 240.0 * 1.2f;

static PxPhysics*				gPhysicsSDK = NULL;			//Instance of PhysX SDK
static PxFoundation*			gFoundation = NULL;			//Instance of singleton foundation SDK class
static PxDefaultErrorCallback	gDefaultErrorCallback;		//Instance of default implementation of the error callback
static PxDefaultAllocator		gDefaultAllocatorCallback;	//Instance of default implementation of the allocator interface required by the SDK

PxScene*						gScene = NULL;				//Instance of PhysX Scene				


class SimulationEvents : public PxSimulationEventCallback
{

	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)	//This is called when a breakable constraint breaks.
	{
	}

	void onWake(PxActor** actors, PxU32 count)	//This is called during PxScene::fetchResults with the actors which have just been woken up.						
	{
	}

	void onSleep(PxActor** actors, PxU32 count)	////This is called during PxScene::fetchResults with the actors which have just been put to sleep.						
	{
	}

	void onTrigger(PxTriggerPair* pairs, PxU32 nbPairs)	//This is called during PxScene::fetchResults with the current trigger pair events.		
	{

		//cout << nbPairs << " Trigger pair(s) detected\n";

		//loop through all trigger-pairs of PhysX simulation
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			//get current trigger actor & other actor info 
			//from current trigger-pair 
			const PxTriggerPair& curTriggerPair = pairs[i];

			PxRigidActor* triggerActor = curTriggerPair.triggerActor;
			PxRigidActor* otherActor = curTriggerPair.otherActor;

			// select other actor -> look for balloon
			int selectedActor = -1; // 0; TODO does this
			for (int i = 0; i < numberOfActors; i++) {
				if (actors[i]->actor == triggerActor || actors[i]->actor == otherActor) {
					selectedActor = i;
				}
			}

			int selectspeedCube = -1;
			for (int i = 0; i < numberOfSpeed; i++) {
				if (speedCube[i]->actor == triggerActor || speedCube[i]->actor == otherActor) {
					selectspeedCube = i;
				}
			}
			if (selectedActor >= 0 && selectspeedCube >= 0) {
				speedCollected[selectedActor] = true;
				continue; // TODO maybe wrong
			}

			int selectedRocket = -1;
			for (int i = 0; i < absolutNumberOfRockets; i++) {
				if (rockets[i]->actor == triggerActor || rockets[i]->actor == otherActor) {
					selectedRocket = i;
				}
			}

			if (selectedActor >= 0 && selectedRocket >= 0 && activeRockets[selectedRocket] && !rocketHit[selectedRocket] && !rocketDeleted[selectedRocket]) {
				//actors[selectedActor]->excelerateSpeed(0.9);
				cout << "rocketHit: " << selectedRocket << endl;
				rocketHit[selectedRocket] = true;
				continue; // TODO maybe wrong
			}

			// search for first Actor
			//if (selectedActor >= 0) {
				for (int j = 0; j < numberOfActors; j++)
				{
					if (otherActor != actors[j]->actor)
					{
						continue; // TODO find a nicer way
					}
					if (addCollisionRigidActor[j] != triggerActor)
					{
						addCollisionRigidActor[j] = triggerActor;
					}

					for (int i = 0; i < ringCount; i++)
					{
						if (activeRing[j] == ring[i] && ring[i]->triggerFront == triggerActor)
						{
							//cout << "active Front" << endl;
							activatedCheckpointRigidActorFront[j] = triggerActor;
						}
						if (activeRing[j] == ring[i] && activatedCheckpointRigidActorFront[j] == ring[i]->triggerFront && triggerActor == ring[i]->triggerBack)
						{
							cout << "active Back" << endl;

							int next = (i + 1) % ringCount;

							if (ring[i]->passRing(selectedActor)) {
								cout << "Gewonnen: Player: " << selectedActor << endl;
							}
							else {
								activeRing[j] = ring[next];
							}
						}
					}
				}
			}
	}


	//The method will be called for a pair of actors if one of the colliding shape pairs requested contact notification.
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		//cout << nbPairs << " Contact pair(s) detected\n";

		const PxU32 buff = 64; //buffer size
		PxContactPairPoint contacts[buff];

		//loop through all contact pairs of PhysX simulation
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			//extract contant info from current contact-pair 
			const PxContactPair& curContactPair = pairs[i];
			PxU32 nbContacts = curContactPair.extractContacts(contacts, buff);

			for (PxU32 j = 0; j < nbContacts; j++)
			{
				//print all positions of contact.   
				PxVec3 point = contacts[j].position;
				//cout<<"Contact point ("<<point.x <<" "<< point.y<<" "<<point.x<<")\n";
			}
		}
	}

};

static SimulationEvents gSimulationEventCallback;			//Instance of 'SimulationEvents' class inherited from 'PxSimulationEventCallback' class

glm::mat4 proj;
glm::mat4 view;

//Defining a custome filter shader 
PxFilterFlags customFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eCONTACT_DEFAULT
		| PxPairFlag::eTRIGGER_DEFAULT
		| PxPairFlag::eNOTIFY_CONTACT_POINTS
		| PxPairFlag::eCCD_LINEAR; //Set flag to enable CCD (Continuous Collision Detection) 

	return PxFilterFlag::eDEFAULT;
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
								  sourceString = "API";
								  break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
										  sourceString = "Application";
										  break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
											sourceString = "Window System";
											break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
											  sourceString = "Shader Compiler";
											  break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
										  sourceString = "Third Party";
										  break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
									sourceString = "Other";
									break;
	}
	default: {
				 sourceString = "Unknown";
				 break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
								  typeString = "Error";
								  break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
												typeString = "Deprecated Behavior";
												break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
											   typeString = "Undefined Behavior";
											   break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
											typeString = "Portability";
											break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
										typeString = "Performance";
										break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
								  typeString = "Other";
								  break;
	}
	default: {
				 typeString = "Unknown";
				 break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
									 severityString = "High";
									 break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
									   severityString = "Medium";
									   break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
									severityString = "Low";
									break;
	}
	default: {
				 severityString = "Unknown";
				 break;
	}
	}

	if (type != GL_DEBUG_TYPE_OTHER)
	{
		stringStream << "OpenGL Error: " << msg;
		stringStream << " [Source = " << sourceString;
		stringStream << ", Type = " << typeString;
		stringStream << ", Severity = " << severityString;
		stringStream << ", ID = " << id << "]";
	}

	return stringStream.str();
}

static void APIENTRY DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
	std::string error = FormatDebugOutput(category, category, id, severity, message);
	std::cout << error;
	std::cout << std::endl;
}

static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	std::string error = FormatDebugOutput(source, type, id, severity, message);
	std::cout << error;
	std::cout << std::endl;
}

int main(int argc, char** argv)
{
	// TODO implement full screen 
	width = 1024;
	height = 768;

	// Parameters
	if (argc == 3) {
		cout << "you are executing '" << argv[0] << "'" << endl;
		if ((stringstream(argv[1]) >> width).fail() || (stringstream(argv[2]) >> height).fail()){
			cout << "ERROR: Invalid argument!" << endl;
			system("PAUSE");
			exit(EXIT_FAILURE);
		}
	}
	else {
		cout << "you are executing '" << argv[0] << "'" << endl;
		cout << "Invalid parameters. Size is set to 800/600" << endl;
	}

	// Init GLFW
	if (!glfwInit()) {
		cout << "ERROR: Could not init glfw." << endl;
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	#if _DEBUG
		// Create a debug OpenGL context or tell your OpenGL library (GLFW, SDL) to do so.
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif

	//OGL INITIALISIEREN
	//OGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//Core Profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// Refresh rate
	int refresh_rate = 60;
	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);

	GLFWwindow* window;
	if (fullscreen)
	{
		const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		width = mode->width;
		height = mode->height;

		// Make Window
		window = glfwCreateWindow(width, height, "Balloon Race 3D", glfwGetPrimaryMonitor(), nullptr);
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		window = glfwCreateWindow(width, height, "Balloon Race 3D", nullptr, nullptr);
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	}

	// Set current
	glfwMakeContextCurrent(window);

	// Init Glew
	glewExperimental = true; // must have, so all functions are loaded
	if (glewInit() != GLEW_OK)
	{
		cout << "ERROR: Could init GLEW." << endl;
		glfwTerminate();
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	glGetError();
	#if _DEBUG
		// Query the OpenGL function to register your callback function.
		PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
		PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
		PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");

		// Register your callback function.
		if (_glDebugMessageCallback != NULL) {
			_glDebugMessageCallback(DebugCallback, NULL);
		}
		else if (_glDebugMessageCallbackARB != NULL) {
			_glDebugMessageCallbackARB(DebugCallback, NULL);
		}
		else if (_glDebugMessageCallbackAMD != NULL) {
			_glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
		}

		// Enable synchronous callback. This ensures that your callback function is called
		// right after an error has occurred. This capability is not defined in the AMD
		// version.
		if ((_glDebugMessageCallback != NULL) || (_glDebugMessageCallbackARB != NULL)) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		}
	#endif

	// FreeType
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;


	init(window);

	// Define the color with which the screen is cleared
	glClearColor(0.35f, 0.36f, 0.43f, 0.3f);
	// Define the area to draw on

	// Game Loop
	auto time = glfwGetTime();
	auto refreshTime = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		auto time_new = glfwGetTime();
		auto time_delta = (float)(time_new - time);
		refreshTime += time_delta;
		time = time_new;

		int drawWidth = width/numberOfActors;

		handleInput(window, time_delta);
		for (int i = 0; i < numberOfActors; i++) {
			handleInputJOISTICK(window, time_delta, i);
		}

		update(time_delta);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		NUMBER_OF_CULLED_MESHES = 0;


		for (int iRender = 0; iRender < numberOfActors; iRender++)
		{
			activeRing[iRender]->setActiveNext(true);

			//cout << "frametime: " << time_delta * 1000 << "ms" << " = " << 1.0 / time_delta << " fps" << endl;

			// Render to our framebuffer
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, drawWidth, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			//glViewport(0, 0, 1024, 1024);  // Render on the whole framebuffer, complete from the lower left corner to the upper right

			// We don't use bias in the shader, but instead we draw back faces, 
			// which are already separated from the front faces by a small distance 
			// (if your geometry is made this way)
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(shadowShader->programHandle);

			glm::vec3 lightInvDir = glm::vec3(-lightPos.x, -1.0, -1.0);

			// Compute the MVP matrix from the light's point of view
			//glm::mat4 depthProjectionMatrix = glm::perspective(90.0f, (float)width / (float)height, -20.0f, 20.0f);
			glm::mat4 depthProjectionMatrix = glm::ortho<float>(-40, 20, -60, 40, -20.0f, 40.0f);
			//glm::mat4 depthViewMatrix = camera->modelMatrix * pxMatToGlm(PxMat44(actor->actor->getGlobalPose().getInverse()));// 
			glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
			//lookAt
			// or, for spot light :
			//glm::vec3 lightPos(5, 20, 20);
			//glm::mat4 depthProjectionMatrix = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
			//glm::mat4 depthViewMatrix = glm::lookAt(lightPos, lightPos-lightInvDir, glm::vec3(0,1,0));

			glm::mat4 depthModelMatrix = glm::mat4(1.0);
			glm::mat4 depthVP = depthProjectionMatrix * depthViewMatrix;
			glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform

			// Shadow
			draw(shadowShader, depthViewMatrix, depthProjectionMatrix, mat4x4(1.0f));

			// 1. Render scene into floating point framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, drawWidth, height);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			// Clear screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(renderShader->programHandle);

			glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
				);

			GLuint DepthVPID = glGetUniformLocation(renderShader->programHandle, "depthVP");
			glUniformMatrix4fv(DepthVPID, 1, GL_FALSE, &depthVP[0][0]);


			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			GLuint ShadowMapID = glGetUniformLocation(renderShader->programHandle, "shadowMap");
			glUniform1i(ShadowMapID, 2);

			mat4x4 camera_model = camera[iRender]->getCameraModel();
			

			mat4x4 view = camera_model * pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose().getInverse()));

			nearDist = 0.1f;
			farDist = 200.0f;
			float fov = 100.0f;
			float ratio = (float)drawWidth / (float)height;

			proj = glm::perspective(fov, ratio, nearDist, farDist);

			hnear = abs(2 * tan(fov / 2) * nearDist);
			wnear = abs(hnear * ratio);

			hfar = abs(2 * tan(fov / 2) * farDist);
			wfar = abs(hfar * ratio);


			/*
			vec4 camera_pos = camera_model * pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose())) * vec4(0, 0, 0, 1);
			vec3 p = vec3(camera_pos.x, camera_pos.y, camera_pos.z);

			vec4 look_pos = pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose())) * vec4(0, 0, 0, 1);
			vec3 l = vec3(look_pos.x, look_pos.y, look_pos.z);

			vec4 up_pos = camera_model * vec4(0, 1, 0, 1);
			vec3 u = vec3(up_pos.x, up_pos.y, up_pos.z);
			u = u - p;
			u = normalize(u);
			*/

			vec4 camera_pos = pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose())) * vec4(camera[iRender]->getCameraPos(), 1);
			vec3 p = vec3(camera_pos.x, camera_pos.y, camera_pos.z);

			vec4 look_pos = pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose())) * vec4(camera[iRender]->getCameraLookAt(), 1);
			vec3 l = vec3(look_pos.x, look_pos.y, look_pos.z);

			vec4 up_pos = pxMatToGlm(PxMat44(actors[iRender]->actor->getGlobalPose())) * vec4(camera[iRender]->getCameraUp(), 1);
			vec3 u = vec3(up_pos.x, up_pos.y, up_pos.z);
			u = u - p;
			u = normalize(u);


			vec3 distcamera = p - l;
			float dist = length(distcamera);


			mat4x4 lookAt = glm::lookAt(p, l, u);

			/*
			cout << "p:" << p.x << "," << p.y << "," << p.z
				<< " l:" << l.x << "," << l.y << "," << l.z
				<< " u:" << u.x << "," << u.y << "," << u.z << " Length: " << dist << endl;
			*/


			/*

			cout << "camera_model" << endl;
			printMatGeometry(camera_model);

			cout << "view" << endl;
			printMatGeometry(view);

			cout << "lookAt" << endl;
			printMatGeometry(lookAt);


			//glm::lookAt()
			//*/

			
			frustumG->setCamInternals(fov, ratio, nearDist, farDist);
			frustumG->setCamDef(p, l, u);
			


			draw(renderShader, lookAt, proj, camera_model);
			
			for (int i = 0; i < absolutNumberOfRockets; i++) {
				if (activeRockets[i]) {
					rockets[i]->drawParticles(time_delta, view, proj);
				}
			}




			// 2. Blur bright fragments w/ two-pass Gaussian Blur 
			GLboolean horizontal = true, first_iteration = true;
			GLuint amount = 20;
			for (GLuint i = 0; i < amount; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				blurShader->useShader();
				glActiveTexture(GL_TEXTURE1);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glUniform1i(glGetUniformLocation(blurShader->programHandle, "horizontal"), horizontal);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
				GLuint BlurTextureID = glGetUniformLocation(blurShader->programHandle, "image");
				glUniform1i(BlurTextureID, 1);
				RenderQuad();
				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}


			// 2. Now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, combineFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, drawWidth, height);
			//glViewport(iRender * drawWidth, 0, drawWidth, height);
			combineShader->useShader();

			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
			auto colorBuffers0_location = glGetUniformLocation(combineShader->programHandle, "scene");
			glUniform1i(colorBuffers0_location, 8);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
			auto colorBuffers1_location = glGetUniformLocation(combineShader->programHandle, "bloomBlur");
			glUniform1i(colorBuffers1_location, 1);

			GLboolean bloom = true;
			GLfloat exposure = 0.8f;
			glUniform1i(glGetUniformLocation(combineShader->programHandle, "bloom"), bloom);
			glUniform1f(glGetUniformLocation(combineShader->programHandle, "exposure"), exposure);
			RenderQuad();
			
			//*
			// 3. Apply HDR-Effect
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(iRender * drawWidth, 0, drawWidth, height);
			hdrShader->useShader();


			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, combineBuffer);
			auto scene_location = glGetUniformLocation(hdrShader->programHandle, "scene");
			glUniform1i(scene_location, 7);
			glGenerateTextureMipmap(combineBuffer);

			auto hdrenabledLocation = glGetUniformLocation(hdrShader->programHandle, "hdrEnabled");
			glUniform1i(hdrenabledLocation, EFFECT_HDR_ENABLED);

			RenderQuad();
			//*/

			activeRing[iRender]->setActiveNext(false);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		/*
		int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
		std::cout << "Joystick/Gamepad 1 status: " << present << std::endl;
		*/
		if (glGetError() != GL_NO_ERROR)
		{
			cout << "ERROR: OpenGL Error" << endl;
		}

		if (refreshTime > 1)
		{
			//printMat("GlobalPose", pxMatToGlm(PxMat44(actor->actor->getGlobalPose())));
			//vec4 pos = pxMatToGlm(PxMat44(actor->actor->getGlobalPose())) * vec4(0, 0, 0, 1);
			//cout << "Balloon Position: " << pos.x << " " << pos.y << " " << pos.z << endl;

			if (CGUE_DISPLAY_FRAME_TIME)
			{
				cout << "Frame time: " << (int)(time_delta * 1000) << "ms, Frame/sec: " << (int)(1.0f / time_delta) << " PhysX Static Actors: " << gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC) << endl;
			}
			if (VIEWFRUSTUM_CULLING) {
				cout << "Number of Culled Meshes: " << NUMBER_OF_CULLED_MESHES << endl;
			}
			/*
			cout << "Rockets Status" << endl;
			for (int i = 0; i < absolutNumberOfRockets; i++) {
				cout << activeRockets[i] << " " << rocketHit[i] << " " << rocketDeleted[i] << endl;
			}
			*/

			refreshTime = 0;
		}
	}

	// Close GLFW Window
	glfwTerminate();

	//system("PAUSE");

	return EXIT_SUCCESS;
}

void OnShutdown()
{
	delete actors; actors = nullptr;
	delete ring; ring = nullptr;
	delete world; world = nullptr;
	delete camera; camera = nullptr;
	delete rockets; rockets = nullptr;
	delete speedCube; speedCube = nullptr;

	delete renderShader; renderShader = nullptr;
	delete combineShader; combineShader = nullptr;
	delete blurShader; blurShader = nullptr;
	delete hdrShader; hdrShader = nullptr;
	delete shadowShader; shadowShader = nullptr;

	delete frustumG; frustumG = nullptr;
	delete activatedCollisionRigidStatic; activatedCollisionRigidStatic = nullptr;
	delete activatedCollisionRigidActor; activatedCollisionRigidActor = nullptr;
	delete addCollisionRigidActor; addCollisionRigidActor = nullptr;
	delete activatedCheckpointRigidActorFront; activatedCheckpointRigidActorFront = nullptr;

	gPhysicsSDK->release();			//Removes any actors,  particle systems, and constraint shaders from this scene
	gFoundation->release();			//Destroys the instance of foundation SDK
}

void init(GLFWwindow* window) 
{

	atexit(OnShutdown);			//Called on application exit 

	//* Source: Learning Physics Modeling with PhysX
	//---------------------------PHYSX-----------------------------]
	//Creating foundation for PhysX
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

	//Creating instance of PhysX SDK
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
	//Mesh::mgPhysicsSDK = gPhysicsSDK;

	if (gPhysicsSDK == NULL)
	{
		cerr << "Error creating PhysX3 device, Exiting..." << endl;
		exit(1);
	}

	//Creating scene
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());		//Descriptor class for scenes 

	sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);					//Setting gravity
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);		//Creates default CPU dispatcher for the scene

	sceneDesc.filterShader = customFilterShader;					//Creates custom user collision filter shader for the scene
	sceneDesc.simulationEventCallback = &gSimulationEventCallback;  //Resgistering for receiving simulation events

	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;					//Set flag to enable CCD (Continuous Collision Detection) 

	gScene = gPhysicsSDK->createScene(sceneDesc);					//Creates a scene 

	//*/This will enable basic visualization of PhysX objects like- actors collision shapes and their axes. 
	//The function PxScene::getRenderBuffer() is used to render any active visualization for scene.
	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0);	//Global visualization scale which gets multiplied with the individual scales
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);	//Enable visualization of actor's shape
	gScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);	//Enable visualization of actor's axis

	//Creating PhysX material (staticFriction, dynamicFriction, restitution)
	PxMaterial* material = gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.5f);

	renderShader = new Shader("Shader/Draw.vert", "Shader/Draw.frag");

	combineShader = new Shader("Shader/Combine.vert", "Shader/Combine.frag");

	blurShader = new Shader("Shader/Blur.vert", "Shader/Blur.frag");

	hdrShader = new Shader("Shader/HDR.vert", "Shader/HDR.frag");

	
	frustumG = new FrustumG();


	glEnable(GL_DEPTH_TEST);

	glfwSetWindowTitle(window, "Balloon Race 3D");

	camera = new Camera*[numberOfActors];

	for (int i = 0; i < numberOfActors; i++){
		camera[i] = new Camera();
	}

	actors = new Actor1*[numberOfActors];

	for (int i = 0; i < numberOfActors; i++){
		actors[i] = new Actor1(renderShader, 2.0f + 4.0f * i, RING_HEIGHT_HIGH - 5, 6.0f, 180.0f);
		actors[i]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
		actors[i]->initActor();
	}
	
	ringCount = 10;
	ring = new Ring*[ringCount];
	
	ring = new Ring*[ringCount];
	ring[0] = new Ring(renderShader, - 4.0f, RING_HEIGHT_HIGH, 2.0f, 90.0f);
	ring[1] = new Ring(renderShader, - 10.0f, RING_HEIGHT_MEDIUM, 8.0, 70.0);
	ring[2] = new Ring(renderShader, -12.0f, RING_HEIGHT_LOW, 17.0, 60.0);
	ring[3] = new Ring(renderShader, -15.0f, RING_HEIGHT_LOW, 24.0, 10.0);
	ring[4] = new Ring(renderShader, -25.0, RING_HEIGHT_LOW, 25.0, 0.0);
	ring[5] = new Ring(renderShader, -30.0, RING_HEIGHT_MEDIUM, 20.0, -90.0);
	ring[6] = new Ring(renderShader, -28.0, RING_HEIGHT_HIGH, 0.0, -100.0);
	ring[7] = new Ring(renderShader, -27, RING_HEIGHT_MEDIUM, -20.0, -90.0);
	ring[8] = new Ring(renderShader, -14, RING_HEIGHT_LOW, -24.0, -170.0);
	ring[9] = new Ring(renderShader, -0, RING_HEIGHT_MEDIUM, -21.0, -180.0);

	ring[0]->setActiveNext(true);
	ring[0]->setLastRing(true);
	activeRing = new Ring*[ringCount];
	for (int i = 0; i < ringCount; i++)
	{
		activeRing[i] = ring[0];
	}

	for (int i = 0; i < ringCount; i++)
	{
		ring[i]->setActiveNext(false);
		ring[i]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
		ring[i]->initActor();
	}

	gScene->addActor(*ring[0]->actor);

	rockets = new Rocket*[absolutNumberOfRockets];

	//*activeRockets = new bool[absolutNumberOfRockets];
	for (int i = 0; i < absolutNumberOfRockets; i++) {
		activeRockets[i] = false;
		rocketHit[i] = false;
		rocketDeleted[i] = false;
		rockets[i] = new Rocket(renderShader, 1, 1, 1, 90, 0, 0);
		rockets[i]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	}

	world = new Stadium(renderShader);
	world->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	world->initActor();

	speedCube = new Speed*[numberOfSpeed];
	for (int i = 0; i < numberOfSpeed; i++)
	{
		speedCube[i] = new Speed(renderShader, -28.0, RING_HEIGHT_HIGH, -10.0, 0);
		speedCube[i]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
		speedCube[i]->initActor();
		//speedCube[i]->setNextTimeVisible(glfwGetTime() + 100.0f + rand() % 10);
	}
	/*
	Nur Frustrum Debug fuer 2ten Spieler
	int e = 0;
	speedCube[++e] = new Speed(renderShader, -6.01, -3.02, -20.9, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, -5.98, -3.02, -20.9, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, -6, -2.97, -20.9, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, -5.98, -2.97, -20.9, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();


	speedCube[++e] = new Speed(renderShader, -42.25, -57.38, 179, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, 30.25, -57.38, 179, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, -42.25, 51.38, 179, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();

	speedCube[++e] = new Speed(renderShader, 30.25, 51.38, 179, 0);
	speedCube[e]->setPhysX(gPhysicsSDK, gFoundation, gDefaultErrorCallback, gDefaultAllocatorCallback, gScene);
	speedCube[e]->initActor();
	*/


	activatedCollisionRigidStatic = new PxRigidStatic*[numberOfActors];
	activatedCollisionRigidActor = new PxRigidActor*[numberOfActors];
	addCollisionRigidActor = new PxRigidActor*[numberOfActors];
	activatedCheckpointRigidActorFront = new PxRigidActor*[numberOfActors];
	for (int i = 0; i < numberOfActors; i++)
	{
		activatedCollisionRigidStatic[i] = nullptr;
		activatedCollisionRigidActor[i] = nullptr;
		addCollisionRigidActor[i] = nullptr;
		activatedCheckpointRigidActorFront[i] = nullptr;
	}

	int drawWidth = width / numberOfActors;

	// 60° Open angle, aspect, near, far
	proj = glm::perspective(100.0f, (float)width / (float)height, 0.1f, 200.0f);

	// Shadow Maps
	shadowShader = new Shader(
		"Shader/Shadow.vert",
		"Shader/Shadow.frag");

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, drawWidth, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	// No color output in the bound framebuffer, only depth.
	glDrawBuffer(GL_NONE);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Couldn't load frame buffer ";
		glfwTerminate();
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set up floating point framebuffer to render scene to
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glGenTextures(2, colorBuffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, drawWidth, height, 0, GL_BGR, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
			);
	}

	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, drawWidth, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Couldn't load frame buffer ";
		glfwTerminate();
		system("PAUSE");
		exit(EXIT_FAILURE);
	}


	// Ping pong framebuffer for 
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, drawWidth, height, 0, GL_BGR, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// Also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	// Set up floating point framebuffer to render scene to
	glGenFramebuffers(1, &combineFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, combineFBO);

	glGenTextures(1, &combineBuffer);
	glBindTexture(GL_TEXTURE_2D, combineBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, drawWidth, height, 0, GL_BGR, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, combineBuffer, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void StepPhysX(float time_delta)					//Stepping PhysX
{
	gScene->simulate(time_delta > 0.0f ? time_delta : 0.001f);	//Advances the simulation by 'gTimeStep' time
	gScene->fetchResults(true);		//Block until the simulation run is completed


	for (int j = 0; j < numberOfActors; j++)
	{
		if (activatedCollisionRigidActor[j] != addCollisionRigidActor[j])
		{
			for (int i = 0; i < ringCount; i++)
			{
				if (addCollisionRigidActor[j] == ring[i]->activateArea)
				{
					if (activatedCollisionRigidActor[j] != nullptr) {
						gScene->removeActor(*activatedCollisionRigidStatic[j]);
					}

					//cout << "Hallo triggerActor" << endl;
					activatedCollisionRigidStatic[j] = ring[i]->actor;
					gScene->addActor(*ring[i]->actor);

					activatedCollisionRigidActor[j] = addCollisionRigidActor[j];
				}
			}
		}
		if (speedCollected[j])
		{
			actors[j]->excelerateSpeed(1.3);
			speedCube[0]->setNextTimeVisible(glfwGetTime() + 10.0f + rand() % 10);
			cout << "Actor " << (j + 1) << " Speed collected" << actors[j]->getExtraSpeed() << endl;
			speedCollected[j] = false;
		}
	}

	for (int j = 0; j < absolutNumberOfRockets; j++)
	{
		if (activeRockets[j] && rocketHit[j] && !rocketDeleted[j])
		{
			rocketDeleted[j] = true;
			int iselectedActor = j / 5;
			actors[iselectedActor]->excelerateSpeed(0.9f);
			cout << "Player " << (iselectedActor + 1) << " was hit by a rocket " << j << " Speed now: " << actors[iselectedActor]->getExtraSpeed() << endl;
			rockets[j]->release();
		}
	}
}

void draw(Shader* drawShader, mat4x4 view, mat4x4 proj, mat4x4 camera_model)
{
	bool cull = false;
	if (drawShader == renderShader) {
		cull = true;
	}
	// Rockets
	for (int i = 0; i < absolutNumberOfRockets; i++) {
		if (activeRockets[i]) {
			//cout << "Draw Rocket: " << i << std::endl;
			rockets[i]->draw(drawShader, view, proj, camera_model, cull);
		}
	}

	// Rings
	for (int i = 0; i < ringCount; i++)
	{
		// Object
		ring[i]->draw(drawShader, view, proj, camera_model, cull);
	}

	// Actors
		for (int i = 0; i < numberOfActors; i++) {
			actors[i]->draw(drawShader, view, proj, camera_model, cull);
	}

	// Goodies
	for (int i = 0; i < numberOfSpeed; i++)
	{
		if (speedCube[i]->isVisible()) {
			speedCube[i]->draw(drawShader, view, proj, camera_model, cull);
		}
	}

	glDisable(GL_CULL_FACE);
	// World
	if (drawShader == renderShader)
	{
		world->draw(drawShader, view, proj, camera_model, false);
	}
}

void update(float time_delta) // TODO change time_delta to delta_time
{
	// Actors
	for (int i = 0; i < numberOfActors; i++) {
		actors[i]->update(time_delta);
	}

	if (gScene)
		StepPhysX(time_delta);

}

void handleInput(GLFWwindow* window, float time_delta)
{
	// move the rocket forward // TODO put somewhere else
	for (int i = 0; i < absolutNumberOfRockets; i++) {
		if (activeRockets[i]) {
			//cout << "Update Pos of Rocket " << i << ". pos: " << rockets[i]->actor->getGlobalPose().p.x << " " << rockets[i]->actor->getGlobalPose().p.y << " " << rockets[i]->actor->getGlobalPose().p.z << endl;
			//rockets[i]->actor->addForce(rockets[i]->actor->getGlobalPose().rotate(PxVec3(0, 0, 10.0 * time_delta)));
		}
	}

	// camera - actor 0 
	if (glfwGetKey(window, GLFW_KEY_J))
	{
		camera[0]->rotateLeft(time_delta);
	}
	if (glfwGetKey(window, GLFW_KEY_L))
	{
		camera[0]->rotateRight(time_delta);
	}
	if (glfwGetKey(window, GLFW_KEY_I))
	{
		camera[0]->rotateDown(time_delta);
	}
	if (glfwGetKey(window, GLFW_KEY_K))
	{		
		camera[0]->rotateUp(time_delta);
	}

	// actor 0 - rotate
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		actors[0]->actor->addTorque(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, -MOVESPEED * time_delta * actors[0]->getExtraSpeed(), 0)));
	}
	if (glfwGetKey(window, GLFW_KEY_D))
	{
		actors[0]->actor->addTorque(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, MOVESPEED * time_delta * actors[0]->getExtraSpeed(), 0)));
	}
	// actor 0 - move 
	if (glfwGetKey(window, GLFW_KEY_Q))
	{
		actors[0]->actor->addForce(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, -ROTATESPEED * 10 * time_delta * actors[0]->getExtraSpeed(), 0)));
	}
	if (glfwGetKey(window, GLFW_KEY_E))
	{
		actors[0]->actor->addForce(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, ROTATESPEED * 10 * time_delta * actors[0]->getExtraSpeed(), 0)));
	}
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		actors[0]->actor->addForce(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, 0, -ROTATESPEED * 15 * time_delta * actors[0]->getExtraSpeed())));
	}
	if (glfwGetKey(window, GLFW_KEY_S))
	{
		actors[0]->actor->addForce(actors[0]->actor->getGlobalPose().rotate(PxVec3(0, 0, ROTATESPEED * 15 * time_delta * actors[0]->getExtraSpeed())));
	}

	if (numberOfActors > 1)
	{
		// camera - actor 1
		if (glfwGetKey(window, GLFW_KEY_UP))
		{
			camera[1]->rotateDown(time_delta);
			//cout << "Camera 1 move DOWN" << std::endl;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN))
		{
			camera[1]->rotateUp(time_delta);
			//cout << "Camera 1 move UP" << std::endl;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT))
		{
			camera[1]->rotateLeft(time_delta);
			//cout << "Camera 1 move LEFT" << std::endl;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT))
		{
			camera[1]->rotateRight(time_delta);
			//cout << "Camera 1 move RIGHT" << std::endl;
		}

		// actor 1 - rotate
		if (glfwGetKey(window, GLFW_KEY_KP_4))
		{
			// TURN LEFT
			actors[1]->actor->addTorque(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, -MOVESPEED * time_delta * actors[1]->getExtraSpeed(), 0)));
		}
		if (glfwGetKey(window, GLFW_KEY_KP_6))
		{
			// TURN RIGHT
			actors[1]->actor->addTorque(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, MOVESPEED * time_delta * actors[1]->getExtraSpeed(), 0)));
		}
		// actor 1 - move
		if (glfwGetKey(window, GLFW_KEY_KP_8))
		{
			// GO FORWARD
			actors[1]->actor->addForce(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, 0, -ROTATESPEED * 15 * time_delta * actors[1]->getExtraSpeed())));
		}
		if (glfwGetKey(window, GLFW_KEY_KP_5))
		{
			// GO BACK
			actors[1]->actor->addForce(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, 0, ROTATESPEED * 15 * time_delta * actors[1]->getExtraSpeed())));
		}
		if (glfwGetKey(window, GLFW_KEY_KP_7))
		{
			// GO UP
			actors[1]->actor->addForce(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, -ROTATESPEED * 5 * time_delta * actors[1]->getExtraSpeed(), 0)));
		}
		if (glfwGetKey(window, GLFW_KEY_KP_9))
		{
			// GO DOWN
			actors[1]->actor->addForce(actors[1]->actor->getGlobalPose().rotate(PxVec3(0, ROTATESPEED * 5 * time_delta * actors[1]->getExtraSpeed(), 0)));
		}
	}

	// move the rocket forward
	for (int i = 0; i < absolutNumberOfRockets; i++) {
		if (activeRockets[i] && rockets[i]->enableDraw) {
			//cout << "Update Pos of Rocket " << i << ". pos: " << rockets[i]->actor->getGlobalPose().p.x << " " << rockets[i]->actor->getGlobalPose().p.y << " " << rockets[i]->actor->getGlobalPose().p.z << endl;
			rockets[i]->actor->addForce(rockets[i]->actor->getGlobalPose().rotate(PxVec3(0, 0, -100.0 * time_delta)));
		}
	}

// ESC - close game
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, true);
	}

	// F1 - Help
	if (glfwGetKey(window, GLFW_KEY_F1)){
		if (!CGUE_F1_PRESSED)
		{
			cout << endl << "HOW TO PLAY:" << endl;
			cout << "Try to pass the parkour of ring faster than your opponent." << endl;
			cout << "-------------------" << endl;
			cout << "CONTROLS:" << endl;
			cout << "PLAYER 1 | PLAYER 2    | PLAYER 1/2 GAMEPAD     | FUNCTION" << endl;
			cout << "W        | NUM 8       | Joystick (left) up     | Accelerate" << endl;
			cout << "S        | NUM 5       | Joystick (left) down   | Decelerate/Backwards" << endl;
			cout << "A        | NUM 4       | Joystick (left) left   | Pan left" << endl;
			cout << "D        | NUM 6       | Joystick (left) right  | Pan right" << endl;
			cout << "I        | Arroy up    | Joystick (right) up    | Camera up" << endl;
			cout << "K        | Arroy down  | Joystick (right) down  | Camera down" << endl;
			cout << "J        | Arroy left  | Joystick (right) left  | Camera left" << endl;
			cout << "L        | Arroy right | Joystick (right) right | Camera right" << endl;
			cout << "Q        | NUM 7       | LT                     | Go up" << endl;
			cout << "E        | NUM 9       | RT                     | Go down" << endl;
			cout << "SPACE    | +           | A                      | Shoot Rocket" << endl;
			cout << "-------------------" << endl;
			cout << "F1 - Help" << endl;
			cout << "F2 - Frame Time on/off" << endl;
			cout << "F3 - Wire Frame on/off" << endl;
			cout << "F4 - Textur-Sampling-Quality: Nearest Neighbor/Bilinear" << endl;
			cout << "F5 - Mip Mapping-Quality: Off/Nearest Neighbor/Linear" << endl;
			cout << "F6 - HDR on/off" << endl;
			cout << "F8 - Viewfrustum-Culling on/off" << endl;
			cout << "F9 - Transparency on/off" << endl;
			cout << "ESC - Quit Game" << endl << endl;
		}
		CGUE_F1_PRESSED = true;
	}
	else
	{
		CGUE_F1_PRESSED = false;
	}

	// F2 - Frame Time on/off
	if (glfwGetKey(window, GLFW_KEY_F2)){
		if (!CGUE_F2_PRESSED)
		{
			if (!CGUE_DISPLAY_FRAME_TIME)
			{
				cout << "Frame Time on" << endl;
				CGUE_DISPLAY_FRAME_TIME = true;
			}
			else
			{
				cout << "Frame Time off" << endl;
				CGUE_DISPLAY_FRAME_TIME = false;
			}
		}
		CGUE_F2_PRESSED = true;
	}
	else
	{
		CGUE_F2_PRESSED = false;
	}

	// F3 - Wire Frame on/off
	if (glfwGetKey(window, GLFW_KEY_F3)){
		if (!CGUE_F3_PRESSED)
		{
			if (CGUE_RENDER == GL_TRIANGLES)
			{
				cout << "Wire Frame on" << endl;
				CGUE_RENDER = GL_LINE_STRIP;// GL_LINES;
			}
			else
			{
				cout << "Wire Frame off" << endl;
				CGUE_RENDER = GL_TRIANGLES;
			}
		}
		CGUE_F3_PRESSED = true;
	}
	else
	{
		CGUE_F3_PRESSED = false;
	}

	//  F4 - Textur-Sampling-Quality: Nearest Neighbor/Bilinear
	if (glfwGetKey(window, GLFW_KEY_F4)){
		if (!CGUE_F4_PRESSED)
		{
			if (!TEXTURE_SAMPLING_QUALITY)
			{
				cout << "Textur-Sampling-Quality: Bilinear" << endl;
				TEXTURE_SAMPLING_QUALITY = true;
			}
			else
			{
				cout << "Textur-Sampling-Quality: Nearest Neighbor" << endl;
				TEXTURE_SAMPLING_QUALITY = false;
			}
		}
		CGUE_F4_PRESSED = true;
	}
	else
	{
		CGUE_F4_PRESSED = false;
	}


	// F5 - Mip Mapping-Quality: Off/Nearest Neighbor/Linear
	if (glfwGetKey(window, GLFW_KEY_F5)){
		if (CGUE_F5_PRESSED == false)
		{
			if (MIP_MAPPING_QUALITY == 0)
			{
				cout << "Mip Mapping-Quality: Nearest Neighbor" << endl;
				MIP_MAPPING_QUALITY = 1;
			}
			else if (MIP_MAPPING_QUALITY == 1)
			{
				cout << "Mip Mapping-Quality: Linear" << endl;
				MIP_MAPPING_QUALITY = 2;
			}
			else
			{
				cout << "Mip Mapping-Quality: Off" << endl;
				MIP_MAPPING_QUALITY = 0;
			}
		}
		CGUE_F5_PRESSED = true;
	}
	else
	{
		CGUE_F5_PRESSED = false;
	}


	// F - HDR
	if (glfwGetKey(window, GLFW_KEY_F6)){
		if (CGUE_F6_PRESSED == false)
		{
			if (EFFECT_HDR_ENABLED)
			{
				cout << "HDR: off" << endl;
				EFFECT_HDR_ENABLED = false;
			}
			else
			{
				cout << "HDR: on" << endl;
				EFFECT_HDR_ENABLED = true;
			}
		}
		CGUE_F6_PRESSED = true;
	}
	else
	{
		CGUE_F6_PRESSED = false;
	}


	// F8 - Viewfrustum-Culling on/off
	if (glfwGetKey(window, GLFW_KEY_F8)){
		if (CGUE_F8_PRESSED == false)
		{
			if (!VIEWFRUSTUM_CULLING)
			{
				cout << "Viewfrustum-Culling on" << endl;
				VIEWFRUSTUM_CULLING = true;
			}
			else
			{
				cout << "Viewfrustum-Culling off" << endl;
				VIEWFRUSTUM_CULLING = false;
			}
		}
		CGUE_F8_PRESSED = true;
	}
	else
	{
		CGUE_F8_PRESSED = false;
	}

	// F9 - Transparency on / off
	if (glfwGetKey(window, GLFW_KEY_F9)){
		if (CGUE_F9_PRESSED == false)
		{
			if (!TRANSPARENCY)
			{
				cout << "Transparency on" << endl;
				TRANSPARENCY = true;
			}
			else
			{
				cout << "Transparency off" << endl;
				TRANSPARENCY = false;
			}
		}
		CGUE_F9_PRESSED = true;
	}
	else
	{
		CGUE_F9_PRESSED = false;
	}

	/*
	PxVec3 pos = actor->actor->getGlobalPose().p;
	PxVec3 velo = actor->actor->getLinearVelocity();
	PxVec3 ang = actor->actor->getAngularVelocity();

	//cout << "pos: " << pos.x << ", " << pos.y << ", " << pos.z << " velo: " << velo.x << ", " << velo.y << ", " << velo.z << " rot: " << ang.x << ", " << ang.y << ", " << ang.z << endl;
	*/
	
}

// Controlls with Joistick
// 0 for player 1; 1 for player 2, ...
void handleInputJOISTICK(GLFWwindow* window, float time_delta, int numberOfJoistick)
{
	int present = glfwJoystickPresent(numberOfJoistick);
	//numberOfJoistick--;

	if (present == 1) {
		int axesCount;
		const float *axes = glfwGetJoystickAxes(numberOfJoistick, &axesCount);
		
		// Left Stick: Y Axis: LEFT = -1
		// cout << "Left Stick Y Axis: "      << axes[0] << endl;
		// Left Stick: X Axis: 
		// cout << "Left Stick X Axis: "      << axes[1] << endl;
		// // TRIGGER: Rechts -1; Links +1
		// cout << "RT (>0) LT (<0) Trigger " << axes[2] << endl;
		// Right Stick: X Axis: UP -1; DOWN 1
		// cout << "Right Stick X Axis: "     << axes[3] << endl;
		// Right Stick: Y Axis:
		// cout << "Left Trigger/L2: "        << axes[4] << endl;

		// Left Stick: Y Axis: LEFT = -1

		// Left Stick: X Axis: 
		
		if (axes[0] >= 0.3)
		{
			actors[numberOfJoistick]->actor->addTorque(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, MOVESPEED * time_delta * axes[0] * actors[numberOfJoistick]->getExtraSpeed(), 0)));
		}
		else if (axes[0] <= -0.3) {
			actors[numberOfJoistick]->actor->addTorque(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, MOVESPEED * time_delta * axes[0] * actors[numberOfJoistick]->getExtraSpeed(), 0)));
			
		}
		if (axes[1] >= 0.25) {
			actors[numberOfJoistick]->actor->addForce(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, 0, ROTATESPEED * 15 * time_delta * axes[1] * actors[numberOfJoistick]->getExtraSpeed())));
		}
		else if (axes[1] <= -0.25) {
			actors[numberOfJoistick]->actor->addForce(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, 0, ROTATESPEED * 15 * time_delta * axes[1] * actors[numberOfJoistick]->getExtraSpeed())));
			
		}
		// TRIGGER: Rechts -1; Links +1
		if (axes[2] <= -0.8){
			actors[numberOfJoistick]->actor->addForce(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, ROTATESPEED * 10 * time_delta * -axes[2] * actors[numberOfJoistick]->getExtraSpeed(), 0)));
		}
		else if (axes[2] >= 0.8){
			actors[numberOfJoistick]->actor->addForce(actors[numberOfJoistick]->actor->getGlobalPose().rotate(PxVec3(0, ROTATESPEED * 10 * time_delta * -axes[2] * actors[numberOfJoistick]->getExtraSpeed(), 0)));
		}
		// Right Stick: X Axis: UP -1; DOWN 1
		if (axes[3] >= 0.25) {
			camera[numberOfJoistick]->rotateUp(time_delta * (axes[3]));
		}
		else if (axes[3] <= -0.25) {
			camera[numberOfJoistick]->rotateDown(time_delta *  (-axes[3]));
		}
		// Right Stick: X Axis: UP -1; DOWN 1
		if (axes[4] >= 0.25) {
			camera[numberOfJoistick]->rotateRight(time_delta * (axes[4]));
		}
		else if (axes[4] <= -0.25) {
			camera[numberOfJoistick]->rotateLeft(time_delta * (-axes[4]));
		}

		int buttonCount;
		const unsigned char *buttons = glfwGetJoystickButtons(numberOfJoistick, &buttonCount);

		/*
		// actor 0 - SHOOT
		if (glfwGetKey(window, GLFW_KEY_SPACE)){
			if (!CGUE_SHOT_ACTOR1_PRESSED)
			{
				//cout << "Actor 0 hat GESCHOSSEN" << std::endl;
				shoot(0);
			}
			CGUE_SHOT_ACTOR1_PRESSED = true;
		}
		else {
			CGUE_SHOT_ACTOR1_PRESSED = false;
		}
		// actor 1 - SHOOT
		if (glfwGetKey(window, GLFW_KEY_KP_ADD)){
			if (!CGUE_SHOT_ACTOR2_PRESSED)
			{
				//cout << "Actor 1 hat GESCHOSSEN" << std::endl;
				shoot(1);
			}
			CGUE_SHOT_ACTOR2_PRESSED = true;
		}
		else {
			CGUE_SHOT_ACTOR2_PRESSED = false;
		}
		*/
		if (numberOfJoistick == 0)
		{
			// actor 0 - SHOOT
			if (GLFW_PRESS == buttons[0] || glfwGetKey(window, GLFW_KEY_SPACE)) {
				//cout << "Button A is pressed!" << endl;
				if (!CGUE_SHOT_ACTOR1_PRESSED) {
					shoot(0);
				}
				CGUE_SHOT_ACTOR1_PRESSED = true;
			}
			else {
				CGUE_SHOT_ACTOR1_PRESSED = false;
			}
		}

		if (numberOfJoistick == 1)
		{
			// actor 1 - SHOOT
			if (GLFW_PRESS == buttons[0] || glfwGetKey(window, GLFW_KEY_KP_ADD)) {
				//cout << "Button A is pressed!" << endl;
				if (!CGUE_SHOT_ACTOR2_PRESSED) {
					shoot(1);
				}
				CGUE_SHOT_ACTOR2_PRESSED = true;
			}
			else {
				CGUE_SHOT_ACTOR2_PRESSED = false;
			}
		}

		if (GLFW_PRESS == buttons[1]) {
			//cout << "Button B is pressed!" << endl;
		}
		if (GLFW_PRESS == buttons[2]) {
			//cout << "Button X is pressed!" << endl;
		}
		if (GLFW_PRESS == buttons[3]) {
			//cout << "Button Y is pressed!" << endl;
		}	
	}
	 else {
		 // actor 0 - SHOOT
		 if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			 //cout << "Button A is pressed!" << endl;
			 if (!CGUE_SHOT_ACTOR1_PRESSED) {
				 shoot(0);

			 }
			 CGUE_SHOT_ACTOR1_PRESSED = true;
		 }
		 else {
			 CGUE_SHOT_ACTOR1_PRESSED = false;
		 }

		 // actor 1 - SHOOT
		 if (glfwGetKey(window, GLFW_KEY_KP_ADD)) {
			 //cout << "Button A is pressed!" << endl;
			 if (!CGUE_SHOT_ACTOR2_PRESSED) {
				 shoot(1);
			 }
			 CGUE_SHOT_ACTOR2_PRESSED = true;
		 }
		 else {
			 CGUE_SHOT_ACTOR2_PRESSED = false;
		 }
	 }

	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);
}

void shoot(int actorID) {
	// find the first index for the rocket to shoot
	// TODO add max amount of shoots
	int numberOfShootsPerPlayer = absolutNumberOfRockets / numberOfActors;
	int newRocketPos = actorID * numberOfShootsPerPlayer;
	int upperBound = (actorID + 1) * numberOfShootsPerPlayer;
	for (; activeRockets[newRocketPos] && newRocketPos <upperBound; newRocketPos++){}
 	if (newRocketPos < upperBound) {
		activeRockets[newRocketPos] = true;
		cout << "Rocket " << newRocketPos <<" is active!"<< endl;
		rockets[newRocketPos]->initActor(actors[actorID]);
	}
}