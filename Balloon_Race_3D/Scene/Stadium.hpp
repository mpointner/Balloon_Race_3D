#include "Geometry.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace cgue;
using namespace std;

namespace cgue
{
	class Stadium : public Geometry
	{
	public:

		Stadium(Shader* shader);
		virtual ~Stadium();
		void initActor();
		mat4x4 getGlobalPose();

		PxRigidStatic* actor;
	private:

	};
};