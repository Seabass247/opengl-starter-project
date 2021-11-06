/*
* For EECS 4530 -- Programming Project 2
*
* main.cpp - The entry point to this program
*
* Created by: Gerald Heuring
* Modified by: Sebastian Hamel
*
*/
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include "LoadShaders.h"
#include "objReader.h"
#include "linmath.h"
#include <map>
#include <vector>
using namespace std;

#define BUFFER_OFFSET(x)  ((const void*) (x))

#define TOTAL_NUM_OBJECTS 3

#define ANIMATION_RATE  0.005f
#define ANIMATION_STEP  0.05f

#define CAMERA_DISTANCE 15.0f

GLuint programID;
/*
* Arrays to store the indices/names of the Vertex Array Objects and
* Buffers.  Rather than using the books enum approach I've just
* gone out and made a bunch of them and will use them as needed.
*
* Not the best choice I'm sure.
*/

GLuint vertexBuffers[10], arrayBuffers[10], elementBuffers[10];
/*
* Global variables
*   The location for the transformation and the current rotation
*   angle are set up as globals since multiple methods need to
*   access them.
*/
float rotationAngle;
bool elements;
int nbrTriangles[10], materialToUse = 0;
int startTriangle = 0, endTriangle = 12;
bool continuousAnimation = true;
bool orthoViewEnabled = false;
mat4x4 rotation, viewMatrix, projectionMatrix;
map<string, GLuint> locationMap;
float currentT = 0.0f;

// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow * glfwStartUp(int& argCount, char* argValues[],
	string windowTitle = "No Title", int width = 500, int height = 500);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
	GLenum fill = GL_FILL);
void buildObjects(int object_indx);
void getLocations();
void SetUpDirectionalLighting();
void buildModelMatrix(mat4x4& translation1, float scaleFactor, float deltax, float deltay, float deltaz);
void init(string vertexShader, string fragmentShader);

/*
 * Error callback routine for glfw -- uses cstdio 
 */
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

/*
 * keypress callback for glfw -- Escape exits...
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float eye_x[] = { CAMERA_DISTANCE, 0.0f, 0.0f };
	float eye_y[] = { 0.0f, CAMERA_DISTANCE, 0.0f };
	float eye_z[] = { 0.0f, 0.0f, CAMERA_DISTANCE };
	float center[] = { 0.0f, 0.0f, 0.0f };
	float up[] = { 0.0f, 1.0f, 0.0f };

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		// Look down the X axis
		mat4x4_look_at(viewMatrix, eye_x, center, up);
	}
	else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		// Look down the Y axis
		mat4x4_look_at(viewMatrix, eye_y, center, eye_x);
	}
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		// Look down the Z axis
		mat4x4_look_at(viewMatrix, eye_z, center, up);
	}
	else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		// Toggle view between orthographic and perspective
		orthoViewEnabled = !orthoViewEnabled;

		if(orthoViewEnabled) {
			mat4x4_ortho(projectionMatrix, -1*CAMERA_DISTANCE, CAMERA_DISTANCE, -1*CAMERA_DISTANCE, CAMERA_DISTANCE, -100.0f, 100.0f);
		} else {
			mat4x4_perspective(projectionMatrix, 45.0f/180.0f * 3.14159f, 1.0f, 0.01f, 1000.0f);
		}
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		// Go into "step by step" animation mode
		continuousAnimation = false;
		// Advance the step
		currentT += ANIMATION_STEP;
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		// Return to "continuous animation mode"
		continuousAnimation = true;
	}
}

/*
 * Routine to encapsulate some of the startup routines for GLFW.  It returns the window ID of the
 * single window that is created.
 */
GLFWwindow* glfwStartUp(int& argCount, char* argValues[], string title, int width, int height) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	return window;
}


/*
 * Use the author's routines to build the program and return the program ID.
 */
GLuint buildProgram(string vertexShaderName, string fragmentShaderName) {

	/*
	*  Use the Books code to load in the shaders.
	*/
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, vertexShaderName.c_str() },
		{ GL_FRAGMENT_SHADER, fragmentShaderName.c_str() },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	if (program == 0) {
		cerr << "GLSL Program didn't load.  Error \n" << endl
			<< "Vertex Shader = " << vertexShaderName << endl
			<< "Fragment Shader = " << fragmentShaderName << endl;
	}
	glUseProgram(program);
	return program;
}

/*
 * Set up the clear color, lineWidth, and the fill type for the display.
 */
