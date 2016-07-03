#include <glm/gtc/type_ptr.hpp>

#include "Actor1.hpp"
#include "../Resources/Const.hpp"

using namespace cgue;
using namespace glm;

Actor1::Actor1(Shader* shader, float sposx, float sposy, float sposz, float sroty)
{
	this->posx = sposx;
	this->posy = sposy;
	this->posz = sposz;
	this->roty = sroty;
	const std::string& displayFile = "Datensatz/balloon.dae";
	const std::string& collisionFile = "Datensatz/balloonConvex.dae";

	mat4& matrix = mat4(1.0f);

	mat4& initTrans = mat4(1.0f);
	initTrans = rotate(initTrans, PI / 2.0f, vec3(1, 0, 0));
	initTrans = rotate(initTrans, PI / 2.0f, vec3(0, 0, 1));
	initTrans = translate(initTrans, vec3(0, 0, 0));

	//matrix = rotate(matrix, sroty / 180.0f * PI, glm::vec3(0, 0, 1));

	init(displayFile, collisionFile, matrix, shader, initTrans);

	enableLighting = 1;
	enableGlow = 0;
	enableDraw = true;
	alpha = 1;
}

Actor1::~Actor1(){
	// TODO delete data??
}

void Actor1::resetPosition()
{
	modelMatrix = mat4(1.0f);
}

void Actor1::initActor()
{
	//PxVec3 position(posx, posy, posz);

	mat4x4 matTrans = mat4x4(1.0);
	matTrans = rotate(matTrans, roty / 180.0f * PI, glm::vec3(0, 1, 0));
	matTrans = translate(matTrans, vec3(posx, posy, posz));

	PxTransform transform = PxTransform(glmToPxMat(matTrans));
	actor = Geometry::gPhysicsSDK->createRigidDynamic(transform);

	PxMaterial *aMaterial = gPhysicsSDK->createMaterial(2.0f, 2.0f, 0.0f);
	//PxMaterial *aMaterial = gPhysicsSDK->createMaterial(10.0f, 10.0f, 10.0f);
	mat4x4 rot = rotate(mat4x4(1.0f), roty / 180.0f * PI, vec3(0, 1, 0));
	vec4 positionSphere = rot * vec4(0, -0.3, 0, 0);
	PxShape* aCapsuleShape = actor->createShape(PxSphereGeometry(1), *aMaterial, PxTransform(PxVec3(positionSphere.x, positionSphere.y, positionSphere.z)));

	vec4 positionBasket = rot * vec4(0.0, 1.1, 0.0, 0);
	PxShape* xyz = actor->createShape(PxBoxGeometry(0.13, 0.6, 0.13), *aMaterial, PxTransform(PxVec3(positionBasket.x, positionBasket.y, positionBasket.z)));

	aCapsuleShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	actor->setMass(100);
	actor->setRigidDynamicFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	actor->setLinearDamping(0.3);
	actor->setAngularDamping(0.2);
	actor->setMaxAngularVelocity(0.3);
	actor->setMaxDepenetrationVelocity(0.3);

	gScene->addActor(*actor);

	PxD6Joint* mJoint = PxD6JointCreate(*gPhysicsSDK, actor, PxTransform::createIdentity(), nullptr, actor->getGlobalPose());
	
	mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE); // Movement on X allowed
	mJoint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE); // Movement on Y allowed
	mJoint->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE); // Movement on Z allowed
	
	//This will lock a rotation around the X axis :
	mJoint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);

	//For the Y rotation axis :
	mJoint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);

	//For the Z rotation axis :
	mJoint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
}

mat4x4 Actor1::getGlobalPose()
{
	return pxMatToGlm(PxMat44(actor->getGlobalPose()));
}

void Actor1::excelerateSpeed(float factor) {
	if (factor > 0.5) {
		extraSpeed *= factor;
	}
}

float Actor1::getExtraSpeed() {
	return extraSpeed;
}