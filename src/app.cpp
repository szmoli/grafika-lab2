//=============================================================================================
// Z�ld h�romsz�g: A framework.h oszt�lyait felhaszn�l� megold�s
//=============================================================================================
#include "../inc/framework.h"

using namespace::glm;
using namespace::std;

// cs�cspont �rnyal�
const char * vertSource = R"(
	#version 330				
    precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec3 cP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = MVP * vec4(cP.x, cP.y, cP.z, 1); 	// bemenet m�r normaliz�lt eszk�zkoordin�t�kban
	}
)";

// pixel �rnyal�
const char * fragSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans sz�n
	out vec4 fragmentColor;		// pixel sz�n

	void main() {
		fragmentColor = vec4(color, 1); // RGB -> RGBA
	}
)";

class Camera {
public:
	/**
	 * Kamera konstruktor.
	 * 
	 * @param wCenter A kamera középpontja világ koordináta szerint.
	 * @param wWidth A kamera szélessége világ koordináta szerint.
	 * @param wHeight A kamera magassága világ koordináta szerint.
	 */
	Camera(const vec3& wCenter, float wWidth, float wHeight) {
		this->wCenter = wCenter;
		this->wWidth = wWidth;
		this->wHeight = wHeight;
	}

	/**
	 * Kiszámítja a model transzformációs mátrixot a kamera tulajdonságaiból.
	 */
	mat4 model() {

	}

	/**
	 * Kiszámítja a model transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 */
	mat4 invModel() {

	}
	
	/**
	 * Kiszámítja a view transzformációs mátrixot a kamera tulajdonságaiból.
	 */
	mat4 view() {
		return mat4 {
			vec4(1.0f, 0.0f, 0.0f, -wCenter.x),
			vec4(0.0f, 1.0f, 0.0f, -wCenter.y),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		};
	}

	/**
	 * Kiszámítja a view transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 */
	mat4 invView() {
		return mat4 {
			vec4(1.0f, 0.0f, 0.0f, wCenter.x),
			vec4(0.0f, 1.0f, 0.0f, wCenter.y),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		};
	}

	/**
	 * Kiszámítja a projection transzformációs mátrixot a kamera tulajdonságaiból.
	 */
	mat4 projection() {
		return mat4 {
			vec4((2.0f / wWidth),	0.0f, 				0.0f, 0.0f),
			vec4(0.0f,				(2.0f / wHeight),	0.0f, 0.0f),
			vec4(0.0f, 				0.0f, 				1.0f, 0.0f),
			vec4(0.0f, 				0.0f, 				0.0f, 1.0f)
		};
	}

	/**
	 * Kiszámítja a projection transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 */
	mat4 invProjection() {
		return mat4 {
			vec4((wWidth / 2.0f),	0.0f, 				0.0f, 0.0f),
			vec4(0.0f,				(wHeight / 2.0f),	0.0f, 0.0f),
			vec4(0.0f, 				0.0f, 				1.0f, 0.0f),
			vec4(0.0f, 				0.0f, 				0.0f, 1.0f)
		};
	}

private:	
	vec3 wCenter;
	float wWidth;
	float wHeight;
};

/**
 * Uniform paraméterezésű Catmull-Rom spline osztály.
 */
class Spline {
public:
	/**
	 * Spline konstruktor. Legenerál a pontokhoz és a vektorizált görbe szakaszokhoz is 1-1 VAO-t és VBO-t. Beállítja a kezdő csomópont értéket.
	 */
	Spline() {
		glGenVertexArrays(1, &controlPointsVAO);
		glGenBuffers(1, &controlPointsVBO);
		glGenVertexArrays(1, &sectionsVAO);
		glGenBuffers(1, &sectionVBO);
		currentKnotValue = 0;
	}

	/**
	 * Hozzáad egy új kontrol pontot világ koordináták szerint uniform paraméterezés szerint automatikusan számított csomópont értékkel.
	 * 
	 * @param wP Pont világ koordinátákkal.
	 */
	void addControlPoint(vec3 wP) {
		wControlPoints.push_back(wP);
		knotValues.push_back(currentKnotValue++); // Uniform paraméterezés szerint automatikusan növeli 1-el 0-tól kezdve a csomópontértékeket.
	}
	
