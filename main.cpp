#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Camera.hpp"

struct App{
int mScreenWidth = 1728;
int mScreenHeight = 1117;
SDL_Window* mGraphicsApplicationWindow = nullptr;
SDL_GLContext mOpenGLContext = nullptr;
int mQuit = 0;
GLuint mGraphicsPipelineShaderProgram = 0; // store our shader object
Camera mCamera;
};

struct Transform{
	float x,y,z;
	glm::mat4 mModelMatrix{glm::mat4(1.0f)};

};

struct Mesh3D{
GLuint mVertexArrayObject = 0; // VAO
GLuint mVertexBufferObject = 0; // VBO
//Index Buffer Object
//To store the array of indices that we want to draw from when we do indexed drawing.
GLuint mIndexBufferObject = 0;
GLuint mPipeline = 0;
Transform mTransform;
// float m_uOffset = -2.0f;
// float m_uRotate = 0.0f;
// float m_uScale = 0.5f;

};
App gApp;
Mesh3D gMesh1;
Mesh3D gMesh2;
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){

    }
}
static bool GLCheckErrorStatus(const char* function, int line){
    while (GLenum error = glGetError()){
        std::cout<<"OpenGL error"<<error<<"\tLine: "<<line<<"\tfunction: "<<function<<std::endl;
        return true;
    }
    return false;
}
#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);
//^^^^^^^^^^^^^^^Error Handling Routines^^^^^^^^^^^^
std::string LoadShaderAsString(const std::string& filename){
    std::string result = "";
    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()){
        while (std::getline(myFile, line)){
            result += line + '\n';
        }
        myFile.close();
    }
    return result;
}
void PrintHWInfo() {
	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;
	std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void MeshCreate(Mesh3D* mesh) {
	// generate and bind VAO
	const std::vector<GLfloat> vertexData{ // lives on CPU
		// x    y     z
		-0.5f, -0.5f, 0.0f, // left vertex 1
        1.0f, 0.0f, 0.0f, // Red for vertex 1
		0.5f, -0.5f, 0.0f, // right vertex 2
        0.0f, 1.0f, 0.0f, // Green for vertex 2
		-0.5f, 0.5f, 0.0f, // top left vertex 3
        0.0f, 0.0f, 1.0f, // Blue for vertex 3
        
        // 0.5f, -0.5f, 0.0f, // right vertex 1
        // 0.0f, 1.0f, 0.0f, // Red for vertex 1
		0.5f, 0.5f, 0.0f, // top right vertex 2
        0.0f, 0.0f, 1.0f, // Green for vertex 2
		// -0.5f, 0.5f, 0.0f, // left vertex 3
        // 0.0f, 0.0f, 1.0f // Blue for vertex 3
	};
	glGenVertexArrays(1, &mesh->mVertexArrayObject); // start sending to GPU
	glBindVertexArray(mesh->mVertexArrayObject);

	// Setup position buffer (attribute 0)
	glGenBuffers(1, &mesh->mVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    
    const std::vector<GLuint> indexBufferData {
        2,0,1,3,2,1
    };
    glGenBuffers(1,&mesh->mIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndexBufferObject);
    //Populate our Index Buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size()*sizeof(GLuint), indexBufferData.data(),GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*6, (void*)0);
    
    // Setup color buffer (attribute 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*6, (GLvoid*)(sizeof(GL_FLOAT)*3));

    // std::cout << "Color array size: " << vertexColor.size() << std::endl;

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void MeshDelete(Mesh3D* mesh){
	glDeleteBuffers(1,&mesh->mVertexBufferObject);
	glDeleteVertexArrays(1,&mesh->mVertexArrayObject);

}
void MeshSetPipeline(Mesh3D* mesh, GLuint pipeline){
	mesh->mPipeline = pipeline;
}

GLuint CompileShader(GLuint type, const std::string& source) {
	GLuint shaderObject;
	if (type == GL_VERTEX_SHADER) {
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER) {
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	// Check for compilation errors
	int result;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length];
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);
		std::cout << "ERROR: Failed to compile " << 
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") << 
			" shader: " << errorMessages << std::endl;
		delete[] errorMessages;
		glDeleteShader(shaderObject);
		return 0;
	}

	return shaderObject;
}

GLuint CreateShaderProgram(const std::string& VertexShaderSource, const std::string& FragmentShaderSource) {
	GLuint programObject = glCreateProgram(); // create graphics pipeline
	GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, VertexShaderSource);
	GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, FragmentShaderSource);

	glAttachShader(programObject, myVertexShader);
	glAttachShader(programObject, myFragmentShader);
	glLinkProgram(programObject);
    glValidateProgram(programObject);
	return programObject;
}

