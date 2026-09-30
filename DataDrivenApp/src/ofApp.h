#pragma once

#include "ofMain.h"
#include "ofxGui.h"

// One day in the 3-day forecast
struct ForecastDay {
	std::string date;
	std::string condition;
	float maxC = 0, maxF = 0;
	float minC = 0, minF = 0;
	int chanceOfRain = 0;
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	// Networking
	void fetchWeatherData();
	void fetchCity(const std::string & city);
	void urlResponse(ofHttpResponse & response);

	// Favourite city quick buttons
	void onFav1();
	void onFav2();
	void onFav3();

	// Helpers
	std::string urlEncode(const std::string & s);
	std::string fmtTemp(float c, float f);
	std::string greeting();
	std::string aqiLabel(int index);

	// ---- API ----
	std::string apiKey = "3830dea557fd4dcbacf100858262409";

	// ---- Fonts ----
	ofTrueTypeFont titleFont, bodyFont, smallFont;

	// ---- GUI ----
	ofxPanel gui;
	ofxTextField locationInput;
	ofxButton searchBtn;
	ofxButton fav1Btn, fav2Btn, fav3Btn;
	ofxToggle useFahrenheit;
	ofxToggle windInKph;
	ofxToggle showAirQuality;
	ofxToggle showForecast;
	ofxToggle darkTheme;
	ofxToggle autoRefresh;
	ofxIntSlider refreshMins;

	// ---- Current weather ----
	std::string cityName = "--", country = "--", region;
	std::string conditionText = "--";
	std::string localTime, lastUpdated;
	float tempC = 0, tempF = 0;
	float feelsLikeC = 0, feelsLikeF = 0;
	int humidity = 0;
	float windMph = 0, windKph = 0;
	std::string windDir;
	float pressureMb = 0;
	float uv = 0;
	float visMiles = 0;
	int cloud = 0;
	int airQualityIndex = 0;
	bool hasData = false;

	// ---- Forecast ----
	std::vector<ForecastDay> forecast;

	// ---- Icon ----
	ofImage conditionIcon;
	bool hasIcon = false;

	// ---- State ----
	bool isLoading = false;
	bool hasError = false;
	std::string statusMessage = "Ready. Pick a city and press Get Forecast.";
	float lastFetchTime = 0;
};
