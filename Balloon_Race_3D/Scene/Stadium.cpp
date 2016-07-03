#include "Stadium.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../Resources/Const.hpp"

using namespace glm;

Stadium::Stadium(Shader* shader)
{
	//const string extraString = "Balloon_Race_3D/";
	const string extraString = "";
	/*
	Shader* shader = new Shader(
		extraString + "Shader/Basic.vert",
		extraString + "Shader/Basic.frag");*/

	//const std::string& displayFile = extraString + "Datensatz/cubeFullTextureDesert.dae";
	const std::string& displayFile = extraString + "Datensatz/cubeFullTexture.dae";
	const std::string& collisionFile = extraString + "Datensatz/cubeFullTexture.dae";

	mat4& matrix = mat4(1.0f);

	float pi = 3.14159265358979323846f;
	mat4& initTrans = mat4(1.0f);
	initTrans = scale(initTrans, vec3(80, 20, 80));
	initTrans = rotate(initTrans, pi / 2.0f, vec3(1, 0, 0));

	Geometry::init(displayFile, collisionFile, matrix, shader, initTrans);

	shader->useShader();

	enableLighting = 0;
	enableGlow = 0;
	enableDraw = true;
	alpha = 1;
}

Stadium::~Stadium()
{

}

void Stadium::initActor()
{
	PxVec3 position(0, -10, 0);
	actor = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(position));

	//gScene->addActor(*actor);

	PxMaterial *aMaterial = gPhysicsSDK->createMaterial(0.0f, 0.0f, 0.0f);


	// LEFT PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(1, 0, 0), 40);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}

	// RIGHT PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(-1, 0, 0), 40);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}

	// BACK PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(0, 0, -1), 40);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}

	// FRONT PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(0, 0, 1), 40);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}

	// BOTTOM PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(0, -1, 0), 9);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}

	// TOP PLANE
	{
		PxPlane planeEq = PxPlane(PxVec3(0, 1, 0), 30);

		PxRigidStatic* wall = Geometry::gPhysicsSDK->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
		PxTransform transform = PxTransformFromPlaneEquation(planeEq);
		PxPlaneGeometry* planeGeometry = new PxPlaneGeometry();
		PxShape* planeShape = wall->createShape(*planeGeometry, *aMaterial, transform);
		planeShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		gScene->addActor(*wall);
	}
	
	
}

mat4x4 Stadium::getGlobalPose()
{
	return pxMatToGlm(PxMat44(actor->getGlobalPose()));
}