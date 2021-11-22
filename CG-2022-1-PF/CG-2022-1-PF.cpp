/*
Semestre 2022-1
Pr�ctica : Iluminaci�n
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
#include "UpperCamera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"
#include "include/irrKlang.h"


//para iluminaci�n
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"

irrklang::ISoundEngine *SoundEngine = irrklang::createIrrKlangDevice();
irrklang::ISoundEngine* Background = irrklang::createIrrKlangDevice();

const float PI = 3.14159265f;
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;
UpperCamera upperCamera;
glm::mat4 viewMatrix;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture dadoTexture;
Texture pisoTexture;
Texture Tagave;
Texture toroidTexture;


Skybox skyboxNight;
Skybox skyboxMorning;

//materiales
Material Material_brillante;
Material Material_opaco;
Material Material_Semibrillante;
Material Material_Semiopaco;
Material Material_opacoBrillante;

//luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Estrella
float estrellaAngle = 0.0f;
float estrellaAngleOffset = 1.0f;
float estrellaAux = 0.0f;
float estrellaAuxOffset = 0.2f;
float estrellaRadioAngle = 90.0f;
float estrellaRadioAngleOffset = 0.03f;
float estrellaAltura = 28.0f;
float estrellaAlturaOffset = 0.05f;
bool estrellaSubida = true;

//control del ciclo dia y noche.
GLfloat tiempo = 0.2f;
GLfloat tiempoOffset = 0.00025f;

//control personaje principal
float padoruAngle = 0.0;
float padoruAngleOffset = 0.5;
float padoruExtrAngle = 0.0;
float padoruExtrAngleOffset = 0.5;
float padoruJumpAngle = 0.0;
float padoruJumpAngleOffset = 2.0;
float padoruJumpY = 0.0;
float padoruJumpYOffset = 0.3;
bool padoruSoundDone = false;

// control ciclo hutao y qiqi
glm::mat4 QHCicleCenter(1.0);

//control Qiqi
float qiqiDerPie = -30.0;
float qiqiDerMuslo = 0.0;
float qiqiIzqPie = -30.0;
float qiqiIzqMuslo = 0.0;
float qiqiExtrAngleOffset = 2.5;
float qiqiAngle = 0.0;
float qiqiAngleOffset = 1.0;

//control Hutao
float hutaoDerPie = -40.0;
float hutaoDerMuslo = 0.0;
float hutaoIzqPie = -40.0;
float hutaoIzqMuslo = 0.0;
float hutaoExtrAngleOffset = 2.0;
float hutaoAngle = 0.0;
float hutaoAngleOffset = 1.0;

//control pi�ata
float pinataMovAngle = 0.0f;
float pinataMovOffset = 1.0f;
bool XfilesSoundDone = false;
GLfloat tiempoIni = 0.0;
GLfloat tiempoFin = 0.0;

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//c�lculo del promedio de las normales para sombreado de Phong
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

void CrearToroide(int mainSegments, int tubeSegments, float mainRadius, float tubeRadius) {

	int numVertices = (mainSegments + 1) * (tubeSegments + 1);
	int primitiveRestartIndex = numVertices;
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
			torus_vertices_vector.push_back(-cosMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = cosMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(-sinMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = sinMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(-sinTubeSegment);
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

// misma estructura, con mayor numero de repeticion de la textura
void CrearToroidePiso(int mainSegments, int tubeSegments, float mainRadius, float tubeRadius) { 

	int numVertices = (mainSegments + 1) * (tubeSegments + 1);
	int primitiveRestartIndex = numVertices;
	int numIndices = (mainSegments * 2 * (tubeSegments + 1)) + mainSegments - 1;
	//size_t coordenada = 0;

	//GLfloat* torus_vertices = (GLfloat*)calloc(sizeof(GLfloat*), (numVertices));
	std::vector<GLfloat> torus_vertices_vector;

	// coordenadas
	GLfloat mainSegmentAngleStep = glm::radians(360.0f / float(mainSegments));
	GLfloat tubeSegmentAngleStep = glm::radians(360.0f / float(tubeSegments));

	// texturas
	GLfloat mainSegmentTextureStep = 16.0f / float(mainSegments);
	GLfloat tubeSegmentTextureStep = 4.0f / float(tubeSegments);
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
			torus_vertices_vector.push_back(-cosMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = cosMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(-sinMainSegment * cosTubeSegment);
			/*torus_vertices[coordenada] = sinMainSegment * cosTubeSegment;
			coordenada++;*/
			torus_vertices_vector.push_back(-sinTubeSegment);
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
	int n, i;
	//n�mero de v�rtices ocupados
	//int verticesBase = (res + 1) * 6;
	//c�lculo del paso interno en la circunferencia y variables que almacenar�n cada coordenada de cada v�rtice
	GLfloat dt = glm::radians(360 / float(res)), x, z, y = -height;
	GLfloat textureSegmentStep = 1.0 / float(res);
	//apuntadores para guardar todos los v�rtices e �ndices generados
	//GLfloat* vertices = (GLfloat*)calloc(sizeof(GLfloat*), (verticesBase) * 3);
	std::vector<GLfloat> vertices; // vertices Paredes
	std::vector<GLfloat> verticesCI; // vertices circulo inferior
	std::vector<GLfloat> verticesCS; // vertices circulo superior
	//unsigned int* indices = (unsigned int*)calloc(sizeof(unsigned int*), verticesBase);
	std::vector<unsigned int> indices; // indices paredes
	std::vector<unsigned int> indicesCI; // indices circulo inferior
	std::vector<unsigned int> indicesCS; // indices circulo superior

	//ciclo for para crear los v�rtices de las paredes del cilindro
	GLfloat currentTextureSegment = 0;
	for (n = 0; n <= (res); n++) {

		x = R * cos((n)*dt);
		z = R * sin((n)*dt);

		for (i = 0; i < 16; i++) {
			switch (i) {
			case 0:// vertice inferior
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			case 3:// texturas
				vertices.push_back(currentTextureSegment);
				break;
			case 4:
				vertices.push_back(0.0f);
				break;
			case 5://normales
				vertices.push_back(-x / R);
				break;
			case 6:
				vertices.push_back(0.0f);
				break;
			case 7:
				vertices.push_back(-z / R);
				break;
			case 8:// vertice superior
				vertices.push_back(x);
				break;
			case 9:
				vertices.push_back(-y);
				break;
			case 10:
				vertices.push_back(z);
				break;
			case 11:// texturas
				vertices.push_back(currentTextureSegment);
				break;
			case 12:
				vertices.push_back(1.0f);
				break;
			case 13://normales
				vertices.push_back(-x / R);
				break;
			case 14:
				vertices.push_back(0.0f);
				break;
			case 15:
				vertices.push_back(-z / R);
				break;
			}
		}
		currentTextureSegment += textureSegmentStep;
	}
	// indices de la pared
	for (i = 0; i < vertices.size(); i++) {
		indices.push_back(i);
	}


	//ciclo for para crear la circunferencia inferior
	// vertice central inferior

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 8; i++) {
			switch (i) {
			case 0:// vertice inferior
				verticesCI.push_back(x);
				break;
			case 1:
				verticesCI.push_back(y);
				break;
			case 2:
				verticesCI.push_back(z);
				break;
			case 3:// texturas
				verticesCI.push_back((0.5f * x / R) + 0.5f);
				break;
			case 4:
				verticesCI.push_back((0.5f * z / R) + 0.5f);
				break;
			case 5://normales
				verticesCI.push_back(0.0f);
				break;
			case 6:
				verticesCI.push_back(1.0f);
				break;
			case 7:
				verticesCI.push_back(0.0f);
				break;
			}
		}
	}

	// indices del circulo inferior
	for (i = 0; i < verticesCI.size(); i++) {
		indicesCI.push_back(i);
	}

	//ciclo for para crear la circunferencia superior

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 8; i++) {
			switch (i) {
			case 0:// vertice superior
				verticesCS.push_back(x);
				break;
			case 1:
				verticesCS.push_back(-y);
				break;
			case 2:
				verticesCS.push_back(z);
				break;
			case 3:// texturas
				verticesCS.push_back((0.5f * x / R) + 0.5f);
				break;
			case 4:
				verticesCS.push_back((0.5f * z / R) + 0.5f);
				break;
			case 5://normales
				verticesCS.push_back(0.0f);
				break;
			case 6:
				verticesCS.push_back(-1.0f);
				break;
			case 7:
				verticesCS.push_back(0.0f);
				break;
			}
		}
	}

	// indices del circulo superior
	for (i = 0; i < verticesCS.size(); i++) {
		indicesCS.push_back(i);
	}


	//se genera el mesh de la pared del cilindro
	Mesh* cilindroP = new Mesh();
	cilindroP->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
	meshList.push_back(cilindroP);

	//se genera el mesh del circulo inferior del cilindro
	Mesh* cilindroCI = new Mesh();
	cilindroCI->CreateMesh(&verticesCI[0], &indicesCI[0], verticesCI.size(), indicesCI.size());
	meshList.push_back(cilindroCI);

	//se genera el mesh del circulo superior del cilindro
	Mesh* cilindroCS = new Mesh();
	cilindroCS->CreateMesh(&verticesCS[0], &indicesCS[0], verticesCS.size(), indicesCS.size());
	meshList.push_back(cilindroCS);
}

