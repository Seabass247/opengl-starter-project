/*
* For EECS 4530 -- Programming Project 1
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
#include "linmath.h"
//#include "vgl.h"
#include <map>
#include <vector>
using namespace std;

#define BUFFER_OFFSET(x)  ((const void*) (x))

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
int nbrTriangles, materialToUse = 0;
int startTriangle = 0, endTriangle = 12;
bool rotationOn = true;
bool orthoViewEnabled = true;
mat4x4 rotation, viewMatrix, projectionMatrix;

map<string, GLuint> locationMap;

// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow * glfwStartUp(int& argCount, char* argValues[],
	string windowTitle = "No Title", int width = 500, int height = 500);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
	GLenum fill = GL_FILL);
void buildObjects();
void getLocations();
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
	float eye_x[] = { 5.0f, 0.0f, 0.0f };
	float eye_y[] = { 0.0f, 5.0f, 0.0f };
	float eye_z[] = { 0.0f, 0.0f, 5.0f };
	float center[] = { 0.0f, 0.0f, 0.0f };
	float up[] = { 0.0f, 1.0f, 0.0f };

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		mat4x4_rotate_Y(rotation, rotation, 0.31419);
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		mat4x4_rotate_Y(rotation, rotation, -0.31419);
    } 
	else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		mat4x4_look_at(viewMatrix, eye_x, center, up);
	}
	else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		mat4x4_look_at(viewMatrix, eye_y, center, eye_x);
	}
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		mat4x4_look_at(viewMatrix, eye_z, center, up);
	}
	else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		orthoViewEnabled = !orthoViewEnabled;

		// Toggle view between orthographic and perspective
		if(orthoViewEnabled) {
			mat4x4_ortho(projectionMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
		} else {
			mat4x4_perspective(projectionMatrix, 45.0f/180.0f * 3.14159f, 1.0f, 0.01f, 1000.0f);
		}
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		// Toggle rotation off
		rotationOn = false;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		// Toggle rotation off
		rotationOn = true;
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

typedef struct
{
    uint16_t NumVerts;
    float Vertices[256];
    float Colors[256];
} DatModel_t;

/*
 * Read a .dat file into a model object 
 */
void readDataFile(const char* fileName, DatModel_t* model) {
    FILE* file = fopen(fileName, "r");
	if(file!=NULL)
	{
		char line[256];

		if(fgets(line, sizeof(line), file) != NULL) {
				uint16_t val = (unsigned short) strtoul(line, NULL, 0);
				model->NumVerts = val;
				// printf("Num Verts = '%hu'\n", val);
		}

		uint16_t vertIndx = 0;

		// Read in vertex positions
		while(fgets(line, sizeof(line), file)) {

			char* ptr = strtok(line, " ");

			while(ptr != NULL)
			{
				float val = atof(ptr);
				// printf("v_pos = '%f'\n", val);
				// printf("WRITE vert[%hu] = %f \n", vertIndx, val);
				model->Vertices[vertIndx++] = val;

				ptr = strtok(NULL, " ");
			}

			if(vertIndx == (model->NumVerts * 4)) {
				// printf("BREAK %hu \n", vertIndx);
				break;
			}
		}

		// Read in vertex colors
		vertIndx = 0;
		while(fgets(line, sizeof(line), file)) {

			char* ptr = strtok(line, " ");

			while(ptr != NULL)
			{
				float val = atof(ptr);
				// printf("WRITE color[%hu] = %f \n", vertIndx, val);

				model->Colors[vertIndx++] = val;
				ptr = strtok(NULL, " ");
			}
		}

		fclose(file);
	} else {
		printf("\nError: Cannot open data file!\n");
	    fclose(file);
	}
	
}

/*
 * read and/or build the objects to be displayed.  Also sets up attributes that are
 * vertex related.
 */
void buildObjects() {

	// Retrieve the object model from file
	DatModel_t model = {0};
	char const* const fileName = "../res/models/diamond.dat";
	readDataFile(fileName, &model);

	size_t bufSize = model.NumVerts * 4;
	nbrTriangles = model.NumVerts / 3;
	GLfloat vertices[bufSize] = {0.0f};
	memcpy(vertices, model.Vertices, sizeof(vertices));

	GLfloat colors[bufSize] = {0.0f};
	memcpy(colors, model.Colors, sizeof(colors));

	// for(int v = 0; v < bufSize; v++) {
	// 	printf("v[%hu] %f\n", v, vertices[v]);
	// }

	// for(int c = 0; c < bufSize; c++) {
	// 	printf("c[%hu] %f\n", c, colors[c]);
	// }
	// GLfloat vertices[] = {
	// printf("nbrTriangles = %hu\n", nbrTriangles);

	glGenVertexArrays(1, vertexBuffers);
	glBindVertexArray(vertexBuffers[0]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//

/*
 * Test code for internal object.
 */
	nbrTriangles = 6;
	glGenBuffers(1, &(arrayBuffers[0]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	GLuint vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(programID, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));
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

	float eye_x[] = { 5.0f, 0.0f, 0.0f };
	float center[] = { 0.0f, 0.0f, 0.0f };
	float up[] = { 0.0f, 1.0f, 0.0f };
	mat4x4_look_at(viewMatrix, eye_x, center, up);

	mat4x4_ortho(projectionMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
	// mat4x4_identity(projectionMatrix);

	buildObjects();

	getLocations();

}

/*
 * The display routine is basically unchanged at this point.
 */
void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed

	if(rotationOn) {
		float speed = 0.01f;
		mat4x4_rotate_Y(rotation, rotation, speed);
	}

	GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat *)rotation);

	GLuint viewMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, false, (const GLfloat *) viewMatrix);
	GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat *)projectionMatrix);

	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles * 3);

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
	window = glfwStartUp(argCount, argValues, "EECS 4530 Programming Project 1");
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
