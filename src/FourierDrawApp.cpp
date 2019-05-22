#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include <vector>
#include <string.h>
#include <array>
#include "cinder/gl/TextureFont.h"
#include <fstream>
#include <filesystem>
using namespace ci;
using namespace ci::app;
using namespace std;

float ppm = 100;//scale of how many pixels are in 1 SI Meter
Font mFont;//custom font for optimized drawing
gl::TextureFontRef mTextureFont;//custom opengl::texture
std::ofstream scriptFile;
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
	complex(double r, double i) : re(r), im(-i) {}
	float re, im;//real and imaginary components
	complex mult(complex a) {//MAKE THIS INTO CLASS FUNCTION
		float re_new = a.re*re - a.im*im;//i^2 = -1
		float im_new = a.re*im + a.im*re;
		return(complex{ re_new, im_new });
	}
	complex operator* (float scalar) {//MAKE THIS INTO CLASS FUNCTION
		return(complex{ scalar*re, -scalar*im });
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
		scaled.push_back (v[i] * (scalar));//33% size
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
	void addArrow(arrow a) {
		train.push_back(a);
		int i = train.size() - 1;//index for newest arrow
		string c = (std::to_string(i + 1) + "  len: " + std::to_string(train[i].length) + "   vel: " + std::to_string(train[i].velocity) + "\n");//kinda long precision but owell
		scriptFile << c;//used for scripting
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
		scriptFile.clear();
		std::vector<complex> v = {
			c{ 0, 3 }, c{ 3, 5 }, c{ 4, 9.2 }, c{ 6, 8 }, c{ 8.1, 8 },
			c{ 9.5, 7 }, c{ 11, 5.5 }, c{ 10, 3 }, c{ 9, 2.4 }, c{ 6.5, 2.5 },
			c{ 6.5, 0 }, c{ 6, -2.6 }, c{ 6, -5 }, c{ 4, -6 }, c{ 1.3, -4.5 },
			c{ -1, -4.1 }, c{ -3.5, -3.5 }, c{ -7, -6 }, c{ -9, -4 }, c{ -9, -2 },
			c{ -6.5, 2 }, c{ -5, 4 }, c{ -3.2, 6 }, c{ -2, 5 }, c{ -3, 3 }
		};
		//interpolate points in btwn the fake points
		coords = interpolateComplex(v, 5);//5 points
		coords = scale(coords, 0.33);
		
		fourierSeries fourier = discreteFourierTransform(coords);
		fourier = sort(fourier);//sorts in order of amplitudes (radii)
		for (int i = 0; i < fourier.size(); i++) {
			float freq = fourier[i].freq;//angular vel
			float radius = fourier[i].amplitude;//radius
			float phase = fourier[i].phase;//initial angle
			addArrow(arrow{ radius, freq, phase, initP });
		}
		dt = train.size() / (2 * M_PI);//NEED THIS

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
		//train.push_back(arrow(0.5 + (rand() % 1), (rand() % 10) - 5, (rand() % 7)));//integer

		// adds random arrow with length from (0.5 -> 1.5) vel (-5 -> 5) & thta (0 -> 6.28 [2M_PI])//no 6.28
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
				gl::drawSolidCircle(ppm*Vec2f(coords[i].re + initP.X, coords[i].im + initP.Y), 5);
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
	ci::gl::Texture picture;
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
	mFont = Font("Arial", 45);//fixed custom font
	mTextureFont = gl::TextureFont::create(mFont);
	//picture = gl::Texture(loadImage(loadAsset("scotty.png")));

	d.initP = vec3(getWindowWidth() / (2 * ppm), getWindowHeight() / (2 * ppm));
	//d.fileInit();//first drawn 
	d.fourierInit();
	//d.init();
}
void FourierDrawApp::mouseDown(MouseEvent event) {
	if (event.isLeft()) {
		d.addRandom();
		int i = d.train.size()-1;//index for new arrow
		completeTxt.append(std::to_string(i + 1) + "  len: " + std::to_string(d.train[i].length) + "   vel: " + std::to_string(d.train[i].velocity) + "\n");//kinda long precision but owell
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
	gl::clear( Color(0, 0, 63/255.0) );//dark blue
	gl::color(1, 1, 1);

	d.draw();
	const vec3 initP{ 1, 1 };
	//gl::drawString(completeTxt, ppm*Vec2f(initP.X, initP.Y), Color(1, 1, 1), Font("Arial", 35));

	gl::drawString("FPS: ", Vec2f(getWindowWidth() - 250, 10), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(getAverageFps(), vec3(getWindowWidth() - 130, 10));

	gl::drawString("RefRt: ", Vec2f(getWindowWidth() - 250, 50), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(d.train.size()/(2*M_PI)*freq, vec3(getWindowWidth() - 130, 50));
}

CINDER_APP_NATIVE( FourierDrawApp, RendererGl )