void setAttributes(float lineWidth, GLenum face, GLenum fill) {
	/*
	* I'm using wide lines so that they are easier to see on the screen.
	* In addition, this version fills in the polygons rather than leaving it
	* as lines.
	*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glLineWidth(lineWidth);
	glPolygonMode(face, fill);
	glEnable(GL_DEPTH_TEST);

}

/*
 * read and/or build the objects to be displayed.  Also sets up attributes that are
 * vertex related.
 */
void buildObjects(const char *obj_path, int obj_indx) {
	glBindVertexArray(vertexBuffers[obj_indx]);

	/*
	* Read object in from obj file.
	*/
	GLfloat *objVertices= nullptr, *objNormals=nullptr;

	objVertices = readOBJFile(obj_path, nbrTriangles[obj_indx], objNormals);

	glGenBuffers(1, &(arrayBuffers[obj_indx]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[obj_indx]);
	GLuint objVerticesSize = nbrTriangles[obj_indx] * 3.0 * 4.0 * sizeof(GLfloat);
	GLuint objNormalsSize = nbrTriangles[obj_indx] * 3.0 * 3.0 * sizeof(GLfloat);

	int numVerts = nbrTriangles[obj_indx] * 3.0 * 4.0;
	GLfloat objColors[numVerts] = {0};
	for (int i = 0; i < numVerts; i++) {
		objColors[i] = 0.7f;
	}
	GLuint objColorsSize = objVerticesSize;
	glBufferData(GL_ARRAY_BUFFER,
		objVerticesSize + objColorsSize + objNormalsSize,
		NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, objVerticesSize, objVertices);
	glBufferSubData(GL_ARRAY_BUFFER, objVerticesSize, objColorsSize, objColors);
	glBufferSubData(GL_ARRAY_BUFFER, objVerticesSize+objColorsSize, objNormalsSize, objNormals);

	GLuint vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	GLuint vColors = glGetAttribLocation(programID, "vColor");
	glEnableVertexAttribArray(vColors);
	glVertexAttribPointer(vColors, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(objVerticesSize));
	GLuint vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(objVerticesSize+objColorsSize));
}

/*
 * This fills in the locations of most of the uniform variables for the program.
 * there are better ways of handling this but this is good in going directly from
 * what we had.
 *
 * Revised to get the locations and names of the uniforms from OpenGL.  These
 * are then stored in a map so that we can look up a uniform by name when we
 * need to use it.  The map is still global but it is a little neater than the
 * version that used all the locations.  The locations are still there right now
 * in case that is more useful for you.
 *
 */

void getLocations() {
	/*
	 * Find out how many uniforms there are and go out there and get them from the
	 * shader program.  The locations for each uniform are stored in a global -- locationMap --
	 * for later retrieval.
	 */
	GLint numberBlocks;
	char uniformName[1024];
	int nameLength;
	GLint size;
	GLenum type;
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numberBlocks);
	for (int blockIndex = 0; blockIndex < numberBlocks; blockIndex++) {
		glGetActiveUniform(programID, blockIndex, 1024, &nameLength, &size, &type, uniformName);
		cout << uniformName << endl;
		locationMap[string(uniformName)] = blockIndex;
	}
}

void init(string vertexShader, string fragmentShader) {

	setAttributes(1.0f, GL_FRONT_AND_BACK, GL_FILL);

	programID = buildProgram(vertexShader, fragmentShader);

	mat4x4_identity(rotation);
	mat4x4_identity(viewMatrix);

	float eye_x[] = { CAMERA_DISTANCE, 0.0f, 0.0f };
	float eye_y[] = { 0.0f, CAMERA_DISTANCE, 0.0f };
	float eye_z[] = { 0.0f, 0.0f, CAMERA_DISTANCE };
	float center[] = { 0.0f, 0.0f, 0.0f };
	float up[] = { 0.0f, 1.0f, 0.0f };
	float right[] = { 1.0f, 0.0f, 0.0f };

	// mat4x4_look_at(viewMatrix, eye_x, center, up);
	// mat4x4_look_at(viewMatrix, eye_y, center, right);
	mat4x4_look_at(viewMatrix, eye_z, center, up);

	if(orthoViewEnabled) {
		mat4x4_ortho(projectionMatrix, -1*CAMERA_DISTANCE, CAMERA_DISTANCE, -1*CAMERA_DISTANCE, CAMERA_DISTANCE, -100.0f, 100.0f);
	} else {
		mat4x4_perspective(projectionMatrix, 45.0f/180.0f * 3.14159f, 1.0f, 0.01f, 1000.0f);
	}

	glGenVertexArrays(TOTAL_NUM_OBJECTS, vertexBuffers);
	buildObjects("../res/models/triangulatedCowDos.obj", 0);
	buildObjects("../res/models/cylinderProject2.obj", 1);
	buildObjects("../res/models/coneProject2.obj", 2);

	getLocations();
}

