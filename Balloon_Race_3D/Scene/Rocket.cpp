#include <PhysX\PxPhysicsAPI.h>		//Single header file to include all features of PhysX API 
#include <glm/gtc/type_ptr.hpp>

#include "Rocket.hpp"
#include "../Resources/Const.hpp"

// CPU representation of a particle
struct Particle{
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 10000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle(){

	for (int i = LastUsedParticle; i<MaxParticles; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

Rocket::Rocket(Shader* shader, float sposX, float sposY, float sposZ,
							   float srotX, float srotY, float srotZ) {
	// Position
	this->posx = sposX;
	this->posy = sposY;
	this->posz = sposZ;
	// Rotation
	this->rotX = srotX;
	this->roty = srotY;
	this->rotZ = srotZ;
	
	const std::string& displayFile = "Datensatz/rocket.dae";
	const std::string& collisionFile = "Datensatz/cube.dae"; // TODO why cube

	glm::mat4& matrix = glm::mat4(1.0f);

	glm::mat4& initTrans = glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1, 0, 0));

	// Rotation around the  - Axis
	initTrans = glm::rotate(initTrans, srotX / 180.0f * PI, glm::vec3(1, 0, 0));
	// Rotation around the  - Axis
	initTrans = glm::rotate(initTrans, srotY / 180.0f * PI, glm::vec3(0, 0, 1));
	// Rotation around the  - Axis
	initTrans = glm::rotate(initTrans, srotZ / 180.0f * PI, glm::vec3(0, 1, 0));

	initTrans = glm::scale(initTrans, glm::vec3(0.02, 0.02, 0.02));

	Geometry::init(displayFile, collisionFile, matrix, shader, initTrans);

	// Shader
	enableLighting = 1;
	enableGlow = 0;
	enableDraw = true;
	alpha = 1;

	particleShader = new Shader("Shader/Particle.vert", "Shader/Particle.frag");

	g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);


	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
}

Rocket::~Rocket()
{
	delete particleShader; particleShader = nullptr;
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	if (actor != nullptr)
	{
		actor->release(); actor = nullptr;
	}
}

void Rocket::release()
{
	if (actor != nullptr)
	{
		cout << "removed" << endl;
		enableDraw = false;
		gScene->removeActor(*actor);
		actor->release();
	}
}

void Rocket::initActor(Actor1* actorObj)
{
	mat4x4 transform = pxMatToGlm(actorObj->actor->getGlobalPose());
	transform = translate(transform, vec3(0, 2, -2.0));
	
	actor = Geometry::gPhysicsSDK->createRigidDynamic(PxTransform(glmToPxMat(transform)));

	PxMaterial *aMaterial = gPhysicsSDK->createMaterial(10.0f, 10.0f, 10.0f);
	mat4x4 rot = rotate(mat4x4(1.0f), roty / 180.0f * PI, vec3(0, 0.1, 0));
	PxShape* aCapsuleShape = actor->createShape(PxSphereGeometry(1), *aMaterial, PxTransform(glmToPxMat(rot)));
	
	aCapsuleShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	aCapsuleShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	
	gScene->addActor(*actor);

}

mat4x4 Rocket::getGlobalPose()
{
	return pxMatToGlm(PxMat44(actor->getGlobalPose()));
}

void Rocket::drawParticles(float delta, mat4x4 view, mat4x4 proj)
{
	if (!enableDraw) return;

	GLuint programID = particleShader->programHandle;

	GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");


	glm::mat4 ProjectionMatrix = proj;
	glm::mat4 ViewMatrix = view;
	glm::mat4 ModelMatrix = getGlobalPose();
	PxVec3 pos = actor->getGlobalPose().p;

	//cout << "Rocketpos: " << pos.x << " " << pos.y << endl;
	if (abs(pos.x) > maxWidth || abs(pos.y) > maxWidth)
	{
		cout << "removed" << endl;
		enableDraw = false;
		gScene->removeActor(*actor);
		actor->release(); actor = nullptr;
	}

	glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

	glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

	int newparticles = (int)(delta*1000.0);
	if (newparticles > (int)(0.016f*1000.0))
		newparticles = (int)(0.016f*1000.0);

	for (int i = 0; i<newparticles; i++){
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 0.3f; // This particle will live 1 seconds.
		ParticlesContainer[particleIndex].pos = glm::vec3(0, 0, 0.05);

		float spread = 1.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 10.0f);
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
			);

		ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;

		int white = rand() % 255;

		ParticlesContainer[particleIndex].r = 255;
		ParticlesContainer[particleIndex].g = 255;
		ParticlesContainer[particleIndex].b = 255;
		ParticlesContainer[particleIndex].a = rand() % 100;

		ParticlesContainer[particleIndex].size = (rand() % 100) / 2000.0f + 0.1f;
	}

	vec3 dest(220, 146, 2);


	// Simulate all particles
	int ParticlesCount = 0;
	for (int i = 0; i<MaxParticles; i++){

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f){

			// Decrease life
			p.life -= delta;
			if (p.life > 0.0f){

				p.speed += glm::vec3(0, 0, 0) * delta * 0.5f;
				p.pos += p.speed * delta;
				p.cameradistance = glm::length(p.pos - CameraPosition);
				
				// Fill the GPU buffer
				g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
				g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
				g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

				g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				p.r = p.r > dest.r ? p.r - (p.r - dest.r) / 10 : dest.r;
				p.g = p.g > dest.g ? p.g - (p.g - dest.g) / 10 : dest.g;
				p.b = p.b > dest.b ? p.b - (p.b - dest.b) / 10 : dest.b;

				g_particule_color_data[4 * ParticlesCount + 0] = p.r;
				g_particule_color_data[4 * ParticlesCount + 1] = p.g;
				g_particule_color_data[4 * ParticlesCount + 2] = p.b;
				g_particule_color_data[4 * ParticlesCount + 3] = p.a;

			}
			else{
				// Died particles
				p.cameradistance = -1.0f;
			}

			ParticlesCount++;

		}
	}

	SortParticles();

	glBindVertexArray(VertexArrayID);

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat)* 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte)* 4, g_particule_color_data);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(programID);

	glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);

	// glDrawArraysInstanced is faster that glDrawArrays
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

}