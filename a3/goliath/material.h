/*
 * This file contains about the information about the ambient, diffuse,
 * specular and shiness values of the materials. This information is
 * from the referenced webpage.
 * 
 * Accessed : 31th Mar 2014
 * 
 * Reference : http://devernay.free.fr/cours/opengl/materials.html
 * 
 */

// Metal
const GLfloat metal[3][4] = {{0.25, 0.25, 0.25, 1.0},				// ambient
							{0.4, 0.4, 0.4, 1.0},					// diffuse
							{0.77, 0.77, 0.77, 1.0}};				// specular
const float metalShine = 0.6;

// Bronze
const GLfloat bronze[3][4] = {{0.2125, 0.1275, 0.054, 1.0},			// ambient
							{0.714, 0.4284, 0.18144, 1.0},			// diffuse
							{0.393548, 0.271906, 0.166721, 1.0}};	// specular
const float bronzeShine = 0.7;

// Ruby
const GLfloat ruby[3][4] = {{0.1745, 0.01175, 0.01175, 1.0},		// ambient
							{0.61424, 0.04136, 0.04136, 1.0},		// diffuse
							{0.727811, 0.626959, 0.626959, 1.0}};	// specular
const float rubyShine = 0.6; 

// Gold
const GLfloat gold[3][4] = {{0.24725, 0.1995, 0.0745, 1.0},			// ambient
							{0.75164, 0.60648, 0.22648, 1.0},		// diffuse
							{0.628281, 0.555802, 0.366065, 1.0}};	// specular
const float goldShine = 1.6; 