/*
 * The display routine is based on the original given but with directional lighting and suppot for multiple objects.
 */
void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed
	glUseProgram(programID);
	mat4x4 translation[10];
	float scaleFactor, deltax, deltay, deltaz;

	float xt = 5.0f * sinf(currentT + 3.14159f / 2.0f);
	float yt = 0.0f;
	float zt = 5.0f * sinf(currentT * 2.0f);

	buildModelMatrix(translation[0], 0.25f, xt, yt, zt);
	buildModelMatrix(translation[1], 0.5f, 2.0f, 0.0f, 0.0f);
	buildModelMatrix(translation[2], 0.5f, -2.0f, 0.0f, 0.0f);

	GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");

	for(int obj_i = 0; obj_i < TOTAL_NUM_OBJECTS; obj_i++) {
		glBindVertexArray(vertexBuffers[obj_i]);
		glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[obj_i]);
		glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat *)translation[obj_i]);
		glDrawArrays(GL_TRIANGLES, 0, nbrTriangles[obj_i] * 3);
	}

	GLuint viewMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, false, (const GLfloat*)viewMatrix);
	GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*)projectionMatrix);
	SetUpDirectionalLighting();

	if(continuousAnimation) {
		currentT += ANIMATION_RATE;
	}
	if (currentT > 2.0f * 3.14159f) {
		currentT -= 2.0f * 3.14159f;
	}
}

void buildModelMatrix(mat4x4& translation1, float scaleFactor, float deltax, float deltay, float deltaz)
{
	mat4x4 temp;
	mat4x4 scale;
	mat4x4_identity(translation1);
	mat4x4_identity(scale);
	mat4x4_scale_aniso(scale, scale, scaleFactor, scaleFactor, scaleFactor);
	mat4x4_translate(temp, deltax, deltay, deltaz);
	mat4x4_mul(translation1, temp, scale);
}

/*
 * Set up the parameters for directional lighting.
 */
void SetUpDirectionalLighting()
{	
	GLfloat ambientLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLuint ambientLightLocation = glGetUniformLocation(programID, "ambientLight");
	glUniform4fv(ambientLightLocation, 1, ambientLight);
	GLuint directionalLightDirectionLoc = glGetUniformLocation(programID, "directionalLightDirection");
	GLuint directionalLightColorLoc = glGetUniformLocation(programID, "directionalLightColor");
	GLuint halfVectorLocation = glGetUniformLocation(programID, "halfVector");
	GLuint shininessLoc = glGetUniformLocation(programID, "shininess");
	GLuint strengthLoc = glGetUniformLocation(programID, "strength");
	GLfloat directionalLightDirection[] = { 0.0f, 0.7071f, 0.7071f };
	GLfloat directionalLightColor[] = { 1.0f, 1.0f, 1.0f };
	GLfloat shininess = 25.0f;
	GLfloat strength = 1.0f;
	GLfloat halfVector[] = { 0.0f, 0.45514f, 0.924f };
	glUniform1f(shininessLoc, shininess);
	glUniform1f(strengthLoc, strength);
	glUniform3fv(directionalLightDirectionLoc, 1, directionalLightDirection);
	glUniform3fv(directionalLightColorLoc, 1, directionalLightColor);
	glUniform3fv(halfVectorLocation, 1, halfVector);
}

/*
* Handle window resizes -- adjust size of the viewport -- more on this later
*/

void reshapeWindow(GLFWwindow* window, int width, int height)
{
	float ratio;
	ratio = width / (float)height;

	glViewport(0, 0, width, height);

}
/*
* Main program with calls for many of the helper routines.
*/
int main(int argCount, char* argValues[]) {
	GLFWwindow* window = nullptr;
	window = glfwStartUp(argCount, argValues, "EECS 4530 Programming Project 2");
	init("../res/shaders/passthrough.vert", "../res/shaders/passthrough.frag");
	glfwSetWindowSizeCallback(window, reshapeWindow);

	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
