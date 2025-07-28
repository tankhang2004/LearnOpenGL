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
int gScreenWidth = 640;
int gScreenHeight = 480;

int quit = 0;
float g_uOffset = 0.0f;
GLuint gVertexArrayObject = 0; // VAO
GLuint gVertexBufferObject = 0; // VBO
GLuint gGraphicsPipelineShaderProgram = 0; // store our shader object
//Index Buffer Object
//To store the array of indices that we want to draw from when we do indexed drawing.
GLuint gIndexBufferObject = 0;

// float g_uOffset = 0.0f;
// GLuint gVertexBufferObject2 = 0;     
// example shaders
// const std::string gVertexShaderSource =
// "#version 410 core\n"
// "in vec4 position;\n"
// "void main()\n"
// "{\n"
// "    gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
// "}\n";

// const std::string gFragmentShaderSource =
// "#version 410 core\n"
// "out vec4 color;\n"
// "void main()\n"
// "{\n"
// "    color = vec4(1.0f, 0.5f, 0.0, 1.0f);\n"
// "}\n";
//vvvvvvvvvvvvvvvError Handling Routinesvvvvvvvvvvvv
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

void VertexSpecification() {
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
	glGenVertexArrays(1, &gVertexArrayObject); // start sending to GPU
	glBindVertexArray(gVertexArrayObject);

	// Setup position buffer (attribute 0)
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    
    const std::vector<GLuint> indexBufferData {
        2,0,1,3,2,1
    };
    glGenBuffers(1,&gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
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
    
	gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void PreDraw() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(1.f, 1.f, 0.f, 1.f);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(gGraphicsPipelineShaderProgram);
	
	//Model Transform
	glm::mat4 translate = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,g_uOffset));
	GLint u_ModelMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");
	if (u_ModelMatrixLocation>=0){
		glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&translate[0][0]);
	} else
	{
		std::cout<<"Could not find u_ModelMatrix";
		exit(EXIT_FAILURE);
	}
	//Projection matrix
	glm::mat4 perspective = glm::perspective(glm::radians(45.0f),(float)gScreenWidth/(float)gScreenHeight,
											0.1f,
											10.0f);
	GLint u_ProjectionLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Projection");
	if (u_ProjectionLocation>=0){
		glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
	} else
	{
		std::cout<<"Could not find u_PerspectiveLocation";
		exit(EXIT_FAILURE);
	}
}

void Draw() {
	GLCheck(glBindVertexArray(gVertexArrayObject);)
	GLCheck(glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);)
	// glDrawArrays(GL_TRIANGLES, 0, 6);
    GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);)
}

int main(int argc, char* args[])
{
	SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window* win = SDL_CreateWindow("hello", 10, 50, 640, 480, SDL_WINDOW_OPENGL);
	if (SDL_GL_CreateContext(win) == NULL) {
		std::cout << "OpenGL context failed: " << SDL_GetError() << std::endl;
	}
	else {
		if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
			std::cout << "glad was not initialized" << std::endl;
			exit(1);
		}
		else {
			PrintHWInfo();
			VertexSpecification();
			CreateGraphicsPipeline();
		}
	}

	while (quit == 0) {
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = 1;
		}
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_UP]) {
			g_uOffset+=0.01f;
			std::cout<<"g_uOffset: "<<g_uOffset<<std::endl;
		}
		if (state[SDL_SCANCODE_DOWN]){
			g_uOffset-=0.01f;
			std::cout<<"g_uOffset: "<<g_uOffset<<std::endl;
		}
		PreDraw();
		Draw();
		SDL_GL_SwapWindow(win);
	}

	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}