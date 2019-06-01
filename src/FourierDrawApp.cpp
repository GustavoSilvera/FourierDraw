#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include <vector>
#include <string.h>
#include <algorithm>    // std::random_shuffle
#include <cstdlib>      // std::rand, std::srand
#include <iostream>     // std::cout
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <array>
#include <iomanip>      // std::setprecision
#include "cinder/gl/TextureFont.h"
#include <fstream>
#include <filesystem>
using namespace ci;
using namespace ci::app;
using namespace std;
int myrandom(int i) { return std::rand() % i; }

float ppm = 100;//scale of how many pixels are in 1 SI Meter
Font mFont;//custom font for optimized drawing
gl::TextureFontRef mTextureFont;//custom opengl::texture
std::ofstream scriptFile;
std::ofstream imageFile;
//typedef std::pair<float, float> complex;
#define c complex
#define NUM_INTERPOLATE 5
static float sqr(float x) { return x * x; }
static inline float toDeg(float rad) { return (rad * 180 / M_PI); }
static inline float toRad(float deg) { return (deg * M_PI / 180); }
static inline float clamp(float min, float max, float i) {
	if (i > min) {
		if (i < max) {
			return i;
		}
		return max;
	}
	return min;
}
std::string completeTxt;
template<typename T>
static inline T sum(const std::vector<T> &v) {
	T fin_sum = 0;
	for (int i = 0; i < v.size(); i++) {
		fin_sum += v[i];
	}
	return fin_sum;
}
class vec3 {
public:
	vec3(float x = 0.0, float y = 0.0, float z = 0.0) : X(x), Y(y), Z(z) {}
	float X, Y, Z;
	vec3 times(float f) {
		return vec3(X*f, Y*f, Z*f);
	}
	float distance(vec3 v) {
		return sqrt(sqr(v.X - X) + sqr(v.Y - Y) + sqr(v.Z - Z));
	}
	vec3 operator+(vec3 v) {
		return vec3(X + v.X, Y + v.Y, Z + v.Z);
	}
};
class arrow {
public:
	arrow(float L, float V, float T, vec3 p) : length(L), velocity(V), theta(T), pos(p) {}
	arrow(float L, float V, float T) : length(L), velocity(V), theta(T) {}
	arrow() : length(1), velocity(0), theta(M_PI/2) {}
	float length;//meters
	float velocity;//radians/sec
	float theta;//radians
	vec3 pos;//(x meters, y meters, 0)
	vec3 tip() {//gets the tip end of the arrow in rectangular coords
		//converts angular(polar) to rectangular
		float x = length * cos(theta);// x = r*cos(thta)
		float y = length * sin(theta);// y = r*sin(thta) [negative bc window is in 4th quadrant]
		return (pos + vec3(x, y));//adds arrowhead to initial 2D rectangular position
	}
};
class complex {
public:
	complex() = default;
	complex(double r, double i) : re(r), im(-i) {}
	float re, im;//real and imaginary components
	complex mult(complex a) {//MAKE THIS INTO CLASS FUNCTION
		float re_new = a.re*re - a.im*im;//i^2 = -1
		float im_new = a.re*im + a.im*re;
		return(complex{ re_new, im_new });
	}
	complex operator* (float scalar) {//MAKE THIS INTO CLASS FUNCTION
		return(complex{ scalar*re, scalar*im });
	}
	void add(complex a) {//not good ood but owell...
		re += a.re;
		im -= a.im;
	}
	float magnitude() {
		return sqrt(sqr(re) + sqr(im));
	}
	float angle() {
		return atan2(im, re);
	}
};
struct f_elements {
	float real, imag, freq, phase, amplitude;
};
typedef std::vector<complex> compVect;
typedef std::vector<f_elements> fourierSeries;
fourierSeries discreteFourierTransform(compVect x) {
	fourierSeries X;
	const float N = x.size();//accuracy??
	for (int k = 0; k < N; k++) {
		complex sum{ 0, 0 };//complex number final
		for (int n = 0; n < N; n++) {
			float phi = (2 * M_PI*k*n) / N;//constant for angular velocity
			const complex c{ cos(phi), -sin(phi) };//cos and sin component
			sum.add(x[n].mult(c));//multiply cos & sin comp by original complex
		}
		sum.re = sum.re / N;
		sum.im = sum.im / N;
		float freq = -k;
		float phase = sum.angle();
		float amplitude = sum.magnitude();
		X.push_back(f_elements{ sum.re, sum.im, freq, phase, amplitude });
	}
	return X;
}
compVect interpolateComplex(compVect v, const int n_interpolate){
	const int initSize = v.size();
	std::vector<complex> newVec;
	for (int i = 0; i < initSize; i++) {
		newVec.push_back(v[i]);
		float deltaX, deltaY;
		if (i < initSize - 1) {
			deltaX = (v[i + 1].re - v[i].re) / (n_interpolate + 1);
			deltaY = (v[i + 1].im - v[i].im) / (n_interpolate + 1);
		}
		else {
			deltaX = (v[0].re - v[i].re) / (n_interpolate + 1);
			deltaY = (v[0].im - v[i].im) / (n_interpolate + 1);
		}
		for (int j = 1; j < n_interpolate + 1; j++) {
			newVec.push_back(complex{ v[i].re + deltaX * j, -(v[i].im + deltaY * j) });
		}
	}
	return newVec;
}
fourierSeries sort(fourierSeries f) {//simple bubble sort of amplitudes
	int size = f.size();
	f_elements temp;
	for (int i = 0; i < size; i++){//fwds
		for (int j = size - 1; j > i; j--){//bkwds
			if (f[j].amplitude > f[j - 1].amplitude){
				temp = f[j - 1];
				f[j - 1] = f[j];
				f[j] = temp;
			}
		}
	}
	return(f);
}
compVect scale(compVect v, const float scalar){
	compVect scaled;
	for (int i = 0; i < v.size(); i++) {
		scaled.push_back (v[i] * (scalar));
	}
	return scaled;
}
class drawing {
public:
	std::vector<arrow> train;
	std::vector<std::pair<vec3, vec3>> path;
	bool penDown = true;
	int last;//index of final arrow
	vec3 initP;
	float dt = 60;
	bool hide = false;
	bool pointDraw = true;
	std::vector<complex> drawPoints;
	void addArrow(arrow a) {
		train.push_back(a);
		int i = train.size() - 1;//index for newest arrow
		scriptFile << std::to_string(i + 1) << "  len: " << std::setprecision(3) << train[i].length << "   vel: " << std::setprecision(3) << train[i].velocity << endl;
		scriptFile = std::ofstream("script.txt", ofstream::app);
	}
	void init() {//normie init
		scriptFile = std::ofstream("script.txt");
		addArrow(arrow{ 1, 1, 0, initP });
	}
	void fileInit() {
		#define MAX_LINE 20
		std::ifstream file("script.txt");
		while (!file.eof()) {
			char line[MAX_LINE];
			file.getline(line, MAX_LINE);
			char radius[MAX_LINE];
			char vel[MAX_LINE];
			char angle[MAX_LINE];
			sscanf(line, "%s %s %s", radius, vel, angle);
			addArrow(arrow{ (float)atof(radius), (float)atof(vel), (float)atof(angle) , initP});
		}
	}
	std::vector<complex> coords;
	
