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
	layout(location = 0) in vec3 wP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = MVP * vec4(wP.x, wP.y, wP.z, 1);
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
	 * Kiszámítja a view transzformációs mátrixot a kamera tulajdonságaiból.
	 * @return mat4 View mátrix
	 */
	mat4 view() {
		// return mat4 {
		// 	vec4(1.0f, 0.0f, 0.0f, -wCenter.x),
		// 	vec4(0.0f, 1.0f, 0.0f, -wCenter.y),
		// 	vec4(0.0f, 0.0f, 1.0f, 0.0f),
		// 	vec4(0.0f, 0.0f, 0.0f, 1.0f)
		// };

		// return transpose(translate(vec3(-wCenter.x, -wCenter.y, 0.0f)));		
		return translate(vec3(-wCenter.x, -wCenter.y, 0.0f));
	}

	/**
	 * Kiszámítja a view transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 * @return mat4 View mátrix inverze
	 */
	mat4 invView() {
		// return mat4 {
		// 	vec4(1.0f, 0.0f, 0.0f, wCenter.x),
		// 	vec4(0.0f, 1.0f, 0.0f, wCenter.y),
		// 	vec4(0.0f, 0.0f, 1.0f, 0.0f),
		// 	vec4(0.0f, 0.0f, 0.0f, 1.0f)
		// };

		return translate(vec3(wCenter.x, wCenter.y, 0));
	}

	/**
	 * Kiszámítja a projection transzformációs mátrixot a kamera tulajdonságaiból.
	 * @return mat4 Projection mátrix
	 */
	mat4 projection() {
		// return mat4 {
		// 	vec4((2.0f / wWidth),	0.0f, 				0.0f, 0.0f),
		// 	vec4(0.0f,				(2.0f / wHeight),	0.0f, 0.0f),
		// 	vec4(0.0f, 				0.0f, 				1.0f, 0.0f),
		// 	vec4(0.0f, 				0.0f, 				0.0f, 1.0f)
		// };
	
		// return transpose(scale(vec3(2.0f / wWidth, 2.0f / wHeight, 1)));
		return scale(vec3(2.0f / wWidth, 2.0f / wHeight, 1.0f));
	}

	/**
	 * Kiszámítja a projection transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 * @return mat4 Projection mátrix inverze
	 */
	mat4 invProjection() {
		// return mat4 {
		// 	vec4((wWidth / 2.0f),	0.0f, 				0.0f, 0.0f),
		// 	vec4(0.0f,				(wHeight / 2.0f),	0.0f, 0.0f),
		// 	vec4(0.0f, 				0.0f, 				1.0f, 0.0f),
		// 	vec4(0.0f, 				0.0f, 				0.0f, 1.0f)
		// };

		return scale(vec3(wWidth / 2.0f, wHeight / 2.0f, 1.0f));
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
		glBindVertexArray(controlPointsVAO);
		glGenBuffers(1, &controlPointsVBO);
		
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

		printf("Control points and knot values:\n");
		for (int i = 0; i < wControlPoints.size(); ++i) {
			printf("\tWorld space: (%lf, %lf): %d\n", wControlPoints.at(i).x, wControlPoints.at(i).y, knotValues.at(i));
			// vec4 clipPoint = MVP * vec4(wControlPoints.at(i), 1.0f);
			// clipPoint /= clipPoint.w; // Perspective divide
			// printf("\tClip space: (%.3f, %.3f, %.3f, %.3f)\n", clipPoint.x, clipPoint.y, clipPoint.z, clipPoint.w);
		}
	}
	
	/**
	 * Megadja a t paraméterhez tartozó pont helyvektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 * @return vec3 t paraméterhez tartozó pont helyvektora világ koordinátákban.
	 */
	vec3 wR(float t) {

	}

	/**
	 * Szinkronizálja a GPU-n és CPU-n tárolt adatokat.
	 */
	void sync() {
		bindControlPoints();

		glBufferData(GL_ARRAY_BUFFER, wControlPoints.size() * sizeof(vec3), wControlPoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		printf("Synced control points.\n");
	}

	/**
	 * Kirajzolja a GPU-n tárolt állapotot.
	 * 
	 * @param gpuProgram Shader program, amin beállítja a szín uniformot.
	 */
	void draw(GPUProgram* gpuProgram, mat4 MVP) {
		// TODO: draw
		// gpuProgram->setUniform(vec3(0.0f, 1.0f, 1.0f), "color"); // yellow
		// bindSections();
		// glLineWidth(3);
		// glEnableVertexAttribArray(0);
		// glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
		
		bindControlPoints();
		
		gpuProgram->Use();
		gpuProgram->setUniform(vec3(1.0f, 0.0f, 0.0f), "color"); // red
		gpuProgram->setUniform(MVP, "MVP");
		
		// glEnableVertexAttribArray(0);
		// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr);
		glDrawArrays(GL_POINTS, 0, wControlPoints.size());

		printf("Drawn control points.\n");
	}

private:
	unsigned int controlPointsVAO;
	unsigned int controlPointsVBO;
	unsigned int sectionsVAO;
	unsigned int sectionVBO;
	unsigned int currentKnotValue;
	vector<vec3> wControlPoints;
	vector<float> knotValues;
	// mat4 MVP;

	/**
	 * Kiszámolja a t paraméterhez tartozó pont helyvektorát, ami a p0-p1 Hermite interpolációs görbére esik.
	 * 
	 * @param p0 Első kontroll pont.
	 * @param v0 Sebességvektor az első kontrollpontban.
	 * @param t0 t0 paraméter az első kontrollponthoz.
	 * @param p1 Második kontroll pont.
	 * @param v1 Sebességvektor az második kontrollpontban.
	 * @param t1 t1 paraméter az második kontrollponthoz.
	 * @param t  t paraméter, amihez tartozó pontot adja vissza
	 * @return vec3 A t paraméterhez tartozó pont helyvektora világ koordinátákban. 
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
	GPUProgram* gpuProgram;
	Camera* camera;
	Spline* spline;
	mat4 MVP;
	mat4 invMVP;
public:
	GreenTriangleApp() : glApp("Lab2") { }

	void onInitialization() {
		gpuProgram = new GPUProgram(vertSource, fragSource);

		camera = new Camera(vec3(10.0f, 10.0f, 1.0f), 20.0f, 20.0f);
		MVP = camera->projection() * camera->view();
		invMVP = camera->invView() * camera->invProjection();

		spline = new Spline();
	}

	void onDisplay() {
		glPointSize(10);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, winWidth, winHeight);

		spline->sync();
		spline->draw(gpuProgram, MVP);
	}

	void onMousePressed(MouseButton but, int pX, int pY) {
		// Screen space point
		vec4 pPoint((float) pX, (float) pY, 1.0f, 1.0f);

		// Clip space point
		vec4 cPoint(
			(float) pX / winWidth * 2.0f - 1.0f,
			1.0f - (float) pY / winHeight * 2.0f,
			1.0f,
			1.0f
		);

		// World space point
		vec4 wPoint = invMVP * vec4(cPoint.x, cPoint.y, 1.0f, 1.0f);
		spline->addControlPoint(wPoint);
		
		// Clip space point again
		vec4 cPointAgain = MVP * wPoint;
		
		printf("Clicked (in device coordinates): (%d, %d)\n", pX, pY);
		printf("Clicked (in clip coordinates): (%lf, %lf)\n", cPoint.x, cPoint.y);
		printf("Clicked (in world coordinates): (%lf, %lf)\n", wPoint.x, wPoint.y);
		printf("Transformed to clip space again: (%lf, %lf)\n", cPointAgain.x, cPointAgain.y);

		refreshScreen();
	}
	
};

GreenTriangleApp app;

