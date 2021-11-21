/*
Semestre 2022-1
Práctica : Iluminación
Cambios en el shader, en lugar de enviar la textura en el shader de fragmentos, enviaremos el finalcolor
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"


//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"

const float PI = 3.14159265f;
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture dadoTexture;
Texture pisoTexture;
Texture Tagave;
Texture toroidTexture;


Model Kitt_M;
Model Llanta_M;
Model Camino_M;
Model Blackhawk_M;
Model Blackhawk_M_Body;
Model Blackhawk_M_UBlade;
Model Blackhawk_M_BBlade;
Model Dado_M;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;

//luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;


// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//cálculo del promedio de las normales para sombreado de Phong
void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}




void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
		0, 1, 2,
		0, 2, 3,
		4,5,6,
		4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};
	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

	Mesh *obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

}

void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		4, 5, 6,
		6, 7, 4,
		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,
	};


	GLfloat cubo_vertices[] = {
		// front
		//x		y		z		S		T			NX		NY		NZ
		-0.5f, -0.5f,  0.5f,	0.27f,  0.35f,		0.0f,	0.0f,	-1.0f,	//0
		0.5f, -0.5f,  0.5f,		0.48f,	0.35f,		0.0f,	0.0f,	-1.0f,	//1
		0.5f,  0.5f,  0.5f,		0.48f,	0.64f,		0.0f,	0.0f,	-1.0f,	//2
		-0.5f,  0.5f,  0.5f,	0.27f,	0.64f,		0.0f,	0.0f,	-1.0f,	//3
		// right
		//x		y		z		S		T
		0.5f, -0.5f,  0.5f,	    0.52f,  0.35f,		-1.0f,	0.0f,	0.0f,
		0.5f, -0.5f,  -0.5f,	0.73f,	0.35f,		-1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  -0.5f,	0.73f,	0.64f,		-1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  0.5f,	    0.52f,	0.64f,		-1.0f,	0.0f,	0.0f,
		// back
		-0.5f, -0.5f, -0.5f,	0.77f,	0.35f,		0.0f,	0.0f,	1.0f,
		0.5f, -0.5f, -0.5f,		0.98f,	0.35f,		0.0f,	0.0f,	1.0f,
		0.5f,  0.5f, -0.5f,		0.98f,	0.64f,		0.0f,	0.0f,	1.0f,
		-0.5f,  0.5f, -0.5f,	0.77f,	0.64f,		0.0f,	0.0f,	1.0f,

		// left
		//x		y		z		S		T
		-0.5f, -0.5f,  -0.5f,	0.0f,	0.35f,		1.0f,	0.0f,	0.0f,
		-0.5f, -0.5f,  0.5f,	0.23f,  0.35f,		1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  0.5f,	0.23f,	0.64f,		1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  -0.5f,	0.0f,	0.64f,		1.0f,	0.0f,	0.0f,

		// bottom
		//x		y		z		S		T
		-0.5f, -0.5f,  0.5f,	0.27f,	0.02f,		0.0f,	1.0f,	0.0f,
		0.5f,  -0.5f,  0.5f,	0.48f,  0.02f,		0.0f,	1.0f,	0.0f,
		 0.5f,  -0.5f,  -0.5f,	0.48f,	0.31f,		0.0f,	1.0f,	0.0f,
		-0.5f, -0.5f,  -0.5f,	0.27f,	0.31f,		0.0f,	1.0f,	0.0f,

		//UP
		 //x		y		z		S		T
		 -0.5f, 0.5f,  0.5f,	0.27f,	0.68f,		0.0f,	-1.0f,	0.0f,
		 0.5f,  0.5f,  0.5f,	0.48f,  0.68f,		0.0f,	-1.0f,	0.0f,
		  0.5f, 0.5f,  -0.5f,	0.48f,	0.98f,		0.0f,	-1.0f,	0.0f,
		 -0.5f, 0.5f,  -0.5f,	0.27f,	0.98f,		0.0f,	-1.0f,	0.0f,

	};

	Mesh *cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
	meshList.push_back(cubo);

}

void CrearToroide( int mainSegments, int tubeSegments, float mainRadius, float tubeRadius ) {

	int numVertices = (mainSegments + 1) * (tubeSegments + 1);
	int primitiveRestartIndex = numVertices; // Solo si no obtengo la forma para guardar todo los indices
	int numIndices = (mainSegments * 2 * (tubeSegments + 1)) + mainSegments - 1;
	//size_t coordenada = 0;

	//GLfloat* torus_vertices = (GLfloat*)calloc(sizeof(GLfloat*), (numVertices));
	std::vector<GLfloat> torus_vertices_vector;

	// coordenadas
	GLfloat mainSegmentAngleStep = glm::radians(360.0f / float(mainSegments));
	GLfloat tubeSegmentAngleStep = glm::radians(360.0f / float(tubeSegments));

	// texturas
	GLfloat mainSegmentTextureStep = 2.0f / float(mainSegments);
	GLfloat tubeSegmentTextureStep = 1.0f / float(tubeSegments);
	GLfloat currentMainSegmentTexCoordV = 0.0f;

	GLfloat currentMainSegmentAngle = 0.0f;
	for (size_t i = 0; i <= mainSegments; i++)
	{
		// Calculate sine and cosine of main segment angle
		GLfloat sinMainSegment = sin(currentMainSegmentAngle);
		GLfloat cosMainSegment = cos(currentMainSegmentAngle);
		GLfloat currentTubeSegmentAngle = 0.0f;

		// texture coordinate and 
		GLfloat currentTubeSegmentTexCoordU = 0.0f;

		// normals
		//GLfloat currentMainSegmentAngle = 0.0f;

		for (size_t j = 0; j <= tubeSegments; j++)
		{
			// Calculate sine and cosine of tube segment angle
			GLfloat sinTubeSegment = sin(currentTubeSegmentAngle);
			GLfloat cosTubeSegment = cos(currentTubeSegmentAngle);

			// Calculate vertex position on the surface of torus
				// coordenada x
			torus_vertices_vector.push_back((mainRadius + tubeRadius * cosTubeSegment) * cosMainSegment);
			/*torus_vertices[coordenada] = (mainRadius + tubeRadius * cosTubeSegment) * cosMainSegment;
			coordenada++;*/
				// coordenada y
			torus_vertices_vector.push_back((mainRadius + tubeRadius * cosTubeSegment) * sinMainSegment);
			/*torus_vertices[coordenada] = (mainRadius + tubeRadius * cosTubeSegment) * sinMainSegment;
			coordenada++;*/
			//	// coordenada z
			torus_vertices_vector.push_back(tubeRadius * sinTubeSegment);
			//torus_vertices[coordenada] = tubeRadius * sinTubeSegment;
			//coordenada++;

			/*auto surfacePosition = glm::vec3(
				(mainRadius + tubeRadius * cosTubeSegment) * cosMainSegment,
				(mainRadius + tubeRadius * cosTubeSegment) * sinMainSegment,
				tubeRadius * sinTubeSegment);*/

			// texturas
			torus_vertices_vector.push_back(currentTubeSegmentTexCoordU);
			/*torus_vertices[coordenada] = currentTubeSegmentTexCoordU;
			coordenada++;*/
			torus_vertices_vector.push_back(currentMainSegmentTexCoordV);
			/*torus_vertices[coordenada] = currentMainSegmentTexCoordV;
			coordenada++;*/
			//auto textureCoordinate = glm::vec2(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV);
			//_vbo.addData(&textureCoordinate, sizeof(glm::vec2));

			currentTubeSegmentTexCoordU += tubeSegmentTextureStep;

			// normals
			torus_vertices_vector.push_back(cosMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = cosMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(sinMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = sinMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(sinTubeSegment);
			/*torus_vertices[coordenada] = sinTubeSegment;
			coordenada++;*/
			/*auto normal = glm::vec3(
				cosMainSegment * cosTubeSegment,
				sinMainSegment * cosTubeSegment,
				sinTubeSegment
			);*/

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
		// texturas
		currentMainSegmentTexCoordV += mainSegmentTextureStep;
	}

	// generacion de indices

	std::vector<unsigned int> torus_indices_vector;
	//unsigned int* torus_indices = (unsigned int*)calloc(sizeof(unsigned int*), numIndices);

	//coordenada = 0;

	GLuint currentVertexOffset = 0;
	for (size_t i = 0; i < mainSegments; i++)
	{
		for (size_t j = 0; j <= tubeSegments; j++)
		{
			GLuint vertexIndexA = currentVertexOffset;
			//_indicesVBO.addData(&vertexIndexA, sizeof(GLuint));
			torus_indices_vector.push_back(vertexIndexA);
			/*torus_indices[coordenada] = vertexIndexA;
			coordenada++;*/
			GLuint vertexIndexB = currentVertexOffset + tubeSegments + 1;
			/*torus_indices[coordenada] = vertexIndexB;
			coordenada++;*/
			torus_indices_vector.push_back(vertexIndexB);
			//_indicesVBO.addData(&vertexIndexB, sizeof(GLuint));
			currentVertexOffset++;
		}

		// Don't restart primitive, if it's last segment, rendering ends here anyway
		if (i != mainSegments - 1) {
			/*torus_indices[coordenada] = primitiveRestartIndex;
			coordenada++;*/
			torus_indices_vector.push_back(primitiveRestartIndex);
			//_indicesVBO.addData(&_primitiveRestartIndex, sizeof(GLuint));
		}
	}


	Mesh* torus = new Mesh();
	torus->CreateMesh(&torus_vertices_vector[0], &torus_indices_vector[0], torus_vertices_vector.size(), torus_indices_vector.size(), primitiveRestartIndex);
	meshList.push_back(torus);
}