void CreateGraphicsPipeline() {
    std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");
    
    std::cout << "Vertex shader loaded: " << (vertexShaderSource.empty() ? "FAILED" : "SUCCESS") << std::endl;
    std::cout << "Fragment shader loaded: " << (fragmentShaderSource.empty() ? "FAILED" : "SUCCESS") << std::endl;
    
	gApp.mGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}
void Input(){
	static int mouseX = gApp.mScreenWidth/2;
	static int mouseY = gApp.mScreenHeight/2;
SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				gApp.mQuit = 1;
			} else if (e.type == SDL_MOUSEMOTION) {
				mouseX += e.motion.xrel;
				mouseY += e.motion.yrel;
				gApp.mCamera.MouseLook(mouseX, mouseY);
			}
		}
		// g_uRotate -= 1.0f;
		
		const Uint8 *state = SDL_GetKeyboardState(NULL);

		float speed = 0.01f;
		if (state[SDL_SCANCODE_UP]) {
			gApp.mCamera.MoveForward(speed);
			// g_uOffset+=0.01f;
			// std::cout<<"g_uOffset: "<<g_uOffset<<std::endl;
		}
		if (state[SDL_SCANCODE_DOWN]){
			gApp.mCamera.MoveBackward(speed);
			// g_uOffset-=0.01f;
			// std::cout<<"g_uOffset: "<<g_uOffset<<std::endl;
		}
		if (state[SDL_SCANCODE_LEFT]){
			gApp.mCamera.MoveLeft(speed);
			// g_uRotate+=0.01f;
			// std::cout<<"g_uRotate: "<<g_uRotate<<std::endl;
		}
		if (state[SDL_SCANCODE_RIGHT]){
			gApp.mCamera.MoveRight(speed);
			// g_uRotate-=0.01f;
			// std::cout<<"g_uRotate: "<<g_uRotate<<std::endl;
		}
}
void MeshUpdate(Mesh3D* mesh) {
	// glDisable(GL_DEPTH_TEST);
	// glDisable(GL_CULL_FACE);
	// glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
	// glClearColor(1.f, 1.f, 0.f, 1.f);

	// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


}

int FindUniformLocation(GLuint pipeline, const GLchar* name){
	GLint location = glGetUniformLocation(pipeline, name);
	if (location >= 0){
		return location;
	} else {
		std::cerr<<"could not find "<<name<<std::endl;
		exit(EXIT_FAILURE);
	}
}

void MeshDraw(Mesh3D* mesh) {
	if (mesh == nullptr){
		return;
	}
	glUseProgram(mesh->mPipeline);
	
	GLuint u_ModelMatrixLocation = FindUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ModelMatrix");
	glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&mesh->mTransform.mModelMatrix[0][0]);

	//View Matrix
	glm::mat4 view = gApp.mCamera.GetViewMatrix();
	GLint u_ViewLocation = FindUniformLocation(gApp.mGraphicsPipelineShaderProgram,"u_ViewMatrix");
	glUniformMatrix4fv(u_ViewLocation,1,GL_FALSE,&view[0][0]);

	//Projection matrix
	glm::mat4 perspective = gApp.mCamera.GetProjectionMatrix();
	// glm::mat4 perspective = glm::perspective(glm::radians(45.0f),(float)gApp.mScreenWidth/(float)gApp.mScreenHeight,
											// 0.1f,
											// 10.0f);
	GLint u_ProjectionLocation = FindUniformLocation(gApp.mGraphicsPipelineShaderProgram,"u_Projection");
	glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);

	GLCheck(glBindVertexArray(mesh->mVertexArrayObject);)
	// GLCheck(glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);)
	// glDrawArrays(GL_TRIANGLES, 0, 6);
    GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);)
	glUseProgram(0);
}

