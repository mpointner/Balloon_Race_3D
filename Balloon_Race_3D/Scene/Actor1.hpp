#pragma once

#include "Geometry.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "../Resources/Shader.hpp"

using namespace std;

namespace cgue
{
	class Actor1 : public Geometry
	{
	public:
		Actor1(Shader* shader, float sposx, float sposy, float sposz, float sroty);
		glm::vec3 getPosition();
		Actor1(Shader* shader);
		virtual ~Actor1();
		void resetPosition();
		void initActor();
		PxRigidDynamic* actor;
		mat4x4 getGlobalPose();

		float extraSpeed = 1.0f;

		// if factor > 1 actor excelerate
		// if factor = 1 speed of actor stays the same
		// if factor < 1 and > 0 actor slows down
		// negativ values are illegal
		void excelerateSpeed(float factor);

		// returns the extraspeed of the Object
		// return value > 0 
		float getExtraSpeed();

	private:
	};
};