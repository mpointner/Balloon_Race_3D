#include <PhysX\PxPhysicsAPI.h>		//Single header file to include all features of PhysX API 
#include <glm/gtc/type_ptr.hpp>

#include "Ring.hpp"
#include "../Resources/Const.hpp"

using namespace physx;
using namespace glm;


Ring::Ring(Shader* shader, float sposX, float sposY, float sposZ, float srotY)
{
	this->posx = sposX;
	this->posy = sposY;
	this->posz = sposZ;
	this->roty = srotY;

	const std::string& displayFile = "Datensatz/ring.dae";
	const std::string& collisionFile = "Datensatz/cube.dae"; // TODO why cube

	glm::mat4& matrix = glm::mat4(1.0f);

	glm::mat4& initTrans = glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1, 0, 0));
	initTrans = glm::rotate(initTrans, srotY / 180.0f * PI, glm::vec3(0, 0, 1));

	Geometry::init(displayFile, collisionFile, matrix, shader, initTrans);

	// Shader
	shader->useShader();
	enableLighting = 1;
	enableGlow = 0;
	enableDraw = true;
	alpha = 1;

	// Gameplay
	lastRing = false;
	int numberOfPlayers = 2;
	for (int i = 0; i < numberOfPlayers; i++) {
		passedCounter[i] = 0;
	}
	lastRing = false;

	// TODO should be implemented somewhere else 
	roundsToGo = 2;

	defaultColor = vec3(0.3, 0.3, 0.3);
	activeColor = vec3(0.527, 0.002, 0.003) * 10.0f;
	wonColor;
}

Ring::~Ring()
{
	// TODO delete data??
}

void Ring::setActiveNext(bool active)
{
	if (active)
	{
		//mesh[0]->diffuse = vec3(1, 1, 1);
		mesh[0]->diffuse = activeColor;
		enableGlow = 1;
	}
	else
	{
		mesh[0]->diffuse = defaultColor;
		enableGlow = 0;
	}
}

void Ring::initActor()
{
	PxVec3 position(posx, posy, posz);
	actor = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(position));

	PxMaterial *aMaterial = gPhysicsSDK->createMaterial(0.0f, 0.0f, 0.0f);

	// Stange
	{
		
		float radius = 0.2f;
		float halfHeight = 6.5f - radius;

		mat4x4 mat(1.0f);
		mat4x4 matGeo = mat;

		PxCapsuleGeometry* stange = new PxCapsuleGeometry(radius, halfHeight);
		PxShape* aCapsuleShape = actor->createShape(*stange, *aMaterial);

		mat = rotate(mat, PI / 2.0f, vec3(0, 0, 1));
		matGeo = rotate(matGeo, PI / 2.0f, vec3(0, 0, 1));
		mat = translate(mat, vec3(3.5, 0, 0));
		matGeo = translate(matGeo, vec3(3.5, 0, 0));

		matGeo = scale(matGeo, vec3(halfHeight + radius, radius, radius));

		aCapsuleShape->setLocalPose(PxTransform(glmToPxMat(mat)));
		test->setMeshTrans(matGeo);

		aCapsuleShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		
	}

	// Kugeln
	{
		for (float angle = 0; angle < 2 * PI; angle += PI / 10.0f)
		{
			float radius = 0.1f;

			mat4x4 mat(1.0f);
			mat4x4 matGeo = mat;

			PxSphereGeometry* sphere = new PxSphereGeometry(radius);
			PxShape* aSpheresShape = actor->createShape(*sphere, *aMaterial);

			vec4 pos = vec4(0, 2, 0, 0);
			mat4x4 rot = rotate(mat4x4(1.0f), -roty / 180.0f * PI, vec3(0, 1, 0));
			pos = rotate(rot, angle, vec3(1, 0, 0)) * pos;
			pos.y -= 5;

			//cout << pos.y << ", " << pos.z << ", " << pos.w << ", " << endl;
			//printMatGeometry(mat);

			mat = translate(mat, vec3(pos.x, pos.y, pos.z));
			matGeo = translate(matGeo, vec3(pos.x, pos.y, pos.z));

			matGeo = scale(matGeo, vec3(radius, radius, radius));

			aSpheresShape->setLocalPose(PxTransform(glmToPxMat(mat)));
			test->setMeshTrans(matGeo);

			aSpheresShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		}
	}

	{
		triggerFront = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(position));

		PxBoxGeometry* sphere = new PxBoxGeometry(1, 1, 1);

		mat4x4 rot = rotate(mat4x4(1.0f), roty / 180.0f * PI, vec3(0, 1, 0));
		vec4 pos = rot * vec4(2.5, -5, 0, 0);
		PxShape* aTriggerShape = triggerFront->createShape(*sphere, *aMaterial, PxTransform(PxVec3(pos.x, pos.y, pos.z)));

		aTriggerShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);	//flagged to disable shape collision
		aTriggerShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);		//flagged as trigger shape

		gScene->addActor(*triggerFront);
	}

	{
		triggerBack = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(position));

		PxBoxGeometry* sphere = new PxBoxGeometry(1, 1, 1);

		mat4x4 rot = rotate(mat4x4(1.0f), roty / 180.0f * PI, vec3(0, 1, 0));
		vec4 pos = rot * vec4(-2.5, -5, 0, 0);
		PxShape* aTriggerShape = triggerBack->createShape(*sphere, *aMaterial, PxTransform(PxVec3(pos.x, pos.y, pos.z)));

		aTriggerShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);	//flagged to disable shape collision
		aTriggerShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);		//flagged as trigger shape

		gScene->addActor(*triggerBack);
	}

	activateArea = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(position));
	//activateArea->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);

	PxBoxGeometry* box = new PxBoxGeometry(1, 10, 3);
	PxShape* boxShape = activateArea->createShape(*box, *aMaterial);

	mat4x4 mat(1.0f);
	mat = rotate(mat, roty / 180.0f * PI, vec3(0, 1, 0));
	boxShape->setLocalPose(PxTransform(glmToPxMat(mat)));

	boxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);	//flagged to disable shape collision
	boxShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);		//flagged as trigger shape

	gScene->addActor(*activateArea);


	mat4x4 matGeo(1.0f);

	matGeo = scale(matGeo, vec3(1, 10, 3));

	test->setMeshTrans(matGeo);
}

mat4x4 Ring::getGlobalPose()
{
	return pxMatToGlm(PxMat44(actor->getGlobalPose()));
}

bool Ring::passRing(int actorID) {
	passedCounter[actorID]++;
	//cout << "Counter: " << passedCounter[actorID] << endl; // passedCounter is wrong
	return lastRing && passedCounter[actorID] >= roundsToGo;
}

void Ring::resetCounter(int actorID) {
	passedCounter[actorID] = 0;
}

void Ring::setLastRing(bool value) {
	lastRing = value;
}

