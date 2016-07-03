#include <glm/gtc/matrix_transform.hpp>
#include "Geometry.hpp"
#include "../Resources/Shader.hpp"

using namespace cgue;
using namespace std;

namespace cgue
{
	class Ring : public Geometry
	{
	public:
		// Constructor
		Ring(Shader* shader, float posX, float posY, float posZ, float rotY);
		// Destructor
		virtual ~Ring();
		void initActor();
		void setActiveNext(bool active);
		mat4x4 getGlobalPose();
		bool passRing(int actorID);
		void resetCounter(int actorID);
		void Ring::setLastRing(bool value);

		PxRigidStatic* actor;
		PxRigidStatic* activateArea;
		PxRigidStatic* triggerFront;
		PxRigidStatic* triggerBack;

		vec3 defaultColor;
		vec3 activeColor;
		vec3 wonColor;

		// Saves how often a player has passed
		// number of position of int is the ID of the player
		int passedCounter[2];
		bool lastRing;
		int roundsToGo; // TODO could be static
	private:

	};
};