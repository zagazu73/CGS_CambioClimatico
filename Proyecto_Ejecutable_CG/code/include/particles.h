#ifndef PARTICLES_H
#define PARTICLES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

struct Particle {
	glm::vec3   p0;
	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec3	acceleration;
	glm::vec3	force;
	float		mass;
};

class Particles 
{
public:
	// Material Attributes
	vector<Particle> particles;

	Particles(unsigned int numParticles) {
		for (unsigned int i = 0; i < numParticles; i++) {
			Particle P;
			P.p0.x = rand() % 50; // posición random de -12 a 12
			P.p0.y = rand() % 25;    // posición random de 0 a 7
			P.p0.z = rand() % 1251 - 625; // posición random de -12 a 12

			P.position = P.p0;
			P.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			P.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
			P.force = glm::vec3(0.0f, 0.0f, 0.0f);
			P.mass = 0.001f;
			
			particles.push_back(P);
		}
	}
	~Particles() {}

	void setGravity(glm::vec3 g) { gravity = g; }
	glm::vec3 getGravity() { return gravity; }

	void UpdatePhysics(float deltaTime) {
		for (int i = 0; i < (int)(particles.size()); i++) {
			Particle *P = &particles.at(i);
			if (P->position.y < 0) P->position = P->p0;  // retornamos a la posición original 
			                                          // de la partícula cuando llega al piso
			//if (P->position.y > 30) P->position = P->p0; //Para que empiezen para arriba
			P->force = P->mass * gravity; //+P->mass * glm::vec3(5.0f, 0.0f, 0.0f); // Suma de fuerzas totales actuando en la partícula
			P->acceleration = P->force / P->mass; // segunda ley de Newton
			P->velocity += P->acceleration * deltaTime; // Integral de la aceleración -> velocidad
			P->position += P->velocity * deltaTime;     // Integral de la velocidad -> posición
		}
	}

private:

	glm::vec3 gravity = glm::vec3(0.0f, -0.25f, 0.0f);//Para la gravedad con la que caen

};

#endif