void MeshTranslate(Mesh3D* mesh, float x, float y, float z){
	//Model Transform
	mesh->mTransform.mModelMatrix = glm::translate(mesh->mTransform.mModelMatrix,glm::vec3(x,y,z));
}
void MeshRotate(Mesh3D* mesh, float angle, glm::vec3 axis)
{
	mesh->mTransform.mModelMatrix = glm::rotate(mesh->mTransform.mModelMatrix,glm::radians(angle),axis);
}
void MeshScale(Mesh3D* mesh, float x, float y, float z)
{
	mesh->mTransform.mModelMatrix = glm::scale(mesh->mTransform.mModelMatrix,glm::vec3(x,y,z));
}
// 	mesh->m_uRotate -= 0.1f;
// 	//Update Model Matrix 
// 	model = glm::rotate(model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f,1.0f,0.0f));
// 	model = glm::scale(model, glm::vec3(mesh->m_uScale, mesh->m_uScale, mesh->m_uScale));
// }
int main(int argc, char* args[])
{
	SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//Setup the camera
	gApp.mCamera.SetProjectionMatrix(glm::radians(45.0f), (float)gApp.mScreenWidth/(float)gApp.mScreenHeight, 0.1f, 10.0f);

	gApp.mGraphicsApplicationWindow = SDL_CreateWindow("hello", 10, 50, gApp.mScreenWidth, gApp.mScreenHeight, SDL_WINDOW_OPENGL);
	if (SDL_GL_CreateContext(gApp.mGraphicsApplicationWindow) == NULL) {
		std::cout << "OpenGL context failed: " << SDL_GetError() << std::endl;
	}
	else {
		if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
			std::cout << "glad was not initialized" << std::endl;
			exit(1);
		}
		else {
			PrintHWInfo();
			MeshCreate(&gMesh1);
			MeshTranslate(&gMesh1,0.0f, 0.0f, -2.0f);
			MeshCreate(&gMesh2);
			MeshTranslate(&gMesh2,0.0f, 0.0f, -4.0f);

			CreateGraphicsPipeline();
			MeshSetPipeline(&gMesh1, gApp.mGraphicsPipelineShaderProgram);
			MeshSetPipeline(&gMesh2, gApp.mGraphicsPipelineShaderProgram);
		}
	}
	//Store the current mouse position
	// int mouseX, mouseY;
	SDL_WarpMouseInWindow(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth/2, gApp.mScreenHeight/2);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	while (gApp.mQuit == 0) {
		Input();	
		//Update our mesh
			glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
	glClearColor(1.f, 1.f, 0.f, 1.f);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// MeshUpdate(&gMesh1);
		// MeshUpdate(&gMesh2);
		static float rotate = 0.0f;
		rotate+= 0.05f;
		MeshRotate(&gMesh1,rotate,glm::vec3(0.0f,1.0f,0.0f));
		MeshDraw(&gMesh1);
		MeshDraw(&gMesh2);
		SDL_GL_SwapWindow(gApp.mGraphicsApplicationWindow);
	}

	SDL_DestroyWindow(gApp.mGraphicsApplicationWindow);
	gApp.mGraphicsApplicationWindow = nullptr;
	MeshDelete(&gMesh1);	
	glDeleteProgram(gApp.mGraphicsPipelineShaderProgram);
	SDL_Quit();
	return 0;
}