	void fourierInit() {
		std::ofstream ofs;
		ofs.open("imagTXT.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();		
		scriptFile.clear();
		std::vector<complex> v;
		if (!drawPoints.empty()){
			v = drawPoints;
		}
		else v = {//basic easy silhouette
			//c{ -0.1, 0.39 }, c{ 0, 0.36 }, c{ 0.09, 0.32 }, c{ 0.16, 0.3 }, c{ 0.23, 0.28 }, c{ 0.31, 0.22 }, c{ 0.36, 0.18 }, c{ 0.44, 0.15 }, c{ 0.47, 0.08 }, c{ 0.52, -0.01 }, c{ 0.54, -0.06 }, c{ 0.6, -0.13 }, c{ 0.64, -0.17 }, c{ 0.69, -0.25 }, c{ 0.73, -0.33 }, c{ 0.76, -0.35 }, c{ 0.79, -0.43 }, c{ 0.82, -0.51 }, c{ 0.8, -0.58 }, c{ 0.76, -0.67 }, c{ 0.76, -0.76 }, c{ 0.81, -0.79 }, c{ 0.87, -0.83 }, c{ 0.9, -0.91 }, c{ 0.89, -0.99 }, c{ 0.89, -1.05 }, c{ 0.88, -1.15 }, c{ 0.88, -1.25 }, c{ 0.87, -1.33 }, c{ 0.92, -1.27 }, c{ 0.96, -1.24 }, c{ 0.98, -1.18 }, c{ 1.03, -1.13 }, c{ 1.06, -1.07 }, c{ 1.1, -1.13 }, c{ 1.15, -1.14 }, c{ 1.2, -1.1 }, c{ 1.23, -1.04 }, c{ 1.26, -0.98 }, c{ 1.32, -0.99 }, c{ 1.4, -1.01 }, c{ 1.47, -1 }, c{ 1.54, -0.98 }, c{ 1.6, -0.97 }, c{ 1.63, -1.03 }, c{ 1.7, -1.06 }, c{ 1.74, -1.09 }, c{ 1.78, -1.11 }, c{ 1.8, -1.05 }, c{ 1.87, -1.05 }, c{ 1.92, -1.05 }, c{ 1.99, -1.05 }, c{ 2.04, -1.01 }, c{ 2.1, -0.99 }, c{ 2.16, -0.95 }, c{ 2.16, -0.91 }, c{ 2.1, -0.89 }, c{ 2.03, -0.9 }, c{ 1.98, -0.89 }, c{ 1.94, -0.86 }, c{ 1.94, -0.83 }, c{ 1.99, -0.79 }, c{ 2.04, -0.76 }, c{ 2.11, -0.73 }, c{ 2.16, -0.71 }, c{ 2.23, -0.7 }, c{ 2.29, -0.67 }, c{ 2.34, -0.65 }, c{ 2.4, -0.63 }, c{ 2.46, -0.62 }, c{ 2.51, -0.6 }, c{ 2.56, -0.59 }, c{ 2.61, -0.56 }, c{ 2.66, -0.51 }, c{ 2.67, -0.45 }, c{ 2.61, -0.41 }, c{ 2.6, -0.33 }, c{ 2.57, -0.26 }, c{ 2.52, -0.19 }, c{ 2.5, -0.13 }, c{ 2.5, -0.06 }, c{ 2.5, 0.01 }, c{ 2.43, -0 }, c{ 2.34, 0.02 }, c{ 2.3, 0.05 }, c{ 2.28, 0.09 }, c{ 2.24, 0.14 }, c{ 2.19, 0.11 }, c{ 2.14, 0.18 }, c{ 2.12, 0.25 }, c{ 2.12, 0.31 }, c{ 2.07, 0.37 }, c{ 2.02, 0.41 }, c{ 1.94, 0.38 }, c{ 1.9, 0.32 }, c{ 1.84, 0.29 }, c{ 1.76, 0.29 }, c{ 1.74, 0.22 }, c{ 1.72, 0.15 }, c{ 1.7, 0.08 }, c{ 1.63, 0.05 }, c{ 1.57, -0 }, c{ 1.54, 0.07 }, c{ 1.52, 0.12 }, c{ 1.49, 0.17 }, c{ 1.44, 0.23 }, c{ 1.42, 0.31 }, c{ 1.41, 0.39 }, c{ 1.39, 0.46 }, c{ 1.39, 0.49 }, c{ 1.39, 0.52 }, c{ 1.39, 0.62 }, c{ 1.39, 0.67 }, c{ 1.4, 0.73 }, c{ 1.4, 0.81 }, c{ 1.4, 0.87 }, c{ 1.4, 0.94 }, c{ 1.4, 1 }, c{ 1.37, 1.09 }, c{ 1.35, 1.18 }, c{ 1.35, 1.25 }, c{ 1.34, 1.34 }, c{ 1.32, 1.41 }, c{ 1.32, 1.47 }, c{ 1.29, 1.54 }, c{ 1.28, 1.6 }, c{ 1.25, 1.67 }, c{ 1.2, 1.7 }, c{ 1.14, 1.73 }, c{ 1.11, 1.81 }, c{ 1.1, 1.9 }, c{ 1.1, 1.97 }, c{ 1.13, 2.03 }, c{ 1.15, 2.1 }, c{ 1.12, 2.13 }, c{ 1.08, 2.17 }, c{ 1.08, 2.23 }, c{ 1.1, 2.24 }, c{ 1.16, 2.3 }, c{ 1.22, 2.35 }, c{ 1.27, 2.41 }, c{ 1.22, 2.45 }, c{ 1.14, 2.45 }, c{ 1.08, 2.5 }, c{ 0.98, 2.51 }, c{ 0.92, 2.52 }, c{ 0.83, 2.52 }, c{ 0.73, 2.52 }, c{ 0.66, 2.52 }, c{ 0.6, 2.48 }, c{ 0.52, 2.43 }, c{ 0.46, 2.35 }, c{ 0.38, 2.3 }, c{ 0.3, 2.24 }, c{ 0.3, 2.13 }, c{ 0.27, 2.05 }, c{ 0.22, 2.01 }, c{ 0.16, 2.01 }, c{ 0.1, 2.02 }, c{ 0.05, 1.98 }, c{ 0, 2.01 }, c{ -0.05, 1.95 }, c{ -0.09, 2.02 }, c{ -0.15, 1.95 }, c{ -0.18, 1.99 }, c{ -0.25, 1.94 }, c{ -0.3, 1.99 }, c{ -0.36, 1.95 }, c{ -0.42, 1.99 }, c{ -0.46, 1.93 }, c{ -0.55, 1.95 }, c{ -0.6, 1.91 }, c{ -0.65, 1.93 }, c{ -0.71, 1.91 }, c{ -0.76, 1.96 }, c{ -0.8, 1.89 }, c{ -0.85, 1.91 }, c{ -0.89, 1.89 }, c{ -0.96, 1.89 }, c{ -1.02, 1.79 }, c{ -1.08, 1.74 }, c{ -1.14, 1.68 }, c{ -1.19, 1.71 }, c{ -1.26, 1.73 }, c{ -1.34, 1.73 }, c{ -1.4, 1.78 }, c{ -1.45, 1.81 }, c{ -1.5, 1.85 }, c{ -1.56, 1.95 }, c{ -1.61, 1.99 }, c{ -1.62, 2.07 }, c{ -1.7, 2.07 }, c{ -1.74, 2.12 }, c{ -1.78, 2.18 }, c{ -1.84, 2.24 }, c{ -1.88, 2.31 }, c{ -1.9, 2.39 }, c{ -1.9, 2.45 }, c{ -1.95, 2.47 }, c{ -2.04, 2.5 }, c{ -2.12, 2.52 }, c{ -2.21, 2.53 }, c{ -2.3, 2.53 }, c{ -2.36, 2.51 }, c{ -2.43, 2.48 }, c{ -2.5, 2.41 }, c{ -2.55, 2.33 }, c{ -2.59, 2.28 }, c{ -2.64, 2.18 }, c{ -2.54, 2.17 }, c{ -2.56, 2.09 }, c{ -2.62, 2.03 }, c{ -2.66, 1.96 }, c{ -2.66, 1.85 }, c{ -2.62, 1.77 }, c{ -2.62, 1.65 }, c{ -2.58, 1.59 }, c{ -2.54, 1.53 }, c{ -2.51, 1.49 }, c{ -2.47, 1.45 }, c{ -2.45, 1.37 }, c{ -2.43, 1.3 }, c{ -2.44, 1.19 }, c{ -2.38, 1.13 }, c{ -2.34, 1.03 }, c{ -2.3, 0.95 }, c{ -2.26, 0.93 }, c{ -2.2, 0.85 }, c{ -2.15, 0.81 }, c{ -2.09, 0.77 }, c{ -2.05, 0.71 }, c{ -2.02, 0.67 }, c{ -1.96, 0.63 }, c{ -1.88, 0.59 }, c{ -1.78, 0.55 }, c{ -1.75, 0.5 }, c{ -1.7, 0.46 }, c{ -1.65, 0.4 }, c{ -1.63, 0.32 }, c{ -1.6, 0.23 }, c{ -1.58, 0.13 }, c{ -1.56, 0.04 }, c{ -1.53, -0.01 }, c{ -1.49, -0.07 }, c{ -1.46, -0.16 }, c{ -1.38, -0.23 }, c{ -1.34, -0.29 }, c{ -1.29, -0.36 }, c{ -1.23, -0.41 }, c{ -1.16, -0.45 }, c{ -1.09, -0.48 }, c{ -1.02, -0.53 }, c{ -0.96, -0.56 }, c{ -0.91, -0.59 }, c{ -0.88, -0.57 }, c{ -0.91, -0.53 }, c{ -0.96, -0.45 }, c{ -1, -0.38 }, c{ -1.04, -0.33 }, c{ -1.06, -0.29 }, c{ -1.12, -0.25 }, c{ -1.16, -0.21 }, c{ -1.2, -0.15 }, c{ -1.24, -0.09 }, c{ -1.28, -0 }, c{ -1.3, 0.07 }, c{ -1.32, 0.17 }, c{ -1.32, 0.23 }, c{ -1.32, 0.28 }, c{ -1.27, 0.32 }, c{ -1.2, 0.34 }, c{ -1.12, 0.37 }, c{ -1.03, 0.41 }, c{ -0.95, 0.41 }, c{ -0.88, 0.43 }, c{ -0.78, 0.42 }, c{ -0.72, 0.4 }, c{ -0.63, 0.4 }, c{ -0.56, 0.39 }, c{ -0.5, 0.39 }, c{ -0.44, 0.38 }, c{ -0.34, 0.39 }, c{ -0.26, 0.39 }
			c{ -2.83, -2.1 }, c{ -3.11, -2.2 }, c{ -3.38, -2.2 }, c{ -3.61, -2.09 }, c{ -3.79, -1.95 }, c{ -3.95, -1.77 }, c{ -4.1, -1.53 }, c{ -4.14, -1.24 }, c{ -4.14, -1.06 }, c{ -4.02, -0.92 }, c{ -3.9, -0.87 }, c{ -3.63, -0.86 }, c{ -3.55, -0.94 }, c{ -3.38, -1.06 }, c{ -3.35, -1.1 }, c{ -3.38, -1.01 }, c{ -3.42, -0.88 }, c{ -3.42, -0.76 }, c{ -3.43, -0.56 }, c{ -3.5, -0.43 }, c{ -3.58, -0.31 }, c{ -3.69, -0.21 }, c{ -3.82, -0.15 }, c{ -3.82, -0.35 }, c{ -3.73, -0.43 }, c{ -3.62, -0.5 }, c{ -3.46, -0.53 }, c{ -3.2, -0.67 }, c{ -3.07, -0.75 }, c{ -2.97, -0.83 }, c{ -2.84, -0.95 }, c{ -2.67, -1.07 }, c{ -2.61, -1.14 }, c{ -2.57, -1.19 }, c{ -2.44, -0.96 }, c{ -2.33, -0.89 }, c{ -2.18, -0.84 }, c{ -1.97, -0.86 }, c{ -1.86, -0.89 }, c{ -1.73, -0.94 }, c{ -1.64, -1 }, c{ -1.54, -1.07 }, c{ -1.38, -1.07 }, c{ -1.25, -1.13 }, c{ -1.13, -1.21 }, c{ -1.02, -1.3 }, c{ -0.96, -1.42 }, c{ -0.95, -1.5 }, c{ -1.01, -1.52 }, c{ -1.06, -1.46 },
			c{ -1.11, -1.39 }, c{ -1.21, -1.31 }, c{ -1.34, -1.26 }, c{ -1.42, -1.22 }, c{ -1.44, -1.24 }, c{ -1.38, -1.35 }, c{ -1.34, -1.51 }, c{ -1.33, -1.68 }, c{ -1.32, -1.79 }, c{ -1.33, -1.93 }, c{ -1.39, -2.07 }, c{ -1.5, -2.18 }, c{ -1.68, -2.21 }, c{ -1.79, -2.21 }, c{ -1.94, -2.17 }, c{ -2.11, -2.09 }, c{ -2.18, -2.03 }, c{ -2.24, -1.98 }, c{ -2.32, -1.91 }, c{ -2.39, -1.79 }, c{ -2.43, -1.65 }, c{ -2.5, -1.53 }, c{ -2.53, -1.47 }, c{ -2.59, -1.36 }, c{ -2.69, -1.26 }, c{ -2.78, -1.16 }, c{ -2.83, -1.11 }, c{ -2.93, -1.05 }, c{ -2.99, -1.01 }, c{ -3.06, -0.96 }, c{ -3.09, -0.94 }, c{ -3.46, -0.78 }, c{ -3.57, -0.75 }, c{ -3.66, -0.69 }, c{ -3.76, -0.61 }, c{ -3.9, -0.55 }, c{ -4.01, -0.48 }, c{ -4.06, -0.43 }, c{ -4.1, -0.31 }, c{ -4.1, -0.23 }, c{ -4.1, -0.12 }, c{ -4.09, -0.1 }, c{ -4, -0.03 }, c{ -3.91, 0.01 }, c{ -3.81, 0.04 }, c{ -3.71, 0.04 }, c{ -3.57, -0.01 }, c{ -3.53, -0.07 }, c{ -3.4, -0.15 }, c{ -3.35, -0.21 },
			c{ -3.31, -0.29 }, c{ -3.31, -0.39 }, c{ -3.27, -0.52 }, c{ -3.25, -0.63 }, c{ -3.16, -0.76 }, c{ -3.16, -0.83 }, c{ -3.16, -0.87 }, c{ -3.14, -0.97 }, c{ -2.92, -1.71 }, c{ -2.94, -1.81 }, c{ -3.02, -1.82 }, c{ -3.06, -1.82 }, c{ -3.12, -1.82 }, c{ -3.15, -1.71 }, c{ -3.26, -1.54 }, c{ -3.29, -1.47 }, c{ -3.38, -1.37 }, c{ -3.52, -1.23 }, c{ -3.67, -1.1 }, c{ -3.78, -1.07 }, c{ -3.81, -1.08 }, c{ -3.82, -1.27 }, c{ -3.81, -1.5 }, c{ -3.73, -1.67 }, c{ -3.6, -1.83 }, c{ -3.5, -1.94 }, c{ -3.26, -2 }, c{ -3.14, -1.99 }, c{ -2.99, -1.93 }, c{ -2.94, -1.91 }, c{ -2.81, -1.87 }, c{ -2.7, -1.95 }, c{ -2.7, -2.02 }, c{ -2.83, -2.11 }, c{ -2.97, -2.19 }, c{ -3.24, -2.19 }, c{ -3.49, -2.13 }, c{ -3.7, -1.99 }, c{ -3.86, -1.85 }, c{ -4.04, -1.65 }, c{ -4.12, -1.38 }, c{ -4.13, -1.04 }, c{ -3.94, -0.92 }, c{ -3.82, -0.87 }, c{ -3.6, -0.87 }, c{ -3.44, -0.97 }, c{ -3.33, -1.09 }, c{ -3.42, -0.67 }, c{ -3.55, -0.37 }, c{ -3.62, -0.19 },
			c{ -3.78, -0.17 }, c{ -3.91, -0.19 }, c{ -3.88, -0.3 }, c{ -3.79, -0.39 }, c{ -3.66, -0.48 }, c{ -3.58, -0.52 }, c{ -3.46, -0.55 }, c{ -3.35, -0.63 }, c{ -3.05, -0.75 }, c{ -2.9, -0.87 }, c{ -2.77, -0.99 }, c{ -2.66, -1.09 }, c{ -2.61, -1.18 }, c{ -2.56, -1.08 }, c{ -2.51, -1.03 }, c{ -2.43, -0.95 }, c{ -2.27, -0.87 }, c{ -2.06, -0.84 }, c{ -1.9, -0.84 }, c{ -1.76, -0.9 }, c{ -1.66, -0.99 }, c{ -1.52, -1.03 }, c{ -1.46, -1.07 }, c{ -1.34, -1.07 }, c{ -1.21, -1.11 }, c{ -1.12, -1.17 }, c{ -1.02, -1.3 }, c{ -0.94, -1.42 }, c{ -0.95, -1.56 }, c{ -0.17, -2.14 }, c{ -0.03, -2.15 }, c{ 0.09, -2.15 }, c{ 0.16, -2.11 }, c{ 0.23, -2.12 }, c{ 0.29, -2.19 }, c{ 0.4, -2.21 }, c{ 0.44, -2.23 }, c{ 0.5, -2.23 }, c{ 0.58, -2.23 }, c{ 0.64, -2.21 }, c{ 0.7, -2.18 }, c{ 0.79, -2.1 }, c{ 0.86, -1.99 }, c{ 0.86, -1.87 }, c{ 0.86, -1.71 }, c{ 0.86, -1.59 }, c{ 0.78, -1.47 }, c{ 0.78, -1.35 }, c{ 0.74, -1.3 }, c{ 0.7, -1.2 },
			c{ 0.86, -1.2 }, c{ 1.01, -1.28 }, c{ 1.1, -1.39 }, c{ 1.18, -1.54 }, c{ 1.24, -1.76 }, c{ 1.25, -1.87 }, c{ 1.3, -1.96 }, c{ 1.32, -2.08 }, c{ 1.38, -2.24 }, c{ 1.5, -2.47 }, c{ 1.58, -2.65 }, c{ 1.76, -2.86 }, c{ 1.94, -2.96 }, c{ 2.17, -2.97 }, c{ 2.27, -2.88 }, c{ 2.28, -2.7 }, c{ 2.26, -2.55 }, c{ 2.26, -2.43 }, c{ 2.22, -2.35 }, c{ 2.17, -2.28 }, c{ 2.1, -2.19 }, c{ 2.02, -2.07 }, c{ 1.89, -1.95 }, c{ 1.76, -1.83 }, c{ 1.67, -1.75 }, c{ 1.58, -1.7 }, c{ 1.48, -1.65 }, c{ 1.54, -1.86 }, c{ 1.55, -2.08 }, c{ 1.62, -2.23 }, c{ 1.68, -2.39 }, c{ 1.74, -2.48 }, c{ 1.79, -2.57 }, c{ 1.85, -2.64 }, c{ 1.89, -2.69 }, c{ 1.98, -2.73 }, c{ 2.02, -2.71 }, c{ 2.04, -2.59 }, c{ 2.02, -2.48 }, c{ 1.99, -2.4 }, c{ 1.94, -2.34 }, c{ 1.86, -2.23 }, c{ 1.79, -2.15 }, c{ 1.7, -2.04 }, c{ 1.62, -1.96 }, c{ 1.55, -1.88 }, c{ 1.54, -1.83 }, c{ 1.47, -1.7 }, c{ 1.45, -1.56 }, c{ 1.45, -1.43 },
			c{ 1.45, -1.34 }, c{ 1.43, -1.23 }, c{ 1.48, -1.15 }, c{ 1.54, -1.09 }, c{ 1.62, -1.05 }, c{ 1.74, -1.05 }, c{ 1.88, -1.12 }, c{ 1.96, -1.22 }, c{ 2.01, -1.27 }, c{ 2.09, -1.37 }, c{ 2.2, -1.57 }, c{ 2.25, -1.69 }, c{ 2.32, -1.81 }, c{ 2.44, -1.9 }, c{ 2.58, -2 }, c{ 2.65, -2.06 }, c{ 2.83, -2.15 }, c{ 2.96, -2.16 }, c{ 3.12, -2.16 }, c{ 3.25, -2.16 }, c{ 3.38, -2.16 }, c{ 3.46, -2.12 }, c{ 3.52, -2.07 }, c{ 3.55, -2.03 }, c{ 3.53, -1.94 }, c{ 3.48, -1.87 }, c{ 3.38, -1.91 }, c{ 3.34, -1.94 }, c{ 3.22, -1.95 }, c{ 3.1, -1.96 }, c{ 3.02, -1.96 }, c{ 2.88, -1.96 }, c{ 2.74, -1.86 }, c{ 2.65, -1.79 }, c{ 2.56, -1.68 }, c{ 2.5, -1.59 }, c{ 2.45, -1.47 }, c{ 2.43, -1.28 }, c{ 2.43, -1.2 }, c{ 2.47, -1.11 }, c{ 2.56, -1.09 }, c{ 2.65, -1.09 }, c{ 2.74, -1.17 }, c{ 2.8, -1.29 }, c{ 2.86, -1.39 }, c{ 2.9, -1.45 }, c{ 2.93, -1.52 }, c{ 2.98, -1.62 }, c{ 3.01, -1.7 }, c{ 3.03, -1.78 },
			c{ 3.05, -1.84 }, c{ 3.12, -1.83 }, c{ 3.25, -1.82 }, c{ 3.34, -1.82 }, c{ 3.3, -1.73 }, c{ 3.27, -1.63 }, c{ 3.26, -1.48 }, c{ 3.23, -1.35 }, c{ 3.22, -1.26 }, c{ 3.21, -1.21 }, c{ 3.22, -1.17 }, c{ 3.25, -1.13 }, c{ 3.3, -1.12 }, c{ 3.36, -1.12 }, c{ 3.44, -1.16 }, c{ 3.51, -1.25 }, c{ 3.56, -1.31 }, c{ 3.62, -1.42 }, c{ 3.67, -1.48 }, c{ 3.71, -1.6 }, c{ 3.74, -1.71 }, c{ 3.78, -1.84 }, c{ 3.8, -1.92 }, c{ 3.83, -1.99 }, c{ 3.87, -2.09 }, c{ 3.88, -2.12 }, c{ 3.97, -2.12 }, c{ 4.04, -2.15 }, c{ 4.06, -2.31 }, c{ 3.98, -2.39 }, c{ 3.97, -2.51 }, c{ 4.01, -2.59 }, c{ 4.1, -2.64 }, c{ 4.16, -2.66 }, c{ 4.22, -2.65 }, c{ 4.26, -2.62 }, c{ 4.29, -2.55 }, c{ 4.29, -2.47 }, c{ 4.29, -2.44 }, c{ 4.24, -2.39 }, c{ 4.2, -2.35 }, c{ 4.14, -2.32 }, c{ 4.08, -2.13 }, c{ 4.18, -2.13 }, c{ 4.18, -2.03 }, c{ 4.14, -1.96 }, c{ 4.1, -1.89 }, c{ 4.06, -1.76 }, c{ 4.05, -1.65 }, c{ 3.98, -1.47 },
			c{ 3.96, -1.35 }, c{ 3.94, -1.23 }, c{ 3.94, -1.16 }, c{ 3.94, -1.13 }, c{ 4.02, -1.1 }, c{ 4.1, -1.11 }, c{ 4.19, -1.19 }, c{ 4.28, -1.27 }, c{ 4.34, -1.39 }, c{ 4.36, -1.55 }, c{ 4.39, -1.63 }, c{ 4.48, -1.77 }, c{ 4.54, -1.84 }, c{ 4.62, -1.91 }, c{ 4.7, -1.99 }, c{ 4.78, -2.04 }, c{ 4.86, -2.11 }, c{ 4.94, -2.15 }, c{ 5.07, -2.16 }, c{ 5.21, -2.17 }, c{ 5.27, -2.17 }, c{ 5.33, -2.17 }, c{ 5.64, -3.01 }, c{ 5.9, -3 }, c{ 5.94, -2.9 }, c{ 5.88, -2.79 }, c{ 5.82, -2.63 }, c{ 5.76, -2.5 }, c{ 5.7, -2.36 }, c{ 5.69, -2.18 }, c{ 5.58, -1.99 }, c{ 5.52, -1.72 }, c{ 5.48, -1.51 }, c{ 5.46, -1.35 }, c{ 5.47, -1.16 }, c{ 5.51, -1.11 }, c{ 5.62, -1.11 }, c{ 5.74, -1.19 }, c{ 5.82, -1.29 }, c{ 5.88, -1.43 }, c{ 5.93, -1.49 }, c{ 5.98, -1.58 }, c{ 6.02, -1.55 }, c{ 6.06, -1.5 }, c{ 6.04, -1.4 }, c{ 5.98, -1.32 }, c{ 5.91, -1.24 }, c{ 5.86, -1.16 }, c{ 5.8, -1.07 }, c{ 5.75, -1.03 },
			c{ 5.69, -0.97 }, c{ 5.59, -0.91 }, c{ 5.56, -0.88 }, c{ 5.49, -0.87 }, c{ 5.34, -0.87 }, c{ 5.32, -0.87 }, c{ 5.25, -0.93 }, c{ 5.19, -1.05 }, c{ 5.19, -1.12 }, c{ 5.15, -1.2 }, c{ 5.15, -1.39 }, c{ 5.15, -1.58 }, c{ 5.24, -1.79 }, c{ 5.26, -1.87 }, c{ 5.28, -1.95 }, c{ 5.14, -1.96 }, c{ 5.05, -1.9 }, c{ 4.91, -1.79 }, c{ 4.82, -1.71 }, c{ 4.73, -1.57 }, c{ 4.66, -1.45 }, c{ 4.66, -1.35 }, c{ 4.66, -1.18 }, c{ 4.69, -1.13 }, c{ 4.79, -1.11 }, c{ 4.88, -1.11 }, c{ 4.98, -1.18 }, c{ 5.04, -1.27 }, c{ 5.1, -1.42 }, c{ 5.13, -1.51 }, c{ 5.1, -1.36 }, c{ 5.1, -1.25 }, c{ 5.1, -1.16 }, c{ 5.09, -1.09 }, c{ 5, -1.01 }, c{ 4.95, -0.96 }, c{ 4.87, -0.91 }, c{ 4.75, -0.87 }, c{ 4.63, -0.85 }, c{ 4.54, -0.85 }, c{ 4.49, -0.86 }, c{ 4.41, -0.92 }, c{ 4.36, -0.99 }, c{ 4.3, -1.09 }, c{ 4.29, -1.12 }, c{ 4.24, -1.09 }, c{ 4.19, -1.03 }, c{ 4.17, -1 }, c{ 4.11, -0.97 }, c{ 4.05, -0.93 },
			c{ 3.98, -0.89 }, c{ 3.93, -0.86 }, c{ 3.78, -0.83 }, c{ 3.74, -0.83 }, c{ 3.67, -0.87 }, c{ 3.63, -0.93 }, c{ 3.59, -1 }, c{ 3.57, -1.07 }, c{ 3.5, -1.07 }, c{ 3.47, -1.03 }, c{ 3.43, -0.98 }, c{ 3.38, -0.94 }, c{ 3.35, -0.91 }, c{ 3.29, -0.89 }, c{ 3.14, -0.89 }, c{ 3.04, -0.89 }, c{ 2.99, -0.89 }, c{ 2.95, -0.95 }, c{ 2.92, -1.03 }, c{ 2.91, -1.11 }, c{ 2.88, -1.16 }, c{ 2.79, -1.07 }, c{ 2.75, -1 }, c{ 2.7, -0.95 }, c{ 2.65, -0.91 }, c{ 2.62, -0.9 }, c{ 2.54, -0.87 }, c{ 2.38, -0.87 }, c{ 2.33, -0.87 }, c{ 2.25, -0.93 }, c{ 2.19, -0.98 }, c{ 2.1, -1.07 }, c{ 2.06, -1.12 }, c{ 2, -1.09 }, c{ 1.96, -1.04 }, c{ 1.93, -1 }, c{ 1.88, -0.97 }, c{ 1.81, -0.91 }, c{ 1.71, -0.85 }, c{ 1.6, -0.81 }, c{ 1.55, -0.82 }, c{ 1.48, -0.83 }, c{ 1.34, -0.87 }, c{ 1.23, -0.94 }, c{ 1.19, -0.99 }, c{ 1.13, -1.14 }, c{ 1.12, -1.23 }, c{ 1.1, -1.23 }, c{ 1.06, -1.19 }, c{ 1.02, -1.15 },
			c{ 0.99, -1.12 }, c{ 0.95, -1.1 }, c{ 0.87, -1.06 }, c{ 0.79, -1.07 }, c{ 0.66, -1.07 }, c{ 0.58, -1.08 }, c{ 0.54, -1.03 }, c{ 0.53, -1 }, c{ 0.49, -0.96 }, c{ 0.43, -0.91 }, c{ 0.32, -0.87 }, c{ 0.24, -0.87 }, c{ 0.08, -0.87 }, c{ -0.07, -0.87 }, c{ -0.1, -0.88 }, c{ -0.17, -0.94 }, c{ -0.2, -0.99 }, c{ -0.12, -1.19 }, c{ -0.1, -1.32 }, c{ 0.06, -1.76 }, c{ 0.21, -1.95 }, c{ 0.37, -1.95 }, c{ 0.46, -1.95 }, c{ 0.55, -1.91 }, c{ 0.58, -1.84 }, c{ 0.58, -1.71 }, c{ 0.58, -1.6 }, c{ 0.54, -1.46 }, c{ 0.5, -1.35 }, c{ 0.45, -1.26 }, c{ 0.39, -1.18 }, c{ 0.32, -1.14 }, c{ 0.22, -1.08 }, c{ 0.09, -1.04 }, c{ -0.02, -1.07 }, c{ -0.1, -1.14 }, c{ -0.18, -1.03 }, c{ -0.22, -0.85 }, c{ -0.22, -0.78 }, c{ -0.28, -0.57 }, c{ -0.33, -0.41 }, c{ -0.37, -0.28 }, c{ -0.38, -0.15 }, c{ -0.46, 0 }, c{ -0.74, 0.03 }, c{ -0.47, -1.16 }, c{ -0.19, -2.15 }, c{ -0.98, -1.56 }, c{ -1.06, -1.45 }, c{ -1.18, -1.35 },
			c{ -1.31, -1.29 }, c{ -1.45, -1.22 }, c{ -1.46, -1.25 }, c{ -1.73, -1.3 }, c{ -1.66, -1.52 }, c{ -1.63, -1.71 }, c{ -1.62, -1.88 }, c{ -1.7, -1.96 }, c{ -1.89, -1.98 }, c{ -1.99, -1.94 }, c{ -2.1, -1.81 }, c{ -2.21, -1.62 }, c{ -2.21, -1.48 }, c{ -2.21, -1.24 }, c{ -2.18, -1.12 }, c{ -2.1, -1.07 }, c{ -2.01, -1.05 }, c{ -1.92, -1.07 }, c{ -1.84, -1.12 }, c{ -1.78, -1.23 }, c{ -1.76, -1.26 }, c{ -1.46, -1.24 }, c{ -1.38, -1.39 }, c{ -1.35, -1.56 }, c{ -1.31, -1.67 }, c{ -1.31, -1.76 }, c{ -1.33, -1.92 }, c{ -1.38, -2.02 }, c{ -1.46, -2.08 }, c{ -1.57, -2.21 }, c{ -1.65, -2.24 }, c{ -1.86, -2.24 }, c{ -2, -2.22 }, c{ -2.09, -2.17 }, c{ -2.21, -2.07 }, c{ -2.3, -1.95 }, c{ -2.34, -1.91 }, c{ -2.71, -1.95 }
		};
		imageFile.clear();
		for (int i = 0; i < v.size(); i++){
			imageFile << "c{ " << std::setprecision(3) << v[i].re << ", " << std::setprecision(3) << -v[i].im << "}, ";
			if (i % 50 == 0) imageFile << "\n";
			imageFile = std::ofstream("imagTXT.txt", ofstream::app);
		}
		//interpolate points in btwn the fake points
		coords = interpolateComplex(v, 2);//interpolated points//LOOK UP SPLINES
		coords = scale(coords, 1);
		
		fourierSeries fourier = discreteFourierTransform(coords);
		fourier = sort(fourier);//sorts in order of amplitudes (radii)
		const int NUM_ARROWS = 1000;
		for (int i = 0; i < NUM_ARROWS; i++) {
			if (i < fourier.size()){
				float freq = fourier[i].freq;//angular vel
				float radius = fourier[i].amplitude;//radius
				float phase = fourier[i].phase;//initial angle
				addArrow(arrow{ radius, freq, phase, initP });
			}
		}
		dt = fourier.size() / (2 * M_PI);//NEED THIS

	}
	bool coloured(vec3 pixel, const int pThresh = 20){
		return (pixel.X > pThresh || pixel.Y > pThresh || pixel.Z > pThresh);
	}
	float calcDist2(complex i, complex j) {
		float deltaX = i.re - j.re;
		float deltaY = i.im - j.im;
		return (sqr(deltaX) + sqr(deltaY));//pythag
	}
	std::vector<complex> sortf(std::vector<complex> d) {
		//now have to SORT all the pixels
		std::vector<complex> sortedPoints;
#if 1
		int start_index = 0;
		std::swap(d[start_index], d.back());
		sortedPoints.push_back(d.back());
		d.resize(d.size() - 1);
		while (!d.empty()) {
			int closest_index = 0;
			// find the closest element to sortedPoints.back()
			double dist2 = calcDist2(d.front(), sortedPoints.front());
			for (int i = 1; i < d.size(); ++i) {
				complex candidate = d[i];
				float candidate_dist2 = calcDist2(candidate, sortedPoints.back());
				if (candidate_dist2 < dist2 ) {
					dist2 = candidate_dist2;
					closest_index = i;	
				}
			}
			std::swap(d[closest_index], d.back());
			if (dist2 < sqr(0.1)) sortedPoints.push_back(d.back());
			d.resize(d.size() - 1);
		}
#else
		sortedPoints.push_back(d[0]);//gets first point
		std::vector<int> usedIndex;
		usedIndex.push_back(0);
		const int dSize = d.size();
		for (int i = 0; i < dSize; i++) {
			int closestInd = usedIndex[usedIndex.size() - 1];//individually closest one
			float dist = 100;//arbitratrily large error
			for (int j = 0; j < dSize; j++) {//calculate all distances(could probably be better to only calculate nearby distances)
				if (usedIndex.empty() || std::find(usedIndex.begin(), usedIndex.end(), j) == usedIndex.end()) {//j not already used (used vector "!contains")
					float newDist = calcDist(d[closestInd], d[j]);
					if (newDist < dist) {//dist from i to j
						closestInd = j;
						dist = newDist;
					}
				}
			}
			usedIndex.push_back(closestInd);//good index
			sortedPoints.push_back(drawPoints[closestInd]);
		}
#endif
		return sortedPoints;
	}
	void autoInit(std::vector<std::vector<vec3>> imagePix){
		for (int i = 1; i < imagePix.size(); i++){//through every line (starts after edge)
			for (int j = 1; j < imagePix[0].size(); j++){//through every pixel (starts after edge)
				bool edge = (i == imagePix.size()-1 || j == imagePix[0].size()-1);//not left or bottom edge
				if (coloured(imagePix[i][j]) && !edge){
					if (!coloured(imagePix[i - 1][j]) /*top*/ || !coloured(imagePix[i + 1][j]) /*bottom*/ ||
						!coloured(imagePix[i][j - 1]) /*left*/ || !coloured(imagePix[i][j + 1]) /*right*/)
					{
						float xPos = j / ppm - initP.X / 2;
						float yPos = i / ppm - initP.Y / 2;
						drawPoints.push_back(complex(xPos, yPos));
					}
				}
			}
		}
		//now have to SORT all the pixels
		drawPoints = sortf(drawPoints);//updates drawPoints with new sorted ones
	}
	void addRandom() {
		vec3 newPos;
		if (train.empty()) {
			newPos = initP;
		}
		else {
			last = train.size() - 1;
			newPos = train[last].tip();//floating point
		}
		addArrow(arrow(0.5 + (rand() % 100) / 100.0, (rand() % 1000) / 100.0 - 5, (rand() % 628) / 100.0, newPos));//floating point
	}
	void update(int freq) {
		if (!train.empty()) {
			for (int j = 0; j < freq; j++) {
				last = train.size() - 1;
				for (int i = 0; i < last + 1; i++) {
					train[i].theta += train[i].velocity / dt;//updates position over time
					while (train[i].theta > 2 * M_PI) {
						train[i].theta -= 2 * M_PI;
					}
					while (train[i].theta < -2 * M_PI) {
						train[i].theta += 2 * M_PI;
					}

					if (i > 0) train[i].pos = train[i - 1].tip();//ensures all positions are tip to tip
				}
				path.push_back(std::pair<vec3, vec3>(tartan(path.size()), train[last].tip()));
			}
		}
	}
	void penToggle() {
		penDown = !penDown;//to draw or not to draw
	}
	vec3 tartan(int i) {
		const vec3 BLUE{ 50, 50, 200 }, GREEN{ 0, 140, 0 }, YELLOW{ 200, 200, 0 }, RED{ 200, 0, 0 }, DGREEN{0, 100, 0};
		std::vector<int> div = { 100, 5, 35, 50 };
		int colour = i % sum(div);
		std::vector<vec3> colours = { GREEN, YELLOW, RED, DGREEN};
		for (int j = 0; j < div.size(); j++) {
			if (colour < div[j]) return colours[j].times(1/255.0);//checks if colour(index) is less than div[j]
			colour -= div[j];//else checks next div[j] by ignoring previous ones
		}
		//should never happen
		return vec3{ 1, 1, 1 };
	}
	void draw() {
		const float width = 0.04;//3cm constant width for all arrows
		if (!hide) {
			for (int i = 0; i < train.size(); i++) {
				glPushMatrix();//rotation
				gl::translate(Vec3f(ppm*train[i].pos.X, ppm*train[i].pos.Y, 0));//origin of rotation
				gl::rotate(Vec3f(0, 0, toDeg(train[i].theta)));//3rd input used for 2D rotation (angle)
				gl::color(1, 1, 1);//cool colour transition (neat maths)
				gl::drawSolidRect(Area(//final transformation
					ppm*Vec2f(0, -width / 2), //([X] no change,[Y] half of height (center))
					ppm*Vec2f(train[i].length, width / 2)//([X] add radius,[Y] half of height (center))
				));
				const float triScale = 0.09;//scale of size of triangle
				float triHeight = 1.732*triScale*train[i].length;//achieved thru height of triangle (L = 2*C*x*cos(30°)), with triscale(c) and length(x)
				gl::drawSolidTriangle(
					ppm*Vec2f(train[i].length - triHeight, -width / 2 - triScale * train[i].length),
					ppm*Vec2f(train[i].length - triHeight, width / 2 + triScale * train[i].length),
					ppm*Vec2f(train[i].length, 0)
				);
				glPopMatrix();//end of rotation code
				gl::color(200 / 255.0, 200 / 255.0, 200 / 255.0);
				gl::drawStrokedCircle(ppm*Vec2f(train[i].pos.X, train[i].pos.Y), ppm*train[i].length);
			}
		}
		if (!coords.empty() && pointDraw) {
			for (int i = 0; i < coords.size(); i++) {
				gl::color(0, 0.5, 1);//light blue
				gl::drawSolidCircle(ppm*Vec2f(coords[i].re + initP.X, coords[i].im + initP.Y), 3);
			}
		}
		if (penDown) {
			for (int i = 1; i < path.size(); i++) {//start @ 2nd to not worry abt vector end
				gl::color(path[i - 1].first.X, path[i - 1].first.Y, path[i - 1].first.Z);//TARTAN
				gl::drawLine(ppm*Vec2f(path[i - 1].second.X, path[i - 1].second.Y), ppm*Vec2f(path[i].second.X, path[i].second.Y));
			}
			gl::color(1, 1, 1);
		}
	}
};
class FourierDrawApp : public AppNative {
  public:
	void prepareSettings(Settings *settings);
	void setup();
	void mouseDown(MouseEvent event);
	void keyDown(KeyEvent event);
	static void drawFontText(float text, vec3 pos);
	void update();
	void draw();
	drawing d;
	float dt = 600;// 60hz refresh rate
	int freq = 1;
	ci::gl::Texture image;
	bool drawImage = true;
	const float imScale = 1;
	std::vector<std::vector<vec3>> imagePix;//vetor of vectors for pixel to rectangular coords
private:
	// Change screen resolution
	int mScreenWidth, mScreenHeight;
	float initWidth, initHeight;
	void getScreenResolution(int& width, int& height);
};
void FourierDrawApp::prepareSettings(Settings *settings) {
	//! setup our window
	settings->setTitle("Fourier Drawing");
	settings->setFrameRate(60);//60fps
	gl::enableVerticalSync();//vsync
	getScreenResolution(mScreenWidth, mScreenHeight);//gets resolution relative to monitor
	settings->setWindowPos(mScreenWidth / 6, mScreenHeight / 6);
	int aspectRatio = mScreenWidth / 7;//using 4/7ths of monitor resolution
	initWidth = aspectRatio * 4;
	initHeight = aspectRatio * 3;
	settings->setWindowSize(initWidth, initHeight);//maintains 4:3 aspect ratio
}
void FourierDrawApp::getScreenResolution(int& width, int& height) {
	const HWND hDesktop = GetDesktopWindow();// Get a handle to the desktop window
	RECT rDesktop;// Get the size of the screen into a rectangle
	GetWindowRect(hDesktop, &rDesktop);
	width = rDesktop.right;
	height = rDesktop.bottom;
}
void FourierDrawApp::setup(){
	srand(time(NULL));//seeds random number generator
	//custom fonts
	mFont = Font("Arial", 45);//fixed custom font
	mTextureFont = gl::TextureFont::create(mFont);
	//clear text file
	imageFile.clear();
	scriptFile.clear();
	//surface & image
	d.initP = vec3(getWindowWidth() / (2 * ppm), getWindowHeight() / (2 * ppm));
	Surface myPicture = loadImage(loadAsset("test.png"));//WORKS
	image = gl::Texture(myPicture);
	//walk the pixels
	Area area(0, 0, image.getWidth(), image.getHeight());
	Surface::Iter iter = myPicture.getIter(area);
	while (iter.line()) {
		std::vector<vec3> linePix;
		while (iter.pixel()) {
			vec3 RGB = vec3(iter.r(), iter.g(), iter.b());
			linePix.push_back(RGB);
		}
		imagePix.push_back(linePix);
	}
	d.autoInit(imagePix);
}
void FourierDrawApp::mouseDown(MouseEvent event) {
	if (event.isLeft()) {
		const float xPos = event.getX() / ppm - d.initP.X;
		const float yPos = event.getY() / ppm - d.initP.Y;
		d.drawPoints.push_back(complex(xPos, yPos));
	}
	if (event.isRight()) {
		d.path.clear();//clear current path
		d.penToggle();
	}
}
void FourierDrawApp::keyDown(KeyEvent event) {
	if (event.getCode() == KeyEvent::KEY_UP) freq += 1;//makes everything a bit faster
	if (event.getCode() == KeyEvent::KEY_DOWN) freq -= 1;//makes everything a bit slower
	if (event.getCode() == KeyEvent::KEY_RIGHT) {
		ppm += 5;//makes everything a bit larger
		if(!d.train.empty()) d.train[0].pos = vec3(getWindowWidth() / (2 * ppm), getWindowHeight() / (2 * ppm));//centers
	}
	if (event.getCode() == KeyEvent::KEY_LEFT) {
		if (ppm > 5) ppm -= 5;//makes everything a bit smaller
		if (!d.train.empty()) d.train[0].pos = vec3(getWindowWidth() / (2 * ppm), getWindowHeight() / (2 * ppm));//centers

	}
	if (event.getCode() == KeyEvent::KEY_DELETE) {
		d.drawPoints.pop_back();
	}
	if (event.getChar() == 'p'){
		d.train.pop_back();
	}
	if (event.getCode() == KeyEvent::KEY_SPACE) if (freq != 0) freq = 0; else freq = 1;//toggles between play & pause
	if (event.getChar() == 'r') {
		d.train.clear();
		completeTxt.clear();
	}
	if (event.getChar() == 'h') {
		d.hide = !d.hide;
	}
	if (event.getChar() == 'j') {
		d.pointDraw = !d.pointDraw;
	}
	if (event.getChar() == 'q'){
		d.addRandom();
	}
	if (event.getChar() == 'f'){
		d.fourierInit();
		d.pointDraw = false;//much clutter
		drawImage = false;
	}


}
void FourierDrawApp::update()
{
	/*static int counter = 0;
	if(counter++ % 4 == 0)*/
	d.update(freq);
}
void FourierDrawApp::drawFontText(float text, vec3 pos) {
	std::stringstream dummyText;
	std::string PRINT;
	dummyText << text;
	dummyText >> PRINT;
	gl::color(1, 1, 1);
	mTextureFont->drawString(PRINT, Vec2f(pos.X, pos.Y + 20));
	gl::color(1, 1, 1);
}
void FourierDrawApp::draw()
{
	gl::enableAlphaBlending();//good for transparent images
	gl::clear(Color(0, 0, 63 / 255.0));//dark blue
	gl::color(1, 1, 1);

	d.draw();
	const vec3 initP{ 1, 1 };
	//gl::drawString(completeTxt, ppm*Vec2f(initP.X, initP.Y), Color(1, 1, 1), Font("Arial", 35));
	if (drawImage){
		gl::color(1, 1, 1);
		gl::draw(image, Area(ppm*Vec2f(d.initP.X/2, d.initP.Y/2), Vec2f(d.initP.X*0.5*ppm + image.getWidth()*imScale, d.initP.Y*0.5*ppm + image.getHeight()*imScale)));
	}
	if (d.pointDraw){
		for (int i = 0; i < d.drawPoints.size(); i++){
			gl::color(1, 0.5, 0);//orange
			gl::drawSolidCircle(ppm*Vec2f(d.drawPoints[i].re + d.initP.X, -d.drawPoints[i].im + d.initP.Y), 2);
		}
	}
	gl::drawString("FPS: ", Vec2f(getWindowWidth() - 250, 10), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(getAverageFps(), vec3(getWindowWidth() - 130, 10));

	gl::drawString("RefRt: ", Vec2f(getWindowWidth() - 250, 50), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(d.train.size() / (2 * M_PI)*freq, vec3(getWindowWidth() - 130, 50));

	gl::drawString("Num: ", Vec2f(getWindowWidth() - 250, 90), Color(1, 1, 1), Font("Arial", 45));
	drawFontText(d.train.size(), vec3(getWindowWidth() - 130, 90));
}

CINDER_APP_NATIVE( FourierDrawApp, RendererGl )
