#include <glm/gtc/matrix_transform.hpp>
#include "Geometry.hpp"
#include "../Resources/Shader.hpp"

using namespace cgue;
using namespace std;

namespace cgue
{
	class Speed : public Geometry
	{
	public:
		// Constructor
		Speed(Shader* shader, float posX, float posY, float posZ, float rotY);
		// Destructor
		virtual ~Speed();
		void initActor();
		mat4x4 getGlobalPose();

		mat4x4 rotation;

		PxRigidDynamic* actor;

		void setNextTimeVisible(double time);
		bool isVisible();

		bool hidden = false;

		// apears the next time
		// at this point in time
		double nextTimeVisible = 0;

	private:
	};
};