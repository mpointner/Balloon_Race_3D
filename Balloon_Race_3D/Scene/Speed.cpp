#include <PhysX\PxPhysicsAPI.h>		//Single header file to include all features of PhysX API 
#include <glm/gtc/type_ptr.hpp>

#include "Speed.hpp"
#include "../Resources/Const.hpp"

using namespace physx;
using namespace glm;


Speed::Speed(Shader* shader, float sposX, float sposY, float sposZ, float srotY)
{
	this->posx = sposX;
	this->posy = sposY;
	this->posz = sposZ;
	this->roty = srotY;

	const std::string& displayFile = "Datensatz/speedCube.dae";
	const std::string& collisionFile = "Datensatz/cube.dae"; // TODO why cube

	glm::mat4& matrix = glm::mat4(1.0f);

	glm::mat4& initTrans = glm::mat4(1.0f);

	Geometry::init(displayFile, collisionFile, matrix, shader, initTrans);

	rotation = mat4x4(1.0f);

	// Shader
	shader->useShader();
	enableLighting = 1;
	enableGlow = 1;
	enableDraw = true;
	alpha = 0.6;
}

Speed::~Speed()
{
	// TODO delete data??
}

void Speed::initActor()
{
	PxVec3 position(posx, posy, posz);
	actor = Geometry::gPhysicsSDK->createRigidDynamic(PxTransform(position));

	PxMaterial *aMaterial = gPhysicsSDK->createMaterial(0.0f, 0.0f, 0.0f);
	PxShape* aBoxShape = actor->createShape(PxBoxGeometry(1, 1, 1), *aMaterial);

	aBoxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	aBoxShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	gScene->addActor(*actor);
}

mat4x4 Speed::getGlobalPose()
{

	rotation = rotate(rotation, 0.001f, vec3(0, 1, 0));
	
	return pxMatToGlm(PxMat44(actor->getGlobalPose())) * rotation;
}

void  Speed::setNextTimeVisible(double time){
	nextTimeVisible = time;
	hidden = true;
	gScene->removeActor(*actor);
}
bool Speed::isVisible(){
	bool active = (glfwGetTime() - nextTimeVisible) > 0;
	if (active && hidden) {
		gScene->addActor(*actor);
		hidden = false;
	}
	return active;
}