void CrearCilindro(int res, float height, float R) {

	//constantes utilizadas en los ciclos for
	int n, i, coordenada = 0;
	//número de vértices ocupados
	int verticesBase = (res + 1) * 6;
	//cálculo del paso interno en la circunferencia y variables que almacenarán cada coordenada de cada vértice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
	//apuntadores para guardar todos los vértices e índices generados
	//GLfloat* vertices = (GLfloat*)calloc(sizeof(GLfloat*), (verticesBase) * 3);
	std::vector<GLfloat> vertices;
	//unsigned int* indices = (unsigned int*)calloc(sizeof(unsigned int*), verticesBase);
	std::vector<unsigned int> indices;

	//ciclo for para crear los vértices de las paredes del cilindro
	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		//caso para terminar el círculo
		else {
			x = R * cos((0) * dt);
			z = R * sin((0) * dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0:
				vertices[i + coordenada] = x;
				break;
			case 1:
				vertices[i + coordenada] = y;
				break;
			case 2:
				vertices[i + coordenada] = z;
				break;
			case 3:
				vertices[i + coordenada] = x;
				break;
			case 4:
				vertices[i + coordenada] = 0.5;
				break;
			case 5:
				vertices[i + coordenada] = z;
				break;
			}
		}
		coordenada += 6;
	}

	//ciclo for para crear la circunferencia inferior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices[coordenada + i] = x;
				break;
			case 1:
				vertices[coordenada + i] = -0.5f;
				break;
			case 2:
				vertices[coordenada + i] = z;
				break;
			}
		}
		coordenada += 3;
	}

	//ciclo for para crear la circunferencia superior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices[coordenada + i] = x;
				break;
			case 1:
				vertices[coordenada + i] = 0.5;
				break;
			case 2:
				vertices[coordenada + i] = z;
				break;
			}
		}
		coordenada += 3;
	}

	//Se generan los indices de los vértices
	for (i = 0; i < verticesBase; i++) {
		indices.push_back( i );
	}

	//se genera el mesh del cilindro
	Mesh* cilindro = new Mesh();
	cilindro->CreateMesh(&vertices[0], &indices[0], coordenada, verticesBase);
	meshList.push_back(cilindro);
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}



