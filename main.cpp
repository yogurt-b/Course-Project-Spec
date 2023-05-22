/*
Student Information
Student ID:202008040224
Student Name:周灵萱
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/stb_image/stb_image.h"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>


// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;
//Sc and camera
glm::vec3 SC_local_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 SC_world_pos = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 camera_world_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camera_world_view = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 SC_local_front = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 SC_local_right = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 SC_world_Front_Dir = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 SC_world_Right_Dir = glm::vec3(0.0f, 0.0f, 0.0f);
// 摄像机定义
glm::vec3 cameraPos = glm::vec3(8.2f, 1.0f, 8.2f);
//glm::vec3 cameraFront = glm::vec3(-8.2f, -3.0f, -8.2f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// timing
float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
//改变纹理
bool texcg1 = 0;
bool texcg2 = 0;
//单击开启视角移动
bool openmove = 0;
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
//移动变量
float angle = 225.0f;

glm::vec3 spacefront = glm::vec3(0.0f, 0.0f, 0.0f);
//改变光照强度
glm::vec3 lightambient(0.2f);

glm::vec3 spotPos = glm::vec3(0.0f, 4.0f, 0.0f);
// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}	
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;

GLuint VAO1, VBO1, EBO1;
Model objearth;
Texture Texture1;
Texture Texture2;

GLuint VAO2, VBO2, EBO2;
Model objmycraft;
Texture Texture3;
Texture Texture4;

void sendDataToOpenGL()
{
	//TODO
	//天空盒
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// skybox VAO

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//Load objects and bind to VAO and VBO
	//Load textures
	//skybox
	vector<std::string> faces
	{
		"resources/skybox/right.bmp",
		"resources/skybox/left.bmp",
		"resources/skybox/top.bmp",
		"resources/skybox/bottom.bmp",
		"resources/skybox/front.bmp",
		"resources/skybox/back.bmp"
	};
	cubemapTexture = loadCubemap(faces);

	//行星
	objearth = loadOBJ("resources/object/planet.obj");
	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, objearth.vertices.size() * sizeof(Vertex), &objearth.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &EBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objearth.indices.size() * sizeof(unsigned int), &objearth.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	Texture1.setupTexture("resources/texture/earthNormal.bmp");
	Texture2.setupTexture("resources/texture/earthTexture.bmp");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
	//驾驶飞行器
	objmycraft = loadOBJ("resources/object/spacecraft.obj");
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);
	//Create Vertex Buffer Objects
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, objmycraft.vertices.size() * sizeof(Vertex), &objmycraft.vertices[0], GL_STATIC_DRAW);
	//Create Element array Buffer Objects
	glGenBuffers(1, &EBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objmycraft.indices.size() * sizeof(unsigned int), &objmycraft.indices[0], GL_STATIC_DRAW);
	// 1st attribute buffer : position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);
	Texture3.setupTexture("resources/texture/spacecraftTexture.bmp");
	Texture4.setupTexture("resources/texture/gold.bmp");
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
}

Shader myshader;
Shader skyboxshader;
void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader
	myshader.setupShader("VertexShaderCode.glsl","FragmentShaderCode.glsl");
	skyboxshader.setupShader("skyVShaderCode.glsl", "skyFShaderCode.glsl");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	myshader.use();
	unsigned int slot = 0;
	glActiveTexture(GL_TEXTURE0 + slot);
	myshader.setInt("material.diffuse", 0);
	myshader.setVec3("viewPos", camera_world_pos);
	// directional light
	myshader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	myshader.setVec3("dirLight.ambient", lightambient);
	myshader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	myshader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// spotLight
	glm::vec3 lightColor;
	lightColor.x = sin(glfwGetTime() * 2.0f);
	lightColor.y = sin(glfwGetTime() * 0.7f);
	lightColor.z = sin(glfwGetTime() * 1.3f);

	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // 降低影响
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // 很低的影响

	myshader.setVec3("spotLight.position", spotPos);
	myshader.setVec3("spotLight.direction", spacefront -spotPos);
	myshader.setVec3("spotLight.ambient", ambientColor);
	myshader.setVec3("spotLight.diffuse", diffuseColor);
	myshader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	myshader.setFloat("spotLight.constant", 1.0f);
	myshader.setFloat("spotLight.linear", 0.09f);
	myshader.setFloat("spotLight.quadratic", 0.032f);
	myshader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	myshader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
	// material properties
	myshader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
	myshader.setFloat("material.shininess", 32.0f);
	// create transformations
	// 模型矩阵
	glm::mat4 model = glm::mat4(1.0f);
	//model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 观察矩阵
	glm::mat4 view = glm::mat4(1.0f);
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	// 投影矩阵
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 20.0f);

	//驾驶飞船
	model = glm::mat4(1.0f);
	glm::mat4 SC_trans = glm::translate(model, (glm::vec3(7.0f, 0.0f, 7.0f) + spacefront));
	glm::mat4 SC_scale = glm::scale(model, glm::vec3(0.0005f, 0.0005f, 0.0005f));
	glm::mat4 SC_Rot = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = SC_trans* SC_Rot* SC_scale;
	SC_world_pos = glm::vec3(7.0f, 0.0f, 7.0f) + spacefront;
	SC_world_Front_Dir = model * glm::vec4(SC_local_front, 0.0f);//应该第四位是0?
	SC_world_Right_Dir = model * glm::vec4(SC_local_right, 0.0f);
	SC_world_Front_Dir = glm::normalize(SC_world_Front_Dir);
	SC_world_Right_Dir = glm::normalize(SC_world_Right_Dir);

	//虚拟相机相对静止位置
	camera_world_pos[0] = SC_world_pos[0] - SC_world_Front_Dir[0] ;
	camera_world_pos[1] = 1.0f;
	camera_world_pos[2] = SC_world_pos[0] - SC_world_Front_Dir[2] ;
	camera_world_view[0] = SC_world_pos[0] + SC_world_Front_Dir[0] * 2;
	camera_world_view[1] = 0.0f;
	camera_world_view[2] = SC_world_pos[0] + SC_world_Front_Dir[2] * 2;
	//SC_world_pos = model * glm::vec4(0.0f, 0.0f, 10.0f, 1.0f);
	//camera_world_pos = model * glm::vec4(0.0f, 10.0f, -10.0f, 1.0f);
	//camera_world_view = SC_world_pos;
	view = glm::lookAt(camera_world_pos, camera_world_view, cameraUp);
	myshader.setMat4("view", view);
	myshader.setMat4("projection", projection);
	myshader.setMat4("model", model);
	glBindVertexArray(VAO2);
	if (texcg2 == 0)
		Texture3.bind(0);
	if (texcg2 == 1)
		Texture4.bind(0);

	glDrawElements(GL_TRIANGLES, objmycraft.indices.size(), GL_UNSIGNED_INT, 0);

	//将矩阵传入着色器
	//行星
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));//放在原点
	myshader.setMat4("model", model);

	//Bind different textures
	glBindVertexArray(VAO1);
	if (texcg1 == 0)
		Texture1.bind(0);
	if (texcg1 == 1)
		Texture2.bind(0);

	glDrawElements(GL_TRIANGLES, objearth.indices.size(), GL_UNSIGNED_INT, 0);

	//天空盒
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
	skyboxshader.use();
	skyboxshader.setInt("skybox", 0);
	// draw skybox as last
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxshader.use();
	skyboxshader.setMat4("model", model);
	skyboxshader.setMat4("view", view);
	skyboxshader.setMat4("projection", projection);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
//鼠标单击
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}
//单击后光标移动改变视角
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	float cameraSpeed = static_cast<float>(2.5 * deltaTime);
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	if (xpos < lastX)
		angle += 0.5f;
	if (xpos > lastX)
		angle -= 0.5f;
	lastX = xpos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}
//按键改变纹理
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		texcg2 = 0;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		texcg2 = 1;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		texcg1 = 0;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		texcg1 = 1;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		lightambient += glm::vec3(0.05f);
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		lightambient -= glm::vec3(0.05f);
	}
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = static_cast<float>(2.5 * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		spacefront[0] += cameraSpeed * SC_world_Front_Dir[0];
		spacefront[2] += cameraSpeed * SC_world_Front_Dir[2];
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		spacefront[0] -= cameraSpeed * SC_world_Front_Dir[0];
		spacefront[2] -= cameraSpeed * SC_world_Front_Dir[2];
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		spacefront[0] -= cameraSpeed * SC_world_Right_Dir[0];
		spacefront[2] -= cameraSpeed * SC_world_Right_Dir[2];
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		spacefront[0] += cameraSpeed * SC_world_Right_Dir[0];
		spacefront[2] += cameraSpeed * SC_world_Right_Dir[2];
	}
}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "homework 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                              //    
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		//每帧时间
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		/* Render here */
		paintGL();
		processInput(window);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






