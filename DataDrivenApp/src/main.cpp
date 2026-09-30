#include "ofMain.h"
#include "ofApp.h"

int main() {
	// Larger window to fit the GUI, details card and 3-day forecast
	ofSetupOpenGL(1600, 1000, OF_WINDOW);
	ofRunApp(new ofApp());
}