void CrearCono(int res, float height, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//n�mero de v�rtices ocupados
	//int verticesBase = (res + 1) * 6;
	//c�lculo del paso interno en la circunferencia y variables que almacenar�n cada coordenada de cada v�rtice
	GLfloat dt = glm::radians(360 / float(res)), x, z, y = height;
	GLfloat textureSegmentStep = 1.0 / float(res);
	//apuntadores para guardar todos los v�rtices e �ndices generados
	//GLfloat* vertices = (GLfloat*)calloc(sizeof(GLfloat*), (verticesBase) * 3);
	std::vector<GLfloat> verticesCI; // vertices circulo inferior
	std::vector<GLfloat> verticesCS; // vertices circulo superior
	//unsigned int* indices = (unsigned int*)calloc(sizeof(unsigned int*), verticesBase);
	std::vector<unsigned int> indicesCI; // indices circulo inferior
	std::vector<unsigned int> indicesCS; // indices circulo superior


	//ciclo for para crear la circunferencia inferior
	// vertice central inferior

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 8; i++) {
			switch (i) {
			case 0:// vertice inferior
				verticesCI.push_back(x);
				break;
			case 1:
				verticesCI.push_back(0.0);
				break;
			case 2:
				verticesCI.push_back(z);
				break;
			case 3:// texturas
				verticesCI.push_back((0.5f * x / R) + 0.5f);
				break;
			case 4:
				verticesCI.push_back((0.5f * z / R) + 0.5f);
				break;
			case 5://normales
				verticesCI.push_back(0.0f);
				break;
			case 6:
				verticesCI.push_back(1.0f);
				break;
			case 7:
				verticesCI.push_back(0.0f);
				break;
			}
		}
	}

	// indices del circulo inferior
	for (i = 0; i < verticesCI.size(); i++) {
		indicesCI.push_back(i);
	}

	//ciclo for para crear la circunferencia superior
	verticesCS.push_back(0.0);
	verticesCS.push_back(y);
	verticesCS.push_back(0.0);
	// texturas
	verticesCS.push_back(0.5f);
	verticesCS.push_back(0.5f);
	//normales
	verticesCS.push_back(0.0f);
	verticesCS.push_back(-1.0f);
	verticesCS.push_back(0.0f);

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 8; i++) {
			switch (i) {
			case 0:// vertice superior
				verticesCS.push_back(x);
				break;
			case 1:
				verticesCS.push_back(0.0);
				break;
			case 2:
				verticesCS.push_back(z);
				break;
			case 3:// texturas
				verticesCS.push_back((0.5f * x / R) + 0.5f);
				break;
			case 4:
				verticesCS.push_back((0.5f * z / R) + 0.5f);
				break;
			case 5://normales
				verticesCS.push_back(-x / R);
				break;
			case 6:
				verticesCS.push_back(-1.0f);
				break;
			case 7:
				verticesCS.push_back(-z / R);
				break;
			}
		}
	}

	// indices del circulo superior
	for (i = 0; i < verticesCS.size(); i++) {
		indicesCS.push_back(i);
	}


	//se genera el mesh del circulo inferior del cilindro
	Mesh* cilindroCI = new Mesh();
	cilindroCI->CreateMesh(&verticesCI[0], &indicesCI[0], verticesCI.size(), indicesCI.size());
	meshList.push_back(cilindroCI);

	//se genera el mesh del circulo superior del cilindro
	Mesh* cilindroCS = new Mesh();
	cilindroCS->CreateMesh(&verticesCS[0], &indicesCS[0], verticesCS.size(), indicesCS.size());
	meshList.push_back(cilindroCS);
}

