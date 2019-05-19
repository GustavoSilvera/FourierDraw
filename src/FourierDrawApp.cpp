#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include <vector>
#include <string.h>
#include <array>
#include "cinder/gl/TextureFont.h"

using namespace ci;
using namespace ci::app;
using namespace std;
#define pi 3.1415

float ppm = 100;//scale of how many pixels are in 1 SI Meter
Font mFont;//custom font for optimized drawing
gl::TextureFontRef mTextureFont;//custom opengl::texture

static float sqr(float x) { return x * x; }
static inline float toDeg(float rad) { return (rad * 180 / pi); }
static inline float toRad(float deg) { return (deg * pi / 180); }
static inline float clamp(float min, float max, float i) {
	if (i > min) {
		if (i < max) {
			return i;
		}
		return max;
	}
	return min;
}
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
private:
};

class arrow {
public:
	arrow(float L, float V, float T, vec3 p) : length(L), velocity(V), theta(T), pos(p) {}
	arrow(float L, float V, float T) : length(L), velocity(V), theta(T) {}
	arrow() : length(1), velocity(0), theta(pi/2) {}
	
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

class drawing {
public:
	std::vector<arrow> train;
	std::vector<std::pair<vec3, vec3>> path;
	bool penDown = false;
	int last;//index of final arrow
	void init(vec3 initPos) {
		train.push_back(arrow(1, 1, 0, initPos));

	}
	void addRandom() {
		train.push_back(arrow(0.5 + (rand() % 100) / 100.0, (rand() % 1000) / 100.0 - 5, (rand() % 628) / 100.0));//floating point
		//train.push_back(arrow(0.5 + (rand() % 1), (rand() % 10) - 5, (rand() % 7)));//integer

		// adds random arrow with length from (0.5 -> 1.5) vel (-5 -> 5) & thta (0 -> 6.28 [2pi])//no 6.28
	}
	void update(float dt, int freq) {
		for (int j = 0; j < freq; j++) {
			last = train.size() - 1;
			for (int i = 0; i < last + 1; i++) {
				train[i].theta += train[i].velocity / dt;//updates position over time
				if (i > 0) train[i].pos = train[i - 1].tip();//ensures all positions are tip to tip
			}
			path.push_back(std::pair<vec3, vec3>(tartan(path.size()), train[last].tip()));
		}
	}
	void penToggle() {
		penDown = !penDown;//to draw or not to draw
	}
	vec3 tartan(int i) {//from 1 to len
		//== 100, 50, 50, 35, 10 == 245
		std::vector<int> div = { 50, 100, 15, 35, 50 };
		int colour = i % sum(div);
		std::vector<vec3> colours = { vec3(0, 0, 100), vec3(0, 140, 0), vec3(200, 200, 0),vec3(255, 0, 0), vec3(0, 100, 0) };
		for (int j = 0; j < div.size(); j++) {
			if (colour < div[j]) return colours[j].times(1/255.0);
			colour -= div[j];
		}
		//should never happen
		return vec3{ 1, 1, 1 };
	}
	void draw() {
		const float width = 0.03;//3cm constant width for all arrows
		for (int i = 0; i < train.size(); i++) {
			glPushMatrix();//rotation
			gl::translate(Vec3f(ppm*train[i].pos.X, ppm*train[i].pos.Y, 0));//origin of rotation
			gl::rotate(Vec3f(0, 0, toDeg(train[i].theta)));//3rd input used for 2D rotation (angle)
			gl::color(1, 1, 1);//cool colour transition (neat maths)
			gl::drawSolidRect(Area(//final transformation
				ppm*Vec2f(0, - width/2), //([X] no change,[Y] half of height (center))
				ppm*Vec2f(train[i].length, width/2)//([X] add radius,[Y] half of height (center))
			));
			const float triScale = 0.09;//scale of size of triangle
			float triHeight = 1.732*triScale*train[i].length;//achieved thru height of triangle (L = 2*C*x*cos(30°)), with triscale(c) and length(x)
			gl::drawSolidTriangle(
				ppm*Vec2f(train[i].length - triHeight, -width / 2 - triScale * train[i].length),
				ppm*Vec2f(train[i].length - triHeight,  width / 2 + triScale * train[i].length),
				ppm*Vec2f(train[i].length, 0)
			);
			glPopMatrix();//end of rotation code
			
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
	void labelList(class drawing *d);
	void draw();
	drawing d;
	float dt = 60;// 60hz refresh rate
	int freq = 1;
	std::vector<std::pair<string, string>> s;
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
void FourierDrawApp::setup()
{
	srand(time(NULL));//seeds random number generator
	mFont = Font("Times", 45);//fixed custom font
	mTextureFont = gl::TextureFont::create(mFont);
	d.init(vec3(getWindowWidth()/(2*ppm), getWindowHeight()/(2*ppm)));//first drawn @ 5m,8m
	s.push_back(std::pair<string, string>("1", "1"));

}
void FourierDrawApp::mouseDown(MouseEvent event) {
	if (event.isLeft()) {
		d.addRandom();
		s.push_back(std::pair<string, string> (std::to_string(d.train[d.last].length), std::to_string(d.train[d.last].velocity)));
	}
	if (event.isRight()) {
		d.path.clear();//clear current path
		d.penToggle();
	}
}
void FourierDrawApp::keyDown(KeyEvent event) {
	if (event.getCode() == KeyEvent::KEY_UP) freq += 1;//makes everything a bit faster
	if (event.getCode() == KeyEvent::KEY_DOWN) freq -= 1;//makes everything a bit slower
	if (event.getCode() == KeyEvent::KEY_RIGHT) ppm += 2;//makes everything a bit faster
	if (event.getCode() == KeyEvent::KEY_LEFT) ppm -= 2;//makes everything a bit slower
}
void FourierDrawApp::update()
{
	d.update(dt, freq);
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
void FourierDrawApp::labelList(class drawing *d) {//***CAN EASILY OPTIMIZE BC EVERYTHING's CONSTANT:: DONT NEED REFRESH
	const vec3 initP(1, 1);
	const float verticalDiff = 0.5;//meters apart vertically
	const float horizontalDiff = 0.5;//meters apart vertically
	
	int i = 0;
	for (arrow& a : d->train) {
		float dY = verticalDiff * i;//increment y position for each button based off index
		gl::drawString(std::to_string(i+1), ppm*Vec2f(initP.X - horizontalDiff, initP.Y + dY), Color(1, 1, 1), Font("Times", 35));
		gl::drawString("len:", ppm*Vec2f(initP.X, initP.Y + dY), Color(1, 1, 1), Font("Times", 35));
		const float rightX = initP.X + horizontalDiff;//slightly shifted to not affect the string
		gl::drawString(s[i].first, ppm*Vec2f(rightX, initP.Y + dY), Color(1, 1, 1), Font("Times", 35));
		gl::drawString("vel:", ppm*Vec2f(rightX + 2*horizontalDiff, initP.Y + dY), Color(1, 1, 1), Font("Times", 35));
		gl::drawString(s[i].second, ppm*Vec2f(rightX + 3 * horizontalDiff, initP.Y + dY), Color(1, 1, 1), Font("Times", 35));
		++i;
	}
	gl::color(1, 1, 1);
}
void FourierDrawApp::draw()
{
	gl::enableAlphaBlending();//good for transparent images
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::color(1, 1, 1);

	d.draw();
	labelList(&d);

	gl::drawString("FPS: ", Vec2f(getWindowWidth() - 250, 10), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(getAverageFps(), vec3(getWindowWidth() - 130, 10));

	gl::drawString("RefRt: ", Vec2f(getWindowWidth() - 250, 50), Color(0, 1, 0), Font("Arial", 45));
	drawFontText(dt*freq, vec3(getWindowWidth() - 130, 50));
}

CINDER_APP_NATIVE( FourierDrawApp, RendererGl )