int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CrearCubo();
	CrearToroide(20, 20, 2, 1);
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	toroidTexture = Texture("Textures/metal.png");
	toroidTexture.LoadTexture();
	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	dadoTexture = Texture("Textures/dado.tga");
	dadoTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	Tagave = Texture("Textures/Agave.tga");
	Tagave.LoadTextureA();

	Kitt_M = Model();
	Kitt_M.LoadModel("Models/black_car.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta.obj");
	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");
	Camino_M = Model();
	Camino_M.LoadModel("Models/railroad track.obj");

	Dado_M = Model();
	Dado_M.LoadModel("Models/dadoanimales.obj");


	Blackhawk_M_Body = Model();
	Blackhawk_M_Body.LoadModel("Models/Black Hawk uh-60-Body.obj");

	Blackhawk_M_UBlade = Model();
	Blackhawk_M_UBlade.LoadModel("Models/Black Hawk uh-60-UBlade.obj");

	Blackhawk_M_BBlade = Model();
	Blackhawk_M_BBlade.LoadModel("Models/Black Hawk uh-60-BBlade2.obj");


	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	//posición inicial del helicóptero
	glm::vec3 posblackhawk = glm::vec3(-20.0f, 6.0f, -1.0);

	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	/*pointLights[0] = PointLight(0.0f, 0.0f, 1.0f,
		0.0f, 1.0f,
		2.0f, 1.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;*/

	unsigned int spotLightCount = 0;
	////linterna
	//spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
	//	0.0f, 2.0f,
	//	0.0f, 0.0f, 0.0f,
	//	0.0f, -1.0f, 0.0f,
	//	1.0f, 0.0f, 0.0f,
	//	5.0f);
	//spotLightCount++;

	//luz fija
	/*spotLights[1] = SpotLight(0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;*/
	
	//luz de helicóptero
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		-20.0f, 5.0f, -1.0,
		0.0f, -1.0f, 0.0f,
		0.1f, 0.05f, 0.001f,
		30.0f);
	spotLightCount++;

	//luz de faro
	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		-2.0f, 1.7f, 2.0f,
		-1.0f, 0.0f, 0.0f,
		0.5f, 0.01f, 0.0009f,
		25.0f);
	spotLightCount++;


	spotLights[2] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		-2.0f, 1.7f, -2.0f,
		-1.0f, 0.0f, 0.0f,
		0.5f, 0.01f, 0.0009f,
		25.0f);
	spotLightCount++;


	

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	
	
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		//luz ligada a la cámara de tipo flash 
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		//spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());
		spotLights[0].SetPos(glm::vec3(-20.0f + mainWindow.getHelix(), 6.0f + mainWindow.getHeliy(), -1.0));
		spotLights[1].SetPos( glm::vec3(-5.0f + mainWindow.getmuevex(), 1.7f, 2.0f + mainWindow.getmuevez()) );
		spotLights[2].SetPos( glm::vec3(-5.0f + mainWindow.getmuevex(), 1.7f, -2.0f + mainWindow.getmuevez()) );

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		glm::mat4 model(1.0);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pisoTexture.UseTexture();
		//agregar material al plano de piso
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		//agregar su coche y ponerle material
		model = glm::mat4(1.0);
		glm::mat4 matrizAux(1.0); //Matriz auxiliar para almacenar informacion a heredar
		model = glm::translate(model, glm::vec3( 1.0f + mainWindow.getmuevex(), 0.0f, mainWindow.getmuevez()) );
		matrizAux = model;
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel();

		// matriz de eje centro auto
		glm::mat4 matrizECentro(1.0);
		model = matrizAux;
		model = glm::translate(model, glm::vec3(0.0f, 1.2f, 0.0f));
		matrizECentro = model;


		// matriz eje trasero izq
		model = matrizECentro;
		model = glm::translate(model, glm::vec3(4.3f , 0.0f, 2.7f));
		matrizAux = model;

		//Agregar llanta trasera izquierda con jerarquía y rotación propia
		model = matrizAux;
		model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.3f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// matriz eje trasero der
		model = matrizECentro;
		model = glm::translate(model, glm::vec3(4.3f, 0.0f, -2.7f));
		matrizAux = model;

		//Agregar llanta trasera derecha
		model = matrizAux;
		model = glm::translate(model, glm::vec3(0.0, 0.0f, -0.3f));
		model = glm::rotate(model, 270 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// matriz eje frontal der
		model = matrizECentro;
		model = glm::translate(model, glm::vec3(-4.3f, 0.0f, -2.7f));
		model = glm::rotate(model, mainWindow.getWheelRot() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		matrizAux = model;

		//Agregar llanta frontal derecha con jerarquía y rotación propia
		model = matrizAux;
		model = glm::translate(model, glm::vec3(0.0, 0.0f, -0.3f));
		model = glm::rotate(model, 270 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// matriz eje frontal izq
		model = matrizECentro;
		model = glm::translate(model, glm::vec3(-4.3f, 0.0f, 2.7f));
		model = glm::rotate(model, mainWindow.getWheelRot() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		matrizAux = model;

		//Agregar llanta frontal izquierda con jerarquía y rotación propia
		model = matrizAux;
		model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.3f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();


		////Agregar llantas con jerarquía y rotación propia
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3( 0.0, 1.2f, 3.0f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//Llanta_M.RenderModel();


		// Helicoptero
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-20.0f + mainWindow.getHelix(), 6.0f + mainWindow.getHeliy(), -1.0));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		matrizAux = model;
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//agregar material al helicóptero
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//Blackhawk_M.RenderModel();
		Blackhawk_M_Body.RenderModel();

		//¿Cómo ligas la luz al helicóptero?
		
		// Matriz de eje de helice superior.
		model = matrizAux;
		//model = glm::translate(model, glm::vec3(0.0f, 6.8f, 0.0));
		model = glm::rotate(model, mainWindow.getHeliBlades() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		// Helices superiores
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Blackhawk_M_UBlade.RenderModel();

		// Matriz de eje de helice trasera.
		model = matrizAux;
		model = glm::translate(model, glm::vec3(10.5f, 2.3f, -0.4));
		model = glm::rotate(model, mainWindow.getHeliBlades() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

		// Helices trasera
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Blackhawk_M_BBlade.RenderModel();



		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.53f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Camino_M.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 30.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		toroidTexture.UseTexture();
		meshList[5]->RenderTorusMesh();


		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
