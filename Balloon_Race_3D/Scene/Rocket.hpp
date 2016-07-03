#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.hpp"
#include "../Resources/Shader.hpp"
#include "Actor1.hpp"

using namespace cgue;
using namespace std;

namespace cgue
{
	class Rocket : public Geometry
	{
	public:
		// Constructor
		Rocket(Shader* shader, float posX, float posY, float posZ, float srotX, float srotY, float srotZ);
		// Destructor
		virtual ~Rocket();
		void release();

		int maxWidth = 100;

		// Physix
		PxRigidDynamic* actor = nullptr;

		void initActor(Actor1* actor);
		mat4x4 getGlobalPose();

		void drawParticles(float time_delta, mat4x4 view, mat4x4 proj);
	private:

		GLfloat* g_particule_position_size_data;
		GLubyte* g_particule_color_data;

		Shader* particleShader;

		GLuint billboard_vertex_buffer;
		GLuint particles_position_buffer;
		GLuint particles_color_buffer;
		GLuint VertexArrayID;
	};
}