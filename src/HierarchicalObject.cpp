#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "HierarchicalObject.h"
using namespace std;

void HierarchicalObject::add(HierarchicalObject* obj) {
	subobjects.push_back(obj);
}
void HierarchicalObject::remove(HierarchicalObject* obj) {
	subobjects.remove(obj);
}
void HierarchicalObject::translate(float dx, float dy, float dz) {
	currentTransforms.push_back(vmath::translate(vmath::vec3(dx, dy, dz)));
}

void HierarchicalObject::scale(float sx, float sy, float sz) {
	currentTransforms.push_back(vmath::scale(sx, sy, sz));
}

void HierarchicalObject::rotate(float angle, float vx, float vy, float vz) {
	currentTransforms.push_back(vmath::rotate(angle, vmath::vec3(vx, vy, vz)));
}

void HierarchicalObject::display(vmath::mat4 projectionMatrix, vmath::mat4 viewingMatrix, vmath::mat4 modelingMatrix)
{
	vmath::mat4 newTransform, newmvpTransform, newModelingTransform;
	vmath::mat4 savedMVMatrix, savedMVPMatrix;
	GLint  savedProgram;

	glGetIntegerv(GL_CURRENT_PROGRAM, &savedProgram);
	glUseProgram(programID);
	/*
	 *  This should be fed into the object rather than going out and getting it each time
	 *  it will also help with the naming -- wouldn't need to be named MVMatrix.  Do I
	 *  need separate Model view and projection matrices?
	 */
	GLuint modelView_loc = glGetUniformLocation(programID, "MVMatrix");
	GLuint mvp_loc = glGetUniformLocation(programID, "MVPMatrix");
	glGetUniformfv(programID, modelView_loc, (GLfloat*)&savedMVMatrix);
	glGetUniformfv(programID, mvp_loc, (GLfloat*)&savedMVPMatrix);
	newTransform = savedMVMatrix * generateCompositeTransform(currentTransforms) * generateCompositeTransform(localTransforms);
	newmvpTransform = savedMVPMatrix *  generateCompositeTransform(currentTransforms) * generateCompositeTransform(localTransforms);
	glUniformMatrix4fv(modelView_loc, 1, GL_FALSE, (GLfloat*)newTransform);
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (GLfloat*)newmvpTransform);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, bao);
	glDrawArrays(GL_TRIANGLES, 0, nbrOfVertices);
	// Draw Subobjects…
	newTransform = savedMVMatrix * generateCompositeTransform(currentTransforms);
	glUniformMatrix4fv(modelView_loc, 1, GL_FALSE, (GLfloat*)newTransform);
	for (HierarchicalObject* current : subobjects) {
		current->display(projectionMatrix, viewingMatrix, modelingMatrix );
	}
	// Restore state
	glUniformMatrix4fv(modelView_loc, 1, GL_FALSE, (GLfloat*)savedMVMatrix);
	glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (GLfloat*)savedMVPMatrix);
	glUseProgram(savedProgram);
}

void HierarchicalObject::setLocalTransform(vmath::mat4 localTransform)
{
	localTransforms.clear();
	localTransforms.push_front(localTransform);
}
void HierarchicalObject::addLocalTransform(vmath::mat4 addedTransform)
{
	localTransforms.push_front(addedTransform);
}
void HierarchicalObject::clearCurrentTransform()
{
	currentTransforms.clear();
	currentTransforms.push_back(vmath::scale(1.0f));
}
vmath::mat4 HierarchicalObject::generateCompositeTransform(list<vmath::mat4> transforms)
{
	vmath::mat4 total;
	total = vmath::scale(1.0f);
	for (auto tr : transforms) {
		total = total * tr;
	}
	return total;
}


