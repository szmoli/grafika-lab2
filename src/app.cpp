//=============================================================================================
// Z�ld h�romsz�g: A framework.h oszt�lyait felhaszn�l� megold�s
//=============================================================================================
#include "../inc/framework.h"
#include <math.h>

// using namespace::glm;
// using namespace::std;

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
		return translate(vec3(-wCenter.x, -wCenter.y, 0.0f));
	}

	/**
	 * Kiszámítja a view transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 * @return mat4 View mátrix inverze
	 */
	mat4 invView() {
		return translate(vec3(wCenter.x, wCenter.y, 0));
	}

	/**
	 * Kiszámítja a projection transzformációs mátrixot a kamera tulajdonságaiból.
	 * @return mat4 Projection mátrix
	 */
	mat4 projection() {
		return scale(vec3(2.0f / wWidth, 2.0f / wHeight, 1.0f));
	}

	/**
	 * Kiszámítja a projection transzformációs mátrix inverzét a kamera tulajdonságaiból.
	 * @return mat4 Projection mátrix inverze
	 */
	mat4 invProjection() {
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
		glGenVertexArrays(1, &curvePointsVAO);
		glBindVertexArray(curvePointsVAO);
		glGenBuffers(1, &curvePointsVBO);

		glGenVertexArrays(1, &controlPointsVAO);
		glBindVertexArray(controlPointsVAO);
		glGenBuffers(1, &controlPointsVBO);
		
		currentKnotValue = 0;
	}

	/**
	 * Visszaadja a kontroll pontok számát.
	 * 
	 * @return unsigned int Kontrollpontok száma.
	 */
	unsigned int controlPointsCount() {
		return wControlPoints.size();
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
		for (unsigned int i = 0; i < wControlPoints.size(); ++i) {
			printf("\tWorld space: (%lf, %lf): %lf\n", wControlPoints.at(i).x, wControlPoints.at(i).y, knotValues.at(i));
		}
	}
	
	/**
	 * Megadja a t paraméterhez tartozó pont helyvektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 * @return vec3 t paraméterhez tartozó pont helyvektora világ koordinátákban.
	 */
	vec3 wR(float t) {
		if (wControlPoints.size() < 2) {
			return vec3(NAN);
		}

		for (unsigned int i = 0; i < wControlPoints.size() - 1; ++i) {
			if (knotValues[i] <= t && t <= knotValues[i + 1]) {
				vec3 v0 = controlPointVelocity(i);
				vec3 v1 = controlPointVelocity(i + 1);

				return wHermite(
					wControlPoints.at(i),
					v0,
					knotValues.at(i),
					wControlPoints.at(i + 1),
					v1,
					knotValues.at(i + 1),
					t
				);
			}
		}

		return vec3(NAN);
	}

	/**
	 * Megadja a t paraméterhez tartozó pont normálvektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 * @return vec3 t paraméterhez tartozó pont normálvektora világ koordinátákban.
	 */
	vec3 wNormal(float t) {
		if (wControlPoints.size() < 2) {
			return vec3(NAN);
		}

		for (unsigned int i = 0; i < wControlPoints.size() - 1; ++i) {
			if (knotValues[i] <= t && t <= knotValues[i + 1]) {
				vec3 v0 = controlPointVelocity(i);
				vec3 v1 = controlPointVelocity(i + 1);

				return wHermiteNormal(
					wControlPoints.at(i),
					v0,
					knotValues.at(i),
					wControlPoints.at(i + 1),
					v1,
					knotValues.at(i + 1),
					t
				);
			}
		}

		return vec3(NAN);
	}

	/**
	 * Megadja a t paraméterhez tartozó pont sebesség vektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 * @return vec3 t paraméterhez tartozó pont sebesség vektora világ koordinátákban.
	 */
	vec3 wVelocity(float t) {
		if (wControlPoints.size() < 2) {
			return vec3(NAN);
		}

		for (unsigned int i = 0; i < wControlPoints.size() - 1; ++i) {
			if (knotValues[i] <= t && t <= knotValues[i + 1]) {
				vec3 v0 = controlPointVelocity(i);
				vec3 v1 = controlPointVelocity(i + 1);

				return wHermiteVelocity(
					wControlPoints.at(i),
					v0,
					knotValues.at(i),
					wControlPoints.at(i + 1),
					v1,
					knotValues.at(i + 1),
					t
				);
			}
		}

		return vec3(NAN);
	}

	/**
	 * Megadja a t paraméterhez tartozó pont gyorsulás vektorát világ koordinátákban.
	 * 
	 * @param t Szabad paramáter.
	 * @return vec3 t paraméterhez tartozó pont gyorsulás vektora világ koordinátákban.
	 */
	vec3 wAcceleration(float t) {
		if (wControlPoints.size() < 2) {
			return vec3(NAN);
		}

		for (unsigned int i = 0; i < wControlPoints.size() - 1; ++i) {
			if (knotValues[i] <= t && t <= knotValues[i + 1]) {
				vec3 v0 = controlPointVelocity(i);
				vec3 v1 = controlPointVelocity(i + 1);

				return wHermiteAcceleration(
					wControlPoints.at(i),
					v0,
					knotValues.at(i),
					wControlPoints.at(i + 1),
					v1,
					knotValues.at(i + 1),
					t
				);
			}
		}

		return vec3(NAN);
	}

	/**
	 * Szinkronizálja a GPU-n és CPU-n tárolt adatokat.
	 */
	void sync() {
		// Görbék kiszámítása
		if (wControlPoints.size() >= 2) {
			wCurvePoints.clear();
			int resolution = 100;

			float incrementation = knotValues.back() / resolution;
			// float incrementation = knotValues.back() / (knotValues.size() * resolution);
			// printf("incrementation: %lf\n", incrementation);

			for (float t = 0; t <= knotValues.back(); t += incrementation) {
				vec3 wCurvePoint = wR(t);
				// printf("curve point added: (%lf, %lf, %lf)\n", wCurvePoint.x, wCurvePoint.y, wCurvePoint.z);

				wCurvePoints.push_back(wCurvePoint);
			}
		}
		
		bindCurvePoints();
		glBufferData(GL_ARRAY_BUFFER, wCurvePoints.size() * sizeof(vec3), wCurvePoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// printf("Calculated and synced curve points.\n");

		bindControlPoints();
		glBufferData(GL_ARRAY_BUFFER, wControlPoints.size() * sizeof(vec3), wControlPoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// printf("Synced control points.\n");
	}

	/**
	 * Kirajzolja a GPU-n tárolt állapotot.
	 * 
	 * @param gpuProgram Shader program, amin beállítja a szín uniformot.
	 */
	void draw(GPUProgram* gpuProgram, mat4 MVP) {
		// Görbe
		bindCurvePoints();
		gpuProgram->Use();
		gpuProgram->setUniform(vec3(1.0f, 1.0f, 0.0f), "color"); // yellow
		gpuProgram->setUniform(MVP, "MVP");
		glDrawArrays(GL_LINE_STRIP, 0, wCurvePoints.size());
		// printf("Drawn curve.\n");
		
		// Kontroll pontok
		bindControlPoints();		
		gpuProgram->Use();
		gpuProgram->setUniform(vec3(1.0f, 0.0f, 0.0f), "color"); // red
		gpuProgram->setUniform(MVP, "MVP");		
		glDrawArrays(GL_POINTS, 0, wControlPoints.size());
		// printf("Drawn control points.\n");
	}

private:
	unsigned int controlPointsVAO;
	unsigned int controlPointsVBO;
	unsigned int curvePointsVAO;
	unsigned int curvePointsVBO;
	unsigned int currentKnotValue;
	std::vector<vec3> wControlPoints;
	std::vector<vec3> wCurvePoints;
	std::vector<float> knotValues;

	/**
	 * Kiszámolja a sebesség vektort a sorszámmal megadott kontroll ponthoz.
	 * 
	 * @param i Kontroll pont sorszáma
	 * @return vec3 Kontrollpont sebesség vektora
	 */
	vec3 controlPointVelocity(unsigned int i) {
		// Első vagy utolsó pont fixen zérus sebesség vektorral.
		if (i == 0 || i == wControlPoints.size() - 1) {
			return vec3(0.f, 0.f, 0.f);
		}

		return 0.5f * (((wControlPoints.at(i + 1) - wControlPoints.at(i)) / (knotValues.at(i + 1) - knotValues.at(i))) + ((wControlPoints.at(i) - wControlPoints.at(i - 1)) / (knotValues.at(i) - knotValues.at(i - 1))));
	}

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
	vec3 wHermite(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t) {
		float tDiff = t1 - t0;
		vec3 a0 =	p0;
		vec3 a1 =	v0;
		vec3 a2 =	(3.f * (p1 - p0) / (tDiff * tDiff)) - ((v1 + 2.f * v0) / (t1 - t0));
		vec3 a3 =	(2.f * (p0 - p1) / (tDiff * tDiff * tDiff)) + ((v1 + v0) / (tDiff * tDiff));

		float dt = t - t0;

		return a3 * (dt * dt * dt) + a2 * (dt * dt) + a1 * dt + a0;
	}

	/**
	 * Kiszámolja a Hermite interpolációs görbe (érintő) sebesség vektorát.
	 * 
	 * @param p0 Első kontroll pont.
	 * @param v0 Sebességvektor az első kontrollpontban.
	 * @param t0 t0 paraméter az első kontrollponthoz.
	 * @param p1 Második kontroll pont.
	 * @param v1 Sebességvektor az második kontrollpontban.
	 * @param t1 t1 paraméter az második kontrollponthoz.
	 * @param t  t paraméter, amihez tartozó pontot adja vissza
	 * @return vec3 A t paraméterhez tartozó pont érintő vektor világ koordinátákban. 
	 */
	vec3 wHermiteVelocity(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t) {
		float tDiff = t1 - t0;
		vec3 a1 =	v0;
		vec3 a2 =	(3.f * (p1 - p0) / (tDiff * tDiff)) - ((v1 + 2.f * v0) / (t1 - t0));
		vec3 a3 =	(2.f * (p0 - p1) / (tDiff * tDiff * tDiff)) + ((v1 + v0) / (tDiff * tDiff));

		float dt = t - t0;

		return 3.f * a3 * (dt * dt) + 2.f * a2 * dt + a1;
	}

	/**
	 * Kiszámolja a Hermite interpolációs görbe normál vektorát.
	 * 
	 * @param p0 Első kontroll pont.
	 * @param v0 Sebességvektor az első kontrollpontban.
	 * @param t0 t0 paraméter az első kontrollponthoz.
	 * @param p1 Második kontroll pont.
	 * @param v1 Sebességvektor az második kontrollpontban.
	 * @param t1 t1 paraméter az második kontrollponthoz.
	 * @param t  t paraméter, amihez tartozó pontot adja vissza
	 * @return vec3 A t paraméterhez tartozó normál vektor világ koordinátákban. 
	 */
	vec3 wHermiteNormal(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t) {
		vec3 tangent = wHermiteVelocity(p0, v0, t0, p1, v1, t1, t);
		return normalize(vec3(-tangent.y, tangent.x, tangent.z));
	}

	/**
	 * Kiszámolja a Hermite interpolációs görbe gyorsulási vektorát.
	 * 
	 * @param p0 Első kontroll pont.
	 * @param v0 Sebességvektor az első kontrollpontban.
	 * @param t0 t0 paraméter az első kontrollponthoz.
	 * @param p1 Második kontroll pont.
	 * @param v1 Sebességvektor az második kontrollpontban.
	 * @param t1 t1 paraméter az második kontrollponthoz.
	 * @param t  t paraméter, amihez tartozó pontot adja vissza
	 * @return vec3 A t paraméterhez tartozó normál vektor világ koordinátákban. 
	 */
	vec3 wHermiteAcceleration(vec3 p0, vec3 v0, float t0, vec3 p1, vec3 v1, float t1, float t) {
		float tDiff = t1 - t0;
		vec3 a2 =	(3.f * (p1 - p0) / (tDiff * tDiff)) - ((v1 + 2.f * v0) / (t1 - t0));
		vec3 a3 =	(2.f * (p0 - p1) / (tDiff * tDiff * tDiff)) + ((v1 + v0) / (tDiff * tDiff));
		
		float dt = t - t0;
		
		return 6.f * a3 * dt + 2.f * a2;
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
	void bindCurvePoints() {
		glBindVertexArray(curvePointsVAO);
		glBindBuffer(GL_ARRAY_BUFFER, curvePointsVBO);
	}
};

enum class WheelState {
	INIT, IDLE, MOVING, FALLING
};

/**
 * Kerék osztály.
 */
class Wheel {
public:
	/**
	 * Kerék konstruktor
	 * @param wCenter Kezdő pozíció világ koordinátákban.
	 * @param wRadius Kör sugara világ koordinátákban.
	 */
	Wheel(Spline *spline) {
		// fizika
		wCenter = vec3(NAN);
		wRadius = 1.0;
		radAlpha = 0.f;
		radOmega = 0.f;
		state = WheelState::INIT;
		this->spline = spline;
		tau = 0.001f;

		// grafika
		glGenVertexArrays(1, &fillVAO);
		glBindVertexArray(fillVAO);
		glGenBuffers(1, &fillVBO);

		glGenVertexArrays(1, &outlinesVAO);
		glBindVertexArray(outlinesVAO);
		glGenBuffers(1, &outlinesVBO);
		
		glGenVertexArrays(1, &spokesVAO);
		glBindVertexArray(spokesVAO);		
		glGenBuffers(1, &spokesVBO);

		// Körvonal pontjainak kiszámítása
		int resolution = 15;
		float multiplier = 360 / resolution;
		for (int phi = 0; phi < resolution; ++phi) {
			float actualPhi = (float)phi * multiplier;
			// printf("phi: %d\nactualPhi: %lf\n", phi, actualPhi);
			vec3 mPoint = vec3(wRadius * cos(inRadians(actualPhi)), wRadius * sin(inRadians(actualPhi)), 1.f);
			// printf("mPoint: (%lf, %lf, %lf)\n", mPoint.x, mPoint.y, mPoint.z);
			mCirclePoints.push_back(mPoint);	// fillhez
			mOutlinePoints.push_back(mPoint);	// körvonalhoz
		}

		// küllők
		mSpokePoints.push_back(vec3(0.f, 1.f, 1.f));
		mSpokePoints.push_back(vec3(0.f, -1.f, 1.f));
		mSpokePoints.push_back(vec3(1.f, 0.f, 1.f));
		mSpokePoints.push_back(vec3(-1.f, 0.f, 1.f));
	}

	/**
	 * A kereket a kezdő pozícióba helyezi, amit a spline alapján számít ki.
	 */
	void reset() {
		tau = 0.001f;
		radAlpha = 0.f;
		radOmega = 0.f;
		vec3 wSplineR = spline->wR(tau);
		vec3 wSplineNormal = spline->wNormal(tau);
		wCenter = wSplineR + wSplineNormal * wRadius;
		state = WheelState::IDLE;
	}

	/**
	 * Elindítja a kerék mozgását.
	 */
	void start() {
		if (state != WheelState::IDLE) {
			return;
		}

		// printf("Wheel started moving.\n");
		state = WheelState::MOVING;
	}

	/**
	 * Mozgatja a kereket. Itt van a fizikai szimuláció implementálva.
	 * @param dt Idő paraméter
	 */
	void move(float dt) {
		if (state != WheelState::MOVING && state != WheelState::FALLING) {
			return;
		}

		// printf("dt: %lf\n", dt);

		// állandók és pálya paraméterek
		float m = 1.f; 						// kerék tömege
		vec3 wG = vec3(0.f, 40.f, 0.f);			// gravitációs gyorsulás
		vec3 wA_s = spline->wAcceleration(tau);	// spline gyorsulás vektor
		vec3 wN_s = spline->wNormal(tau);		// spline normál vektor
		vec3 wV_s = spline->wVelocity(tau);		// spline sebesség vektor
		vec3 wR_s = spline->wR(tau);			// spline és kerék érintkezési pontja

		// printf("wCenter before: (%lf, %lf, %lf)\n", wCenter.x, wCenter.y, wCenter.z);
		// printf("wR_s: (%lf, %lf, %lf)\n", wR_s.x, wR_s.y, wR_s.z);
		// printf("wV_s: (%lf, %lf, %lf)\n", wV_s.x, wV_s.y, wV_s.z);
		
		wCenter = wR_s + wN_s * wRadius;		// kerék pozíció frissítése
		// printf("wCenter after: (%lf, %lf, %lf)\n", wCenter.x, wCenter.y, wCenter.z);
		
		// kényszer erő kiszámítása
		float wV_sLen = length(wV_s);
		float wNormalGravity = dot(wG, wN_s); 	// gravitációs gyorsulás normálvektor irányú komponense
		float wKappa = dot(wA_s, wN_s) / (wV_sLen * wV_sLen);							// görbület
		float wVelocity = sqrtf((2 * length(wG) * (spline->wR(0.f).y - wR_s.y)) / 2);
		vec3 wK = m * (wNormalGravity * wN_s + (wVelocity * wVelocity) * wKappa);
		float wNormalForce = dot(wK, wN_s); // erő normálvektor irányú komponense

		if (wNormalForce <= 0.0f) {
			state = WheelState::FALLING;
			reset();
		}

		// forgó mozgás
		float wInertia = m * (wRadius * wRadius); 					// tehetetlenségi nyomaték (PHI)
		vec3 wLeverArm = wCenter - wR_s; 							// erőkar
		float wTorque = wLeverArm.x * wK.y - wLeverArm.y * wK.x; 	// forgatónyomaték
		float radBeta = wTorque / wInertia; 						// szöggyorsulás
		radOmega = radOmega + radBeta * dt; 						// szögsebesség frissítése
		radAlpha = radAlpha + (radOmega * dt) + (0.5f * radBeta * (dt * dt)); // elfordulási szög frissítése
		
		// görbe paraméter
		float dTau = wVelocity * dt / length(wV_s);
		tau += dTau; // tau frissítése

		// if (tau >= spline->controlPointsCount() - 1) {
		// 	reset();
		// }
		// printf("dTau: %lf\n", dTau);
		// printf("wCenter: (%lf, %lf, %lf)\n", wCenter.x, wCenter.y, wCenter.z);
	}

	/**
	 * Elforgatja a kereket a tárolt aktuális elfordulásával.
	 */
	mat4 model() {
		return translate(vec3(wCenter.x, wCenter.y, 0.f)) * rotate(radAlpha, vec3(0.f, 0.f, 1.f));
	}

	/**
	 * Szinkronizálja a kerék pontjait a GPU-ra.
	 */
	void sync() {
		bindFill();
		glBufferData(GL_ARRAY_BUFFER, mCirclePoints.size() * sizeof(vec3), mCirclePoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// printf("Synced circle fill points.\n");

		bindOutlines();
		glBufferData(GL_ARRAY_BUFFER, mOutlinePoints.size() * sizeof(vec3), mOutlinePoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// printf("Synced circle outline points.\n");

		bindSpokes();
		glBufferData(GL_ARRAY_BUFFER, mSpokePoints.size() * sizeof(vec3), mSpokePoints.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		// printf("Synced circle spoke points.\n");
	}

	/**
	 * Visszaadja a kerék állapotát.
	 * @return WheelState kerékállapot
	 */
	WheelState getState() {
		return state;
	}

	/**
	 * Megrajzolja a kereket.
	 */
	void draw(GPUProgram* gpuProgram, mat4 MVP) {		
		MVP = MVP * model();
		gpuProgram->Use();
		gpuProgram->setUniform(MVP, "MVP");

		gpuProgram->setUniform(vec3(0.0f, 0.0f, 1.0f), "color"); // blue
		bindFill();
		glDrawArrays(GL_TRIANGLE_FAN, 0, mCirclePoints.size());
		// printf("Drawn wheel fill.\n");
		
		gpuProgram->setUniform(vec3(1.0f, 1.0f, 1.0f), "color"); // white
		bindOutlines();
		glDrawArrays(GL_LINE_LOOP, 0, mOutlinePoints.size());
		// printf("Drawn wheel outline.\n");

		bindSpokes();
		glDrawArrays(GL_LINES, 0, mSpokePoints.size());
		// printf("Drawn wheel spokes.\n");
	}

private:
	// Fizikai jellemzők
	vec3 wCenter;		// pozíció
	float wRadius;		// sugár
	float radAlpha;		// elfordulási szög
	float radOmega;		// szögsebesség
	WheelState state;	// állapot
	Spline* spline;		// pálya referencia
	float tau;			// görbe paraméter

	// OpenGL cuccok
	// Kerék körvonal
	unsigned int outlinesVAO;
	unsigned int outlinesVBO;	
	std::vector<vec3> mOutlinePoints;
	// Kerék küllők
	unsigned int spokesVAO;
	unsigned int spokesVBO;		
	std::vector<vec3> mSpokePoints;
	// Kerék kitöltés
	unsigned int fillVAO;
	unsigned int fillVBO;		
	std::vector<vec3> mCirclePoints;

	/**
	 * Bindolja a körvonal és a küllők VAO és VBO-ját.
	 */
	void bindOutlines() {
		glBindVertexArray(outlinesVAO);
		glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
	}

	/**
	 * Bindolja a kék kitöltés VAO és VBO-ját.
	 */
	void bindFill() {
		glBindVertexArray(fillVAO);
		glBindBuffer(GL_ARRAY_BUFFER, fillVBO);
	}

	void bindSpokes() {
		glBindVertexArray(spokesVAO);
		glBindBuffer(GL_ARRAY_BUFFER, spokesVBO);
	}

	float inRadians(float degrees) {
		return degrees * (M_PI / 180.f);
	}
};

const int winWidth = 600, winHeight = 600;

class GreenTriangleApp : public glApp {
	GPUProgram* gpuProgram;
	Camera* camera;
	Spline* spline;
	Wheel* wheel;
	mat4 MVP;
	mat4 invMVP;
	float time;
public:
	GreenTriangleApp() : glApp("Lab2") { }

	void onInitialization() override {
		gpuProgram = new GPUProgram(vertSource, fragSource);		
		spline = new Spline();
		wheel = new Wheel(spline);
		camera = new Camera(vec3(10.0f, 10.0f, 1.0f), 20.0f, 20.0f);

		time = 0.0f;
		MVP = camera->projection() * camera->view();
		invMVP = camera->invView() * camera->invProjection();

		glLineWidth(3);
		glPointSize(10);
	}

	void onDisplay() override {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, winWidth, winHeight);

		wheel->sync();
		wheel->draw(gpuProgram, MVP);
		spline->sync();
		spline->draw(gpuProgram, MVP);
	}

	void onMousePressed(MouseButton but, int pX, int pY) override {
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
		spline->addControlPoint(vec3(wPoint.x, wPoint.y, wPoint.z));
		
		if (spline->controlPointsCount() == 2) {
			wheel->reset();
		}

		refreshScreen();
	}

	void onKeyboard(int key) override {
		switch (key) {
			case 32:
				if (wheel->getState() != WheelState::IDLE) {
					return;
				}
				wheel->start();
				break;

			default:
				break;
		}
	}
	
	void onTimeElapsed(float startTime, float endTime) override {
		if (wheel->getState() != WheelState::MOVING && wheel->getState() != WheelState::FALLING) {
			return;
		}

		// printf("start: %lf, end: %lf\n", startTime, endTime);

		float dt = 0.01f;
		for (float t = startTime; t < endTime; t += dt) {
			float Dt = fmin(dt, endTime - t);
			wheel->move(dt);
		}
		
		refreshScreen();
	}
};

GreenTriangleApp app;