	/**
	 * Megadja a t paraméterhez tartozó pont helyvektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 */
	vec3 wR(float t) {

	}

	/**
	 * Szinkronizálja a GPU-n és CPU-n tárolt adatokat.
	 */
	void sync() {
		bindControlPoints();
		glBufferData(GL_ARRAY_BUFFER, wControlPoints.size() * sizeof(vec3), wControlPoints.data(), GL_STATIC_DRAW);
	}

	/**
	 * Kirajzolja a GPU-n tárolt állapotot.
	 */
	void draw(GPUProgram* gpuProgram) {
		gpuProgram->setUniform(vec3(0.0f, 1.0f, 1.0f), "color"); // yellow
		bindSections();
		glLineWidth(3);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
		// TODO: draw
		
		gpuProgram->setUniform(vec3(1.0f, 0.0f, 0.0f), "color"); // red
		bindControlPoints();
		glPointSize(10);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
		glDrawArrays(GL_POINTS, 0, wControlPoints.size());
	}

private:
	unsigned int controlPointsVAO;
	unsigned int controlPointsVBO;
	unsigned int sectionsVAO;
	unsigned int sectionVBO;
	unsigned int currentKnotValue;
	vector<vec3> wControlPoints;
	vector<float> knotValues;

	/**
	 * Kiszámolja a t paraméterhez tartozó pont helyvektorát, ami a p0-p1 Hermite interpolációs görbére esik.
	 * 
	 * @param p0 Első kontroll pont.
	 * @param v0 Sebességvektor az első kontrollpontban.
	 * @param t0 T paraméter az első kontrollponthoz.
	 * @param p1 Második kontroll pont.
	 * @param v1 Sebességvektor az második kontrollpontban.
	 * @param t1 T paraméter az második kontrollponthoz.
	 */
	vec3 wHermite(vec3 p0, vec3 v0, vec3 t0, vec3 p1, vec3 v1, vec3 t1, float t) {
		// r(t)		= a_3 * (t - t_i)^3 + a_2 * (t - t_i)^2 + a_1 * (t - t_i) + a_0
		// r'(t) 	= 3 * a_3 * (t - t_i)^2 + 2 * a_2 * (t - t_i) + a_1
		// a_0 = p_i
		// a_1 = v_i
		// a_2 = 3 * (p_{i+1} - p_i) / (t_{i+1} - t_i)^2 - (v_{i+1} + 2 * v_i) / (t_{i+1} - t_i)
		// a_3 = 2 * (p_i - p_{i+1}) / (t_{i+1} - t_i)^3 - (v_{i+1} + v_i) / (t_{i+1} - t_i)^2
	}

	/**
	 * Bindolja a kontrollpontok VAO és VBO-ját.
	 */
	void bindControlPoints() {
		glBindVertexArray(controlPointsVAO);
		glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
	}

	/**
	 * Bindolja a vektorizált görbék VAO és VBO-ját.
	 */
	void bindSections() {
		glBindVertexArray(sectionsVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sectionVBO);
	}
};

const int winWidth = 600, winHeight = 600;

class GreenTriangleApp : public glApp {
	Geometry<vec3>* triangle;  // geometria
	GPUProgram* gpuProgram;	   // cs�cspont �s pixel �rnyal�k
public:
	GreenTriangleApp() : glApp("Lab2") { }

	// Inicializ�ci�, 
	void onInitialization() {
		triangle = new Geometry<vec3>;
		triangle->Vtx() = { vec3(-0.8f, -0.8f, 1.0f), vec3(-0.6f, 1.0f, 1.0f), vec3(0.8f, -0.2f, 1.0f) };
		triangle->updateGPU();
		gpuProgram = new GPUProgram(vertSource, fragSource);
	}

	// Ablak �jrarajzol�s
	void onDisplay() {
		glClearColor(0, 0, 0, 0);     // h�tt�r sz�n
		glClear(GL_COLOR_BUFFER_BIT); // rasztert�r t�rl�s
		glViewport(0, 0, winWidth, winHeight);
		triangle->Draw(gpuProgram, GL_TRIANGLES, vec3(0.0f, 1.0f, 0.0f));
	}
};

GreenTriangleApp app;