void CrearEsfera( float radio,  int numStacks) {

	std::vector<GLfloat> esfera_vertices_vector;
	std::vector<unsigned int> esfera_indices_vector;
	std::vector<unsigned int> esfera_indices_PN_vector;
	std::vector<unsigned int> esfera_indices_PS_vector;

	// Pre-calculate sines / cosines for given number of slices
	float SliceAngleStep = 2.0f*PI / float(numStacks);
	float currentSliceAngle = 0.0f;
	std::vector<float> sliceSines, sliceCosines;
	for (auto i = 0; i <= numStacks; i++)
	{
		sliceSines.push_back(sin(currentSliceAngle));
		sliceCosines.push_back(cos(currentSliceAngle));

		// Update stack angle
		currentSliceAngle += SliceAngleStep;
	}

	// Pre-calculate sines / cosines for given number of stacks
	float stackAngleStep = -PI / float(numStacks);
	float currentStackAngle = PI / 2.0f;
	std::vector<float> stackSines, stackCosines;
	for (auto i = 0; i <= numStacks; i++)
	{
		stackSines.push_back(sin(currentStackAngle));
		stackCosines.push_back(cos(currentStackAngle));

		// Update stack angle
		currentStackAngle += stackAngleStep;
	}
	
	// generacion de los vectores
	for (size_t i = 0; i <= numStacks; i++)
	{
		for (size_t j = 0; j <= numStacks; j++)
		{
			// Incersion de vectores de posicion
			float x = radio * stackCosines[i] * sliceCosines[j];
			float y = radio * stackSines[i];
			float z = radio * stackCosines[i] * sliceSines[j];
			//_vbo.addData(glm::vec3(x, y, z));
			esfera_vertices_vector.push_back(x);
			esfera_vertices_vector.push_back(y);
			esfera_vertices_vector.push_back(z);

			// incersion de texturas
			float u = 1.0f - float(j) / numStacks;
			float v = 1.0f - float(i) / numStacks;
			//_vbo.addData(glm::vec2(u, v));
			esfera_vertices_vector.push_back(u);
			esfera_vertices_vector.push_back(v);

			// incersion de normales
			esfera_vertices_vector.push_back(-x/radio);
			esfera_vertices_vector.push_back(-y/radio);
			esfera_vertices_vector.push_back(-z/radio);
			
		}
	}

	/// indices

	// generacion de indices para el polo superior
	for (int i = 0; i < numStacks; i++)
	{
		GLuint sliceIndex = i;
		GLuint nextSliceIndex = sliceIndex + numStacks + 1;
		esfera_indices_PN_vector.push_back(float((sliceIndex)));
		esfera_indices_PN_vector.push_back(float((nextSliceIndex)));
		esfera_indices_PN_vector.push_back(float((nextSliceIndex + 1)));
		//esfera_indices_PN_vector.push_back(i);
		//_indicesVBO.addData(static_cast(sliceIndex));
		//_indicesVBO.addData(static_cast(nextSliceIndex));
		//_indicesVBO.addData(static_cast(nextSliceIndex + 1));
	}

	int numVertices = (numStacks + 1) * (numStacks + 1);
	int primitiveRestartIndex = numVertices;

	// generacion de indices para el cuerpo de la esfera
	GLuint currentVertexIndex = numStacks + 1;
	for (int i = 0; i < (numStacks-2); i++)
	{
		// Primitive restart triangle strip from second body stack on
		if (i > 0)
		{
			esfera_indices_vector.push_back(primitiveRestartIndex);
			//_indicesVBO.addData(primitiveRestartIndex);
		}

		for (int j = 0; j <= numStacks; j++)
		{
			GLuint sliceIndex = currentVertexIndex + j;
			GLuint nextSliceIndex = currentVertexIndex + numStacks + 1 + j;
			esfera_indices_vector.push_back(sliceIndex);
			esfera_indices_vector.push_back(nextSliceIndex);
			//_indicesVBO.addData(sliceIndex);
			//_indicesVBO.addData(nextSliceIndex);
		}

		currentVertexIndex += numStacks + 1;
	}

	// And finally south pole (triangles again)
	GLuint beforeLastStackIndexOffset = numVertices - 2 * (numStacks + 1);
	for (int i = 0; i < numStacks; i++)
	{
		GLuint sliceIndex = beforeLastStackIndexOffset + i;
		GLuint nextSliceIndex = sliceIndex + numStacks + 1;
		esfera_indices_PS_vector.push_back(sliceIndex);
		esfera_indices_PS_vector.push_back(sliceIndex + 1);
		esfera_indices_PS_vector.push_back(nextSliceIndex);

		//esfera_indices_PS_vector.push_back(i);
		//_indicesVBO.addData(static_cast(sliceIndex));
		//_indicesVBO.addData(static_cast(sliceIndex + 1));
		//_indicesVBO.addData(static_cast(nextSliceIndex));
	}

	//la primera capa superior de la esfera
	Mesh* esferaPN = new Mesh();
	esferaPN->CreateMesh(&esfera_vertices_vector[0], &esfera_indices_PN_vector[0], esfera_vertices_vector.size(), esfera_indices_PN_vector.size());
	meshList.push_back(esferaPN);

	//El cuerpo de la esfera.
	Mesh* esferaC = new Mesh();
	esferaC->CreateMesh(&esfera_vertices_vector[0], &esfera_indices_vector[0], esfera_vertices_vector.size(), esfera_indices_vector.size());
	meshList.push_back(esferaC);

	//la ultima capa inferior de la esfera
	Mesh* esferaPS = new Mesh();
	esferaPS->CreateMesh(&esfera_vertices_vector[0], &esfera_indices_PS_vector[0], esfera_vertices_vector.size(), esfera_indices_PS_vector.size());
	meshList.push_back(esferaPS);
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

/****************************************************************************************************/
/****************************************************************************************************/
//										Funciones de animacion
/****************************************************************************************************/
/****************************************************************************************************/

// animacion personaje principal
void padoruAnimation() {
	if (!padoruSoundDone) {
		SoundEngine->play2D("Sounds/padoru_sound.mp3");
		padoruSoundDone = true;
	}
	padoruAngle += padoruAngleOffset * deltaTime;
	if (padoruAngle >= 360.0) {
		padoruAngle = 0.0;
		padoruSoundDone = false;
		mainWindow.setMainStart(false);
	}

	if (padoruExtrAngle >= 20.0 or padoruExtrAngle <= -20.0) {
		padoruExtrAngleOffset = -padoruExtrAngleOffset;
	}

	if (padoruAngle > 180 and padoruAngle < 270) { // 90 grados para saltar
		padoruJumpAngle += padoruJumpAngleOffset * deltaTime;
		if (padoruAngle < 225) {
			padoruJumpY += padoruJumpYOffset*deltaTime;
		}
		else {
			padoruJumpY -= padoruJumpYOffset*deltaTime;
		}

		if (padoruAngle >= 270) {
			padoruJumpAngle = 0.0f;
			padoruJumpY = 0.0f;
		}
	}
	padoruExtrAngle += padoruExtrAngleOffset;
}

void qiqiAnimation() {
	if (qiqiAngle >= 360) {
		qiqiAngle = 0.0;
	}

	if (qiqiDerMuslo > 30.0 ) {
		qiqiExtrAngleOffset = -qiqiExtrAngleOffset;
		qiqiDerPie = -1.0;
		qiqiDerMuslo = 29.0;
		qiqiIzqPie = -59.0;
		qiqiIzqMuslo = -29.0;
	}
	if (qiqiDerMuslo < -30.0) {
		qiqiExtrAngleOffset = -qiqiExtrAngleOffset;
		qiqiDerPie = -59.0;
		qiqiDerMuslo = -29.0;
		qiqiIzqPie = -1.0;
		qiqiIzqMuslo = 29.0;
	}
	qiqiDerMuslo += qiqiExtrAngleOffset * deltaTime;
	qiqiDerPie += qiqiExtrAngleOffset * deltaTime;
	qiqiIzqMuslo -= qiqiExtrAngleOffset * deltaTime;
	qiqiIzqPie -= qiqiExtrAngleOffset * deltaTime;
	qiqiAngle += qiqiAngleOffset * deltaTime;
}

void hutaoAnimation() {
	if (hutaoAngle >= 360) {
		hutaoAngle = 0.0;
	}

	if (hutaoDerMuslo > 40.0) {
		hutaoExtrAngleOffset = -hutaoExtrAngleOffset;
		hutaoDerPie = -79.0;
		hutaoDerMuslo = 39.0;
		hutaoIzqPie = -1.0;
		hutaoIzqMuslo = -39.0;
	}
	if (hutaoDerMuslo < -40.0) {
		hutaoExtrAngleOffset = -hutaoExtrAngleOffset;
		hutaoDerPie = -1.0;
		hutaoDerMuslo = -39.0;
		hutaoIzqPie = -79.0;
		hutaoIzqMuslo = 39.0;
	}
	hutaoDerMuslo += hutaoExtrAngleOffset * deltaTime;
	hutaoDerPie -= hutaoExtrAngleOffset * deltaTime;
	hutaoIzqMuslo -= hutaoExtrAngleOffset * deltaTime;
	hutaoIzqPie += hutaoExtrAngleOffset * deltaTime;
	hutaoAngle += hutaoAngleOffset * deltaTime;
}

void pinataMov() {

	if (!XfilesSoundDone) {
		SoundEngine->play2D("Sounds/Xfiles.mp3");
		XfilesSoundDone = true;
		tiempoIni = glfwGetTime();
	}

	if (pinataMovAngle > 50.0) {
		pinataMovOffset = -pinataMovOffset;
		pinataMovAngle = 49.0;
	}
	if (pinataMovAngle < -50.0) {
		pinataMovOffset = -pinataMovOffset;
		pinataMovAngle = -49.0;
	}
	pinataMovAngle += pinataMovOffset * deltaTime;
	tiempoFin = glfwGetTime();
	if (tiempoFin - tiempoIni > 15.0f) {
		XfilesSoundDone = false;
	}
}

int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CrearCubo();
	CrearToroide(20, 20, 2, 1);
	CrearCilindro( 10, 3, 1 );
	CrearCono( 10, 3, 1 );
	CrearEsfera( 1, 20 );
	CreateShaders();
	CrearToroidePiso(20, 5, 3, 0.5); // pasillo central

	camera = Camera(glm::vec3(0.0f, 10.0f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 2.0f, 0.5f);
	upperCamera = UpperCamera( 1.0f );

	//control audio
	SoundEngine->setSoundVolume( 0.5 );
	Background->setSoundVolume(0.2);

	toroidTexture = Texture("Textures/metal.png");
	toroidTexture.LoadTexture();
	Texture rocastex = Texture("Textures/rosca1.png");
	rocastex.LoadTextureA();
	Texture madera = Texture("Textures/madera.png");
	madera.LoadTexture();
	Texture pinata_center = Texture("Textures/pinata.png");
	pinata_center.LoadTexture();
	Texture pinata_barro = Texture("Textures/pinata_barro.png");
	pinata_barro.LoadTexture();
	Texture pinata_rosa = Texture("Textures/pinata_rosa.png");
	pinata_rosa.LoadTexture();
	Texture pinata_azul = Texture("Textures/pinata_azul.png");
	pinata_azul.LoadTexture();
	Texture pinata_verde = Texture("Textures/pinata_verde.png");
	pinata_verde.LoadTexture();
	Texture pinata_turquesa = Texture("Textures/pinata_turquesa.png");
	pinata_turquesa.LoadTexture();
	Texture pinata_rojo = Texture("Textures/pinata_rojo.png");
	pinata_rojo.LoadTexture();
	Texture pisoBrick = Texture("Textures/piso_hielo_cuad.png");
	pisoBrick.LoadTextureA();
	Texture arbol_tex = Texture("Textures/arbol.png");
	arbol_tex.LoadTexture();
	Texture candy_tex = Texture("Textures/candy_cane.jpg");
	candy_tex.LoadTexture();
	Texture faro_tex = Texture("Textures/faro.png");
	faro_tex.LoadTexture();
	Texture bush_tex = Texture("Textures/bush.png");
	bush_tex.LoadTexture();
	Texture edificio1_tex = Texture("Textures/edificio1.png");
	edificio1_tex.LoadTextureA();
	Texture edificio2_tex = Texture("Textures/edificio2.png");
	edificio2_tex.LoadTextureA();
	pisoTexture = Texture("Textures/nieve.jpg");
	pisoTexture.LoadTextureA();


	// Modelos
	Model arbol_central = Model();
	arbol_central.LoadModel("Modelos_Listos/arbol.obj");
	Model candy_cane = Model();
	candy_cane.LoadModel("Modelos_Listos/candy_cane.obj");
	Model faro = Model();
	faro.LoadModel("Modelos_Listos/faro.obj");
	Model bush = Model();
	bush.LoadModel("Models/bush.obj");
	Model edificio1 = Model();
	edificio1.LoadModel("Models/edificio1.obj");
	Model edificio2 = Model();
	edificio2.LoadModel("Models/edificio2.obj");


	// modelo personaje principal
	Texture padoru_tex = Texture("Textures/padoru.png");
	padoru_tex.LoadTexture();
	Model padoru_cuerpo = Model();
	padoru_cuerpo.LoadModel("Models/Padoru/cuerpo.obj");
	Model padoru_cabeza = Model();
	padoru_cabeza.LoadModel("Models/Padoru/cabeza.obj");
	Model padoru_brazo_der = Model();
	padoru_brazo_der.LoadModel("Models/Padoru/brazo_der.obj");
	Model padoru_brazo_izq = Model();
	padoru_brazo_izq.LoadModel("Models/Padoru/brazo_izq.obj");
	Model padoru_pierna_der = Model();
	padoru_pierna_der.LoadModel("Models/Padoru/pierna_der.obj");
	Model padoru_pierna_izq = Model();
	padoru_pierna_izq.LoadModel("Models/Padoru/pierna_izq.obj");

	// modelo Qiqi
	Texture qiqi_tex = Texture("Textures/qiqi.png");
	qiqi_tex.LoadTexture();
	Model qiqi_cuerpo = Model();
	qiqi_cuerpo.LoadModel("Models/qiqi/qiqi_cuerpo.obj");
	Model qiqi_muslo_der = Model();
	qiqi_muslo_der.LoadModel("Models/qiqi/qiqi_muslo_der.obj");
	Model qiqi_muslo_izq = Model();
	qiqi_muslo_izq.LoadModel("Models/qiqi/qiqi_muslo_izq.obj");
	Model qiqi_patorrilla_der = Model();
	qiqi_patorrilla_der.LoadModel("Models/qiqi/qiqi_pantorrilla_der.obj");
	Model qiqi_patorrilla_izq = Model();
	qiqi_patorrilla_izq.LoadModel("Models/qiqi/qiqi_pantorrilla_izq.obj");

	// modelo Hu Tao

	Texture hutao_tex = Texture("Textures/hutao.png");
	hutao_tex.LoadTexture();
	Model hutao_cuerpo = Model();
	hutao_cuerpo.LoadModel("Models/hutao/hutao_cuerpo.obj");
	Model hutao_muslo_der = Model();
	hutao_muslo_der.LoadModel("Models/hutao/hutao_muslo_der.obj");
	Model hutao_muslo_izq = Model();
	hutao_muslo_izq.LoadModel("Models/hutao/hutao_muslo_izq.obj");
	Model hutao_patorrilla_der = Model();
	hutao_patorrilla_der.LoadModel("Models/hutao/hutao_pantorrilla_der.obj");
	Model hutao_patorrilla_izq = Model();
	hutao_patorrilla_izq.LoadModel("Models/hutao/hutao_pantorrilla_izq.obj");
	

	std::vector<std::string> skyboxFacesNight;
	skyboxFacesNight.push_back("Textures/Skybox/night_rt.tga");
	skyboxFacesNight.push_back("Textures/Skybox/night_lf.tga");
	skyboxFacesNight.push_back("Textures/Skybox/night_dw.tga");
	skyboxFacesNight.push_back("Textures/Skybox/night_up.tga");
	skyboxFacesNight.push_back("Textures/Skybox/night_bk.tga");
	skyboxFacesNight.push_back("Textures/Skybox/night_ft.tga");
	Model estrella = Model();
	estrella.LoadModel("Models/estrellaMario.obj");
	Model trineo = Model();
	trineo.LoadModel("Models/trineo.obj");
	Model reno_1 = Model();
	reno_1.LoadModel("Models/reno_1.obj");
	Model reno_2 = Model();
	reno_2.LoadModel("Models/reno_2.obj");
	Model regalo_1 = Model();
	regalo_1.LoadModel("Models/regalo_1.obj");
	Model regalo_2 = Model();
	regalo_2.LoadModel("Models/regalo_2.obj");
	Model regalo_3 = Model();
	regalo_3.LoadModel("Models/regalo_3.obj");
	Model pinguino_1 = Model();
	pinguino_1.LoadModel("Models/pinguino_1.obj");
	Model pinguino_2 = Model();
	pinguino_2.LoadModel("Models/pinguino_2.obj");
	Model grape_kun = Model();
	grape_kun.LoadModel("Models/grape_kun_2.obj");
	Model iglu = Model();
	iglu.LoadModel("Models/iglu.obj");
	Model oso_polar = Model();
	oso_polar.LoadModel("Models/oso_polar.obj");
	Model ice_climbers = Model();
	ice_climbers.LoadModel("Models/ice_climbers.obj");
	Model casa_galleta = Model();
	casa_galleta.LoadModel("Models/casa_galleta.obj");
	Model hombre_galleta = Model();
	hombre_galleta.LoadModel("Models/hombre_galleta.obj");
	Model arbol_galleta = Model();
	arbol_galleta.LoadModel("Models/arbol_galleta.obj");
	Model candy_cane_2 = Model();
	candy_cane_2.LoadModel("Models/candy_cane_2.obj");
	Model knuckles = Model();
	knuckles.LoadModel("Models/Knuckles/u_knuckles.obj");
	Model iceberg_1 = Model();
	iceberg_1.LoadModel("Models/iceberg_1.obj");
	Model iceberg_2 = Model();
	iceberg_2.LoadModel("Models/iceberg_2.obj");
	Model iceberg_3 = Model();
	iceberg_3.LoadModel("Models/iceberg_3.obj");

	std::vector<std::string> skyboxFacesMorning;
	skyboxFacesMorning.push_back("Textures/Skybox/morning_rt.tga");
	skyboxFacesMorning.push_back("Textures/Skybox/morning_lf.tga");
	skyboxFacesMorning.push_back("Textures/Skybox/morning_dw.tga");
	skyboxFacesMorning.push_back("Textures/Skybox/morning_up.tga");
	skyboxFacesMorning.push_back("Textures/Skybox/morning_bk.tga");
	skyboxFacesMorning.push_back("Textures/Skybox/morning_ft.tga");

	//skybox = Skybox(skyboxFaces);
	skyboxNight = Skybox(skyboxFacesNight);
	skyboxMorning = Skybox(skyboxFacesMorning);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);
	Material_Semibrillante = Material( 3.0f, 100);
	Material_Semiopaco = Material(0.5f, 50);
	Material_opacoBrillante = Material( 1.0, 10 );

	/****************************************************************************************************/
	/****************************************************************************************************/
	//										Defincion luces
	/****************************************************************************************************/
	/****************************************************************************************************/

	//luz direccional, s�lo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		tiempo, 0.3f,//0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	
	//Declaraci�n de primer luz puntual
	pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 100.0f, 0.0f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;
	pointLights[1] = PointLight(0.127f, 0.787f, 0.199f,
		0.0f, 1.0f,
		0.0f, 26.0f, -110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;
	pointLights[2] = PointLight(0.127f, 0.787f, 0.199f,
		0.0f, 1.0f,
		0.0f, 26.0f, 110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;
	pointLights[3] = PointLight(0.866f, 0.192f, 0.192f,
		0.0f, 1.0f,
		sin(60 * toRadians) * 110.0f, 26.0f, cos(60 * toRadians) * 110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;
	pointLights[4] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		sin(120 * toRadians) * 110.0f, 26.0f, cos(120 * toRadians) * 110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;
	pointLights[5] = PointLight(0.866f, 0.192f, 0.192f,
		0.0f, 1.0f,
		-sin(60 * toRadians) * 110.0f, 26.0f, -cos(60 * toRadians) * 110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;
	pointLights[6] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		-sin(120 * toRadians) * 110.0f, 26.0f, -cos(120 * toRadians) * 110.0f,
		0.3f, 0.06f, 0.000001f);
	pointLightCount++;

	unsigned int spotLightCount = 0;

	//luz fija
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		-170.0f, 150.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;
	spotLights[3] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		170.0f, 150.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;
	spotLights[2] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		sin(30 * toRadians) * 170.0f, 150.0f, cos(30 * toRadians) * 170.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;
	spotLights[4] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		sin(150 * toRadians) * 170.0f, 150.0f, cos(150 * toRadians) * 170.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;
	spotLights[5] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		-sin(30 * toRadians) * 170.0f, 150.0f, -cos(30 * toRadians) * 170.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;
	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 2.0f,
		-sin(150 * toRadians) * 170.0f, 150.0f, -cos(150 * toRadians) * 170.0f,
		0.0f, -1.0f, 0.0f,
		0.03f, 0.0008f, 0.0001f,
		20.0f);
	spotLightCount++;

	mainWindow.setOffSpotLights( spotLightCount );

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 500.0f);
	
	lastTime = glfwGetTime();
	Background->play2D("Sounds/fondo.mp3", true);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		// control del tiempo
		mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
			tiempo, 0.3f,//0.3f, 0.3f,
			0.0f, 0.0f, -1.0f);
		if (tiempo < 0.1 or tiempo > 1.0) {
			tiempoOffset = -tiempoOffset;
		}
		tiempo += tiempoOffset*deltaTime;


		//Recibir eventos del usuario
		glfwPollEvents();
		if ( mainWindow.getFreeCamera() ) {
			camera.keyControl(mainWindow.getsKeys(), deltaTime);
			camera.mouseControl(mainWindow.getXChange());
			viewMatrix = camera.calculateViewMatrix();
		}
		else
		{
			upperCamera.keyControl(mainWindow.getsKeys(), deltaTime);
			viewMatrix = upperCamera.calculateViewMatrix();
		}

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// control de escenario del tiempo
		//skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		if (tiempo < 0.6f) {
			skyboxNight.DrawSkybox(viewMatrix, projection);
		}
		else {
			skyboxMorning.DrawSkybox(viewMatrix, projection);
		}
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();

		//informaci�n en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		if (mainWindow.getFreeCamera()) {
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
			glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		}
		else
		{
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(upperCamera.calculateViewMatrix()));
			glUniform3f(uniformEyePosition, upperCamera.getCameraPosition().x, upperCamera.getCameraPosition().y, upperCamera.getCameraPosition().z);
		}


		//informaci�n al shader de fuentes de iluminaci�n
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount - mainWindow.getOffSpotLights());

		/****************************************************************************************************/
		/****************************************************************************************************/
		//										Control animaciones
		/****************************************************************************************************/
		/****************************************************************************************************/

		// control personje principal
		if ( mainWindow.getMainStart() ) {
			padoruAnimation();
		}

		qiqiAnimation();
		hutaoAnimation();

		if (mainWindow.getAni1Start()) {
			pinataMov();
		}

		/****************************************************************************************************/
		/****************************************************************************************************/
		//										Modelado geometrico
		/****************************************************************************************************/
		/****************************************************************************************************/

		// piso

		glm::mat4 model(1.0);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f, 1.0f, 100.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pisoTexture.UseTexture();
		//agregar material al plano de piso
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		/****************************************************************************************************/
		/****************************************************************************************************/
		//										Modelado geometrico
		/****************************************************************************************************/
		/****************************************************************************************************/

		// cubo - mesa

		glm::mat4 mesaModel(1.0f);
		glm::mat4 matAux(1.0f);

		model = glm::mat4(1.0);
		mesaModel = glm::translate(mesaModel, glm::vec3(80.0f, 2.0f, 90.0f));
		mesaModel = glm::rotate(mesaModel, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		mesaModel = glm::scale(mesaModel, glm::vec3(5.0f, 5.0f, 5.0f));

		model = glm::rotate(mesaModel, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.1f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		madera.UseTexture();
		//agregar material al plano de piso
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();

		// cilindro - pata mesa 1

		model = glm::mat4(1.0);
		model = glm::translate(mesaModel, glm::vec3(1.8f, -0.8f, 0.8f));
		model = glm::scale(model, glm::vec3(0.1f, 0.25f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		madera.UseTexture();
		meshList[6]->RenderStripMesh();
		meshList[7]->RenderFanMesh();
		meshList[8]->RenderFanMesh();

		// cilindro - pata mesa 2

		model = glm::mat4(1.0);
		model = glm::translate(mesaModel, glm::vec3(-1.8f, -0.8f, 0.8f));
		model = glm::scale(model, glm::vec3(0.1f, 0.25f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		madera.UseTexture();
		meshList[6]->RenderStripMesh();
		meshList[7]->RenderFanMesh();
		meshList[8]->RenderFanMesh();

		// cilindro - pata mesa 3

		model = glm::mat4(1.0);
		model = glm::translate(mesaModel, glm::vec3(1.8f, -0.8f, -0.8f));
		model = glm::scale(model, glm::vec3(0.1f, 0.25f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		madera.UseTexture();
		meshList[6]->RenderStripMesh();
		meshList[7]->RenderFanMesh();
		meshList[8]->RenderFanMesh();

		// cilindro - pata mesa 4

		model = glm::mat4(1.0);
		model = glm::translate(mesaModel, glm::vec3(-1.8f, -0.8f, -0.8f));
		model = glm::scale(model, glm::vec3(0.1f, 0.25f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		madera.UseTexture();
		meshList[6]->RenderStripMesh();
		meshList[7]->RenderFanMesh();
		meshList[8]->RenderFanMesh();

		// toroide - rosca

		model = glm::mat4(1.0);
		model = glm::translate(mesaModel, glm::vec3(0.0f, 0.3f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		rocastex.UseTexture();
		meshList[5]->RenderTorusMesh();


		/****************************************************************************************************/

		// esfera - pi�ata

		glm::mat4 pinataRot(1.0f);
		pinataRot = glm::translate(pinataRot, glm::vec3(94.0f, 22.0f, 150.0f));
		pinataRot = glm::rotate(pinataRot, pinataMovAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 pinataModel(1.0f);

		pinataModel = glm::mat4(1.0);
		//pinataModel = glm::translate(pinataModel, glm::vec3(100.0f, 10.0f, 160.0f));
		pinataModel = glm::translate(pinataRot, glm::vec3(0.0f, -5.0f, 0.0f));
		pinataModel = glm::rotate(pinataModel, 0 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		pinataModel = glm::scale(pinataModel, glm::vec3(3.0f, 3.0f, 3.0));

		model = glm::scale(pinataModel, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_center.UseTexture();
		Material_Semibrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[11]->RenderMesh();
		meshList[12]->RenderTorusMesh();
		meshList[13]->RenderMesh();

		// cono - pi�ata 1 inferior

		model = pinataModel;
		model = glm::translate(model, glm::vec3(0.0f, -0.7f, 0.0f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_turquesa.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// cono - pi�ata 2 superior

		model = pinataModel;
		model = glm::translate(model, glm::vec3(0.0f, 1.7f, 0.0f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_barro.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// cono - pi�ata 3

		model = pinataModel;
		model = glm::translate(model, glm::vec3(0.0f, cos(40) * 0.7f, sin(40) * 0.7f));
		model = glm::rotate(model, 125 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_verde.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// cono - pi�ata 4

		model = pinataModel;
		model = glm::translate(model, glm::vec3(0.0f, cos(40) * 0.7f, -sin(40) * 0.7f));
		model = glm::rotate(model, -125 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_azul.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// cono - pi�ata 5

		model = pinataModel;
		model = glm::translate(model, glm::vec3(sin(70) * -0.7f, cos(70) * 0.7f, 0.0f));
		model = glm::rotate(model, 70 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_rojo.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// cono - pi�ata 6

		model = pinataModel;
		model = glm::translate(model, glm::vec3(sin(70) * 0.7f, cos(70) * 0.7f, 0.0f));
		model = glm::rotate(model, -70 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.9f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinata_rosa.UseTexture();
		meshList[9]->RenderFanMesh();
		meshList[10]->RenderFanMesh();

		// toroide pasillo alrededor

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 5.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pisoBrick.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[14]->RenderTorusMesh();

		//// toroide

		//model = glm::mat4(1.0);
		//model = glm::translate(mesaModel, glm::vec3(0.0f, 0.3f, 0.0f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.3f, 0.3f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//rocastex.UseTexture();
		//meshList[5]->RenderTorusMesh();

		//// cilindro

		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 30.0f, 0.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//toroidTexture.UseTexture();
		//meshList[6]->RenderStripMesh();
		//meshList[7]->RenderFanMesh();
		//meshList[8]->RenderFanMesh();

		//// cono

		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 30.0f, -10.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//toroidTexture.UseTexture();
		//meshList[9]->RenderFanMesh();
		//meshList[10]->RenderFanMesh();

		//// esfera

		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(10.0f, 30.0f, 0.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//toroidTexture.UseTexture();
		//meshList[11]->RenderMesh();
		//meshList[12]->RenderTorusMesh();
		//meshList[13]->RenderMesh();

		/****************************************************************************************************/
		/****************************************************************************************************/
		//									Uso de modelos optimizados.
		/****************************************************************************************************/
		/****************************************************************************************************/

		// arbol central

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(25.0f, 25.0f, 25.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arbol_tex.UseTexture();
		Material_opacoBrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		arbol_central.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// bastones de caramelo al rededor del arbol

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, -60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, 60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(60 * toRadians) * 60.0f, 4.0f, cos(60 * toRadians) * 60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(120 * toRadians) * 60.0f, 4.0f, cos(120 * toRadians) * 60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(60 * toRadians) * 60.0f, 4.0f, -cos(60 * toRadians) * 60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(120 * toRadians) * 60.0f, 4.0f, -cos(120 * toRadians) * 60.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_tex.UseTexture();
		candy_cane.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// faros

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, -110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		Material_Semibrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		faro.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, 110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		faro.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(60 * toRadians) * 110.0f, -1.5f, cos(60 * toRadians) * 110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		faro.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(120 * toRadians) * 110.0f, -1.5f, cos(120 * toRadians) * 110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		faro.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(60 * toRadians) * 110.0f, -1.5f, -cos(60 * toRadians) * 110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		faro.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(120 * toRadians) * 110.0f, -1.5f, -cos(120 * toRadians) * 110.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		faro_tex.UseTexture();
		faro.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// arbustos centrales

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-60.0f, 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		Material_Semiopaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		bush.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(60.0f, 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		bush.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(30 * toRadians) * 60.0f, 2.0f, cos(30 * toRadians) * 60.0f));
		model = glm::rotate(model, -60 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		bush.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(150 * toRadians) * 60.0f, 2.0f, cos(150 * toRadians) * 60.0f));
		model = glm::rotate(model, -120 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		bush.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(30 * toRadians) * 60.0f, 2.0f, -cos(30 * toRadians) * 60.0f));
		model = glm::rotate(model, 120 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		bush.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(150 * toRadians) * 60.0f, 2.0f, -cos(150 * toRadians) * 60.0f));
		model = glm::rotate(model, 60 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		bush_tex.UseTexture();
		bush.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// edificios
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 220.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio1_tex.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		edificio1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, -220.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio2_tex.UseTexture();
		edificio2.RenderModel();

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(60 * toRadians) * 220.0f, -2.0f, cos(60 * toRadians) * 220.0f));
		model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio2_tex.UseTexture();
		edificio2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(60 * toRadians) * -220.0f, -2.0f, -cos(60 * toRadians) * -220.0f));
		model = glm::rotate(model, -10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio2_tex.UseTexture();
		edificio2.RenderModel();

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(60 * toRadians) * 220.0f, -2.0f, -cos(60 * toRadians) * 220.0f));
		model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio1_tex.UseTexture();
		edificio1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-sin(60 * toRadians) * -220.0f, -2.0f, cos(60 * toRadians) * -220.0f));
		model = glm::rotate(model, -10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(60.0f, 60.0f, 60.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		edificio1_tex.UseTexture();
		edificio1.RenderModel();

		/****************************************************************************************************/
/****************************************************************************************************/
//										personaje principal
/****************************************************************************************************/
/****************************************************************************************************/

		glm::mat4 padoruModel(1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(sin(padoruAngle * toRadians) * 90.0f, 3.7f + padoruJumpY, cos(padoruAngle * toRadians) * 90.0f));
		model = glm::rotate(model, -(90 - padoruAngle + padoruJumpAngle) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		padoruModel = model;
		//model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		Material_Semibrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		padoru_cuerpo.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(padoruModel, glm::vec3(0.0f, 6.5f, 0.0f));
		//model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		padoru_cabeza.RenderModel();

		// brazo derecho
		model = glm::mat4(1.0);
		model = glm::translate(padoruModel, glm::vec3(1.5f, 1.5f, 0.3f));
		model = glm::rotate(model, padoruExtrAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // rotacion animada
		matAux = model;

		model = glm::translate(matAux, glm::vec3(2.0f, -1.3f, -1.5f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		padoru_brazo_der.RenderModel();

		// brazo izquierdo
		model = glm::mat4(1.0);
		model = glm::translate(padoruModel, glm::vec3(-1.5f, 1.5f, 0.3f));
		model = glm::rotate(model, -padoruExtrAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // rotacion animada
		matAux = model;

		model = glm::mat4(1.0);
		model = glm::translate(matAux, glm::vec3(-2.0f, -1.3f, -1.5f));
		//model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		padoru_brazo_izq.RenderModel();

		// pie derecho
		model = glm::mat4(1.0);
		model = glm::translate(padoruModel, glm::vec3(1.2f, -1.5f, -0.24f));
		model = glm::rotate(model, -padoruExtrAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // rotacion animada
		matAux = model;

		model = glm::mat4(1.0);
		model = glm::translate(matAux, glm::vec3(0.0f, -1.2f, 0.0f));
		//model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		padoru_pierna_der.RenderModel();

		// pie izquierdo
		model = glm::mat4(1.0);
		model = glm::translate(padoruModel, glm::vec3(-1.2f, -1.5f, -0.24f));
		model = glm::rotate(model, padoruExtrAngle * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // rotacion animada
		matAux = model;

		model = glm::mat4(1.0);
		model = glm::translate(matAux, glm::vec3(0.0f, -1.2f, 0.0f));
		//model = glm::rotate(model, 10 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		padoru_tex.UseTexture();
		padoru_pierna_izq.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/
		//										Qiqi y Hu Tao
		/****************************************************************************************************/
		/****************************************************************************************************/

		QHCicleCenter = glm::mat4(1.0);
		QHCicleCenter = glm::translate(QHCicleCenter, glm::vec3(-170.0f, 6.0f, 0.0f)); //  matriz central sobre la que se jerarquizaran los modelos.

		// Qiqi

		glm::mat4 qiqiCent(1.0);

		model = glm::mat4(1.0f);
		model = glm::translate(QHCicleCenter, glm::vec3(sin(qiqiAngle * toRadians) * 35.0f, 3.7f, cos(qiqiAngle * toRadians) * 35.0f));
		model = glm::rotate(model, -(180 - qiqiAngle) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		qiqiCent = model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		qiqi_tex.UseTexture();
		Material_opacoBrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		qiqi_cuerpo.RenderModel();

		// muslo derecho
		matAux = glm::translate(qiqiCent, glm::vec3(0.89f, -4.15f, -0.33f));
		matAux = glm::rotate(matAux, qiqiDerMuslo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(-0.094f, -1.151f, -0.107f));
		matAux = model;
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		qiqi_tex.UseTexture();
		qiqi_muslo_der.RenderModel();

		// pie derecho
		matAux = glm::translate(matAux, glm::vec3(-0.258f, -1.54f, -0.057f));
		matAux = glm::rotate(matAux, qiqiDerPie * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(0.01f, -1.937f, -0.112f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		qiqi_tex.UseTexture();
		qiqi_patorrilla_der.RenderModel();

		// muslo izquierdo
		matAux = glm::translate(qiqiCent, glm::vec3(-0.89f, -4.15f, -0.33f));
		matAux = glm::rotate(matAux, qiqiIzqMuslo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(0.094f, -1.151f, -0.107f));
		matAux = model;
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		qiqi_tex.UseTexture();
		qiqi_muslo_izq.RenderModel();

		// pie izquierdo
		matAux = glm::translate(matAux, glm::vec3(0.258f, -1.54f, -0.057f));
		matAux = glm::rotate(matAux, qiqiIzqPie * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(-0.01f, -1.937f, -0.112f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		qiqi_tex.UseTexture();
		qiqi_patorrilla_izq.RenderModel();

		// Hu Tao

		glm::mat4 hutaoCent(1.0);

		model = glm::mat4(1.0f);
		model = glm::translate(QHCicleCenter, glm::vec3(sin((hutaoAngle - 20) * toRadians) * 35.0f, 3.7f, cos((hutaoAngle - 20) * toRadians) * 35.0f));
		model = glm::rotate(model, (hutaoAngle - 30) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		hutaoCent = model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hutao_tex.UseTexture();
		hutao_cuerpo.RenderModel();

		// muslo derecho
		matAux = glm::translate(hutaoCent, glm::vec3(-0.8f, -1.18f, -0.308f));
		matAux = glm::rotate(matAux, hutaoDerMuslo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(-0.076f, -1.868f, 0.351f));
		matAux = model;
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hutao_tex.UseTexture();
		hutao_muslo_der.RenderModel();

		// pie derecho
		matAux = glm::translate(matAux, glm::vec3(0.2f, -2.223f, 0.1));
		matAux = glm::rotate(matAux, -hutaoDerPie * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(0.02f, -2.982f, 0.189f));
		//model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hutao_tex.UseTexture();
		hutao_patorrilla_der.RenderModel();

		// muslo izquierdo
		matAux = glm::translate(hutaoCent, glm::vec3(0.8f, -1.18f, -0.308f));
		matAux = glm::rotate(matAux, hutaoIzqMuslo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(0.076f, -1.868f, 0.351f));
		matAux = model;
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hutao_tex.UseTexture();
		hutao_muslo_izq.RenderModel();

		// pie izquierdo
		matAux = glm::translate(matAux, glm::vec3(-0.2f, -2.223f, 0.1));
		matAux = glm::rotate(matAux, -hutaoIzqPie * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		model = matAux;
		model = glm::translate(model, glm::vec3(-0.02f, -2.982f, 0.189f));
		//model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hutao_tex.UseTexture();
		hutao_patorrilla_izq.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Estrella
		// animaci�n
		estrellaAngle += estrellaAngleOffset * deltaTime;
		if (estrellaAngle >= 360) { estrellaAngle = 0.0f; }
		if (estrellaSubida) {
			if (estrellaAltura <= 178.0f) {
				estrellaAltura += estrellaAlturaOffset * deltaTime;
			}
			else {
				if (estrellaAux <= 100.0f) {
					estrellaAux += estrellaAuxOffset * deltaTime;
				}
				else {
					estrellaSubida = false;
					estrellaAux = 0.0f;
				}
			}
			if (estrellaRadioAngle >= 0.0f) {
				estrellaRadioAngle -= estrellaRadioAngleOffset * deltaTime;
			}
		}
		else {
			if (estrellaAltura >= 28.0f) {
				estrellaAltura -= estrellaAlturaOffset * deltaTime;
			}
			else {
				if (estrellaAux <= 100.0f) {
					estrellaAux += estrellaAuxOffset * deltaTime;
				}
				else {
					estrellaSubida = true;
					estrellaAux = 0.0f;
				}
			}
			if (estrellaRadioAngle <= 90.0f) {
				estrellaRadioAngle += estrellaRadioAngleOffset * deltaTime;
			}
		}

		//renderizado
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::translate(model, glm::vec3(sin(estrellaAngle * toRadians) * 90.0f, cos(estrellaAngle * toRadians) * 90.0f, 28.0f));
		model = glm::translate(model, glm::vec3(sin(estrellaAngle * toRadians) * estrellaRadioAngle, cos(estrellaAngle * toRadians) * estrellaRadioAngle, estrellaAltura));
		model = glm::rotate(model, estrellaAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		estrella.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// knuckles
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(115.0f, -1.5f, 137.0f));
		model = glm::rotate(model, -55 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::rotate(model, -125 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(72.0f, -1.5f, 163.0f));
		model = glm::rotate(model, -235 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(95.0f, -1.5f, 175.0f));
		model = glm::rotate(model, -175 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(116.0f, -1.5f, 162.0f));
		model = glm::rotate(model, -115 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(93.0f, -1.5f, 125.0f));
		model = glm::rotate(model, 5 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(72.0f, -1.5f, 139.0f));
		model = glm::rotate(model, -295 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(87.0f, -1.5f, 97.0f));
		model = glm::rotate(model, 222 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(69.0f, -1.5f, 100.0f));
		model = glm::rotate(model, 132 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(91.0f, -1.5f, 80.0f));
		model = glm::rotate(model, 312 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		knuckles.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Trineo
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-80.0, -1.0f, 140.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 55 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		trineo.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Renos
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-75.0, -1.5f, 90.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 30 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(40.0f, 40.0f, 40.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		reno_1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-72.0, 3.5f, 87.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 30 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		reno_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-60.0, 3.5f, 100.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		reno_2.RenderModel();


		// Regalos
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-60.0, -1.5f, 145.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 170 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		regalo_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-89.0, -1.5f, 124.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -100 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		regalo_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-25.0, -1.5f, 125.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		regalo_3.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-90.0, -1.5f, 157.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		regalo_3.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-110.0, -1.5f, 137.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		regalo_3.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Iglu
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(75.0, -1.0f, -130.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 150 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(13.0f, 13.0f, 13.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iglu.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Oso polar
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(110.0, -1.0f, -140.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		oso_polar.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(30.0, -1.0f, -120.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 110 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		oso_polar.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Ice climbers
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(80.0, -1.0f, -78.0f));
		model = glm::rotate(model, 130 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ice_climbers.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Casa galleta
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-65.0, -1.0f, -120.0f));
		model = glm::rotate(model, -60 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		casa_galleta.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Homre de galleta
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-65.0, -1.0f, -90.0f));
		//model = glm::translate(model, glm::vec3(-50.0, -1.0f, -130.0f));
		model = glm::rotate(model, 65 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hombre_galleta.RenderModel();

		// Homre de galleta
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-63.0, -1.0f, -93.0f));
		model = glm::rotate(model, 15 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		hombre_galleta.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Arboles galleta
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-20.0, -1.0f, -110.0f));
		//model = glm::rotate(model, 15 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arbol_galleta.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-90.0, -1.0f, -87.0f));
		model = glm::rotate(model, 60 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		arbol_galleta.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Baston de caramelo
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-66.7,-1.0f, -87.3f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(5.5f, 5.5f, 5.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		candy_cane_2.RenderModel();

		/****************************************************************************************************/
		/****************************************************************************************************/

		// Iceberg
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(225.0f, 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(113.0f, -25.0f, -195.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-113.0f, 5.0f, -195.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_3.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-245.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_3.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-113.0f, -25.0f, 195.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(113.0f, 0.0f, 195.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_3.RenderModel();

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(50.0f, -10.0f, -141.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_3.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-170.0f, -25.0f, 0.0f));
		model = glm::rotate(model, -180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(39.0f, -25.0, 114.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iceberg_3.RenderModel();
		

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/

		/****************************************************************************************************/
		/****************************************************************************************************/

		//Pinguinos
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(120.0, -0.5f, 40.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(130.0, -0.5f, 25.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 135 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(110.0, -0.5f, 25.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 135 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_2.RenderModel();

		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
		
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(120.0, -0.5f, -40.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 *toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(130.0, -0.5f, -25.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(110.0, -0.5f, -25.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();
		
		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
		
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(180.0, -0.5f, -30.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(170.0, -0.5f, -20.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();
		
		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
		
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(170.0, -0.5f, 20.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 135 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_2.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(180.0, -0.5f, 30.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pinguino_1.RenderModel();
		
		/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
		
		//Grape-kun
		model = glm::mat4(1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Activar alpha
		model = glm::translate(model, glm::vec3(130.0, 4.0f, 0.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(18.0f, 18.0f, 18.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		grape_kun.RenderModel();
		glDisable(GL_BLEND);
		
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
