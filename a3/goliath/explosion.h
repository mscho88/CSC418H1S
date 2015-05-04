/*
 * explosion.c - Simple particle system based explosion demonstration.
 *                   ,
 * Author: Gustav Taxen, nv91-gta@nada.kth.se
 * 
 * Modified by: MinSu Kevin Cho
 * 
 * This program is in the Public Domain. You may freely use, modify
 * and distribute this program.
 *
 *
 * Description:
 *    This program shows an easy way to make simple explosions using
 *    OpenGL. Press SPACE to blow the rotating cube to pieces! Use
 *    the menu to toggle between normalized speed vectors and non-
 *    normalized speed vectors.
 *
 * Theory:
 *    This program uses an extremely simple particle system to create
 *    an explosion effect. Each particle is moved from the origin
 *    towards a random direction and, if activated, a random speed.
 *    The color of the particles are changed from white to orange to
 *    red to create a glowing effect. The GL_POINTS primitive is used
 *    to render the particles.
 *       The debris is similar to the particles, with the addition of
 *    orientation and orientation speed.
 *       A point light source is placed in the center of the explosion.
 * 
 */


#include <time.h>
#include <sys/types.h>

#define NUM_PARTICLES    1000          /* Number of particles  */
#define NUM_DEBRIS       100            /* Number of debris     */

/* A particle */

struct particleData
{
  float   position[3];
  float   speed[3];
  float   color[3];
};
typedef struct particleData    particleData;


/* A piece of debris */

struct debrisData
{
  float   position[3];
  float   speed[3];
  float   orientation[3];        /* Rotation angles around x, y, and z axes */
  float   orientationSpeed[3];
  float   color[3];// = {0.7, 0.7, 0.7};
  float   scale[3];
};
typedef struct debrisData    debrisData;


/* Globals */

particleData     particles[NUM_PARTICLES];
debrisData       debris[NUM_DEBRIS];   
int              fuel = 0;                /* "fuel" of the explosion */

GLfloat  materialAmb[4] = { 0.25, 0.22, 0.26, 1.0 };
GLfloat  materialDif[4] = { 0.63, 0.57, 0.60, 1.0 };
GLfloat  materialSpec[4] = { 0.99, 0.91, 0.81, 1.0 };
GLfloat  materialShininess = 27.8;

int      wantNormalize = 0;   /* Speed vector normalization flag */


/*
 * newSpeed
 *
 * Randomize a new speed vector.
 *
 */

void newSpeed (float dest[3]){
	float    x;
	float    y;
	float    z;
	float    len;

	x = (2.0 * ((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;
	y = (2.0 * ((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;
	z = (2.0 * ((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;

	/*
	* Normalizing the speed vectors gives a "fireball" effect
	*
	*/

	if (wantNormalize){
		len = sqrt (x * x + y * y + z * z);
		if (len){
			x = x / len;
			y = y / len;
			z = z / len;
		}
    }
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
}

void newExplosion (){
	int i;

	for (i = 0; i < NUM_PARTICLES; i++){
		particles[i].position[0] = 15.0;
		particles[i].position[1] = 7.0;
		particles[i].position[2] = 0.0;

		particles[i].color[0] = 1.0;
		particles[i].color[1] = 1.0;
		particles[i].color[2] = 0.5;

		newSpeed (particles[i].speed);
    }

	for (i = 0; i < NUM_DEBRIS; i++){
		debris[i].position[0] = 15.0;
		debris[i].position[1] = 7.0;
		debris[i].position[2] = 0.0;

		debris[i].orientation[0] = 0.0;
		debris[i].orientation[1] = 0.0;
		debris[i].orientation[2] = 0.0;

		debris[i].color[0] = 0.7;
		debris[i].color[1] = 0.7;
		debris[i].color[2] = 0.7;
		
		debris[i].scale[0] = (15.0 * 
				((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;
		debris[i].scale[1] = (15.0 * 
				((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;
		debris[i].scale[2] = (15.0 * 
				((GLfloat) rand ()) / ((GLfloat) RAND_MAX)) - 1.0;

		newSpeed (debris[i].speed);
		newSpeed (debris[i].orientationSpeed);
	}

	fuel = 150;
}


/*
 * display
 *
 * Draw the scene.
 *
 */
void displayExplosion (){
	int    i; 

	if (fuel > 0){
		glPushMatrix ();
			glBegin (GL_POINTS);
				for (i = 0; i < NUM_PARTICLES; i++){
					glColor3fv (particles[i].color);
					glVertex3fv (particles[i].position);
				}
			glEnd ();

		glPopMatrix ();
		
		glNormal3f (0.0, 0.0, 1.0);

		for (i = 0; i < NUM_DEBRIS; i++){
			glColor3fv (debris[i].color);

			glPushMatrix ();

				glTranslatef (debris[i].position[0],
				debris[i].position[1],
				debris[i].position[2]);

				glRotatef (debris[i].orientation[0], 1.0, 0.0, 0.0);
				glRotatef (debris[i].orientation[1], 0.0, 1.0, 0.0);
				glRotatef (debris[i].orientation[2], 0.0, 0.0, 1.0);

				glScalef (debris[i].scale[0],
				debris[i].scale[1],
				debris[i].scale[2]);

				glBegin (GL_TRIANGLES);
					glVertex3f (0.0, 0.5, 0.0);
					glVertex3f (-0.25, 0.0, 0.0);
					glVertex3f (0.25, 0.0, 0.0);
				glEnd ();	  

			glPopMatrix ();
		}
		for (i = 0; i < NUM_PARTICLES; i++){
			particles[i].position[0] += particles[i].speed[0] * 0.5;
			if(fuel >= 60)
				particles[i].position[1] += particles[i].speed[1] * 0.5;
			else if(fuel < 60)
				particles[i].position[1] -= particles[i].speed[1] * 0.15;
			else if(fuel < 30)
				particles[i].position[1] -= particles[i].speed[1] * 0.4;
			particles[i].position[2] += particles[i].speed[2] * 0.5;

			particles[i].color[0] -= 1.0 / 500.0;
			if (particles[i].color[0] < 0.0){
				particles[i].color[0] = 0.0;
			}

			particles[i].color[1] -= 1.0 / 100.0;
			if (particles[i].color[1] < 0.0){
				particles[i].color[1] = 0.0;
			}

			particles[i].color[2] -= 1.0 / 50.0;
			if (particles[i].color[2] < 0.0){
				particles[i].color[2] = 0.0;
			}
		}

		for (i = 0; i < NUM_DEBRIS; i++){
			debris[i].position[0] += debris[i].speed[0] * 1.0;
			if(fuel >= 60)
				debris[i].position[1] += debris[i].speed[1] * 1.0;
			else if(fuel < 60)
				debris[i].position[1] -= debris[i].speed[1] * 0.3;
			else if(fuel < 30)
				debris[i].position[1] -= debris[i].speed[1] * 0.5;
			debris[i].position[2] += debris[i].speed[2] * 1.0;

			debris[i].orientation[0] += debris[i].orientationSpeed[0] * 10;
			debris[i].orientation[1] += debris[i].orientationSpeed[1] * 10;
			debris[i].orientation[2] += debris[i].orientationSpeed[2] * 10;
		}

		--fuel;
	}
}
