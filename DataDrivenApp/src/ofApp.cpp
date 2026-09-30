#include "ofApp.h"
#include <sstream>
#include <iomanip>

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofRegisterURLNotification(this);

	titleFont.load(OF_TTF_SANS, 36);
	bodyFont.load(OF_TTF_SANS, 24);
	smallFont.load(OF_TTF_SANS, 18);

	ofxGuiSetDefaultWidth(500);
	ofxGuiSetDefaultHeight(45);

	gui.setup("TARA-YAH WEATHER");
	gui.add(locationInput.setup("City Search", "Manchester"));
	gui.add(searchBtn.setup("Get Forecast"));

	// Favourite cities - change these to any cities you like
	gui.add(fav1Btn.setup("Favourite: Manchester"));
	gui.add(fav2Btn.setup("Favourite: Salford"));
	gui.add(fav3Btn.setup("Favourite: London"));

	// Display options
	gui.add(useFahrenheit.setup("Use Fahrenheit", false));
	gui.add(windInKph.setup("Wind in km/h", false));
	gui.add(showAirQuality.setup("Show Air Quality", true));
	gui.add(showForecast.setup("Show 3-Day Forecast", true));
	gui.add(darkTheme.setup("Dark Theme", true));

	// Auto refresh
	gui.add(autoRefresh.setup("Auto Refresh", false));
	gui.add(refreshMins.setup("Refresh Every (mins)", 10, 1, 60));

	searchBtn.addListener(this, &ofApp::fetchWeatherData);
	fav1Btn.addListener(this, &ofApp::onFav1);
	fav2Btn.addListener(this, &ofApp::onFav2);
	fav3Btn.addListener(this, &ofApp::onFav3);

	// Load my home city on start-up
	fetchWeatherData();
}

//--------------------------------------------------------------
void ofApp::onFav1() { locationInput = std::string("Manchester"); fetchWeatherData(); }
void ofApp::onFav2() { locationInput = std::string("Salford");    fetchWeatherData(); }
void ofApp::onFav3() { locationInput = std::string("London");     fetchWeatherData(); }

//--------------------------------------------------------------
void ofApp::fetchWeatherData() {
	std::string city = locationInput;
	fetchCity(ofTrim(city));
}

void ofApp::fetchCity(const std::string & city) {
	if (city.empty()) {
		hasError = true;
		statusMessage = "Error: Input cannot be empty.";
		return;
	}

	isLoading = true;
	hasError = false;
	statusMessage = "Fetching weather for " + city + "...";
	lastFetchTime = ofGetElapsedTimef();

	// forecast.json returns current weather AND the forecast in one call
	std::string url = "https://api.weatherapi.com/v1/forecast.json?key=" + apiKey;
	url += "&q=" + urlEncode(city);
	url += "&days=3";
	url += showAirQuality ? "&aqi=yes" : "&aqi=no";
	url += "&alerts=no";

	ofLoadURLAsync(url, "weatherReq");
}

//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response) {

	// ---- Weather icon download ----
	if (response.request.name == "iconReq") {
		if (response.status == 200) {
			hasIcon = conditionIcon.load(response.data);
		}
		return;
	}

	if (response.request.name != "weatherReq") return;
	isLoading = false;

	if (response.status != 200) {
		hasError = true;
		if (response.status == 400 || response.status == 404)
			statusMessage = "Error: Location not found. Try another city.";
		else if (response.status == 401 || response.status == 403)
			statusMessage = "Error: Check your API key.";
		else if (response.status < 0)
			statusMessage = "Error: No internet connection.";
		else
			statusMessage = "API Error: HTTP Code " + ofToString(response.status);
		return;
	}

	try {
		ofJson json = ofJson::parse(response.data.getText());

		if (!json.contains("location") || !json.contains("current")) {
			hasError = true;
			statusMessage = "Error: Invalid JSON structure.";
			return;
		}

		auto & loc = json["location"];
		auto & cur = json["current"];

		cityName  = loc["name"].get<std::string>();
		region    = loc["region"].get<std::string>();
		country   = loc["country"].get<std::string>();
		localTime = loc["localtime"].get<std::string>();

		tempC       = cur["temp_c"].get<float>();
		tempF       = cur["temp_f"].get<float>();
		feelsLikeC  = cur["feelslike_c"].get<float>();
		feelsLikeF  = cur["feelslike_f"].get<float>();
		humidity    = cur["humidity"].get<int>();
		windMph     = cur["wind_mph"].get<float>();
		windKph     = cur["wind_kph"].get<float>();
		windDir     = cur["wind_dir"].get<std::string>();
		pressureMb  = cur["pressure_mb"].get<float>();
		uv          = cur["uv"].get<float>();
		visMiles    = cur["vis_miles"].get<float>();
		cloud       = cur["cloud"].get<int>();
		lastUpdated = cur["last_updated"].get<std::string>();
		conditionText = cur["condition"]["text"].get<std::string>();

		// Air quality (only present when aqi=yes)
		airQualityIndex = 0;
		if (cur.contains("air_quality") && cur["air_quality"].contains("us-epa-index")) {
			airQualityIndex = cur["air_quality"]["us-epa-index"].get<int>();
		}

		// 3-day forecast
		forecast.clear();
		if (json.contains("forecast")) {
			for (auto & d : json["forecast"]["forecastday"]) {
				ForecastDay fd;
				fd.date         = d["date"].get<std::string>();
				fd.maxC         = d["day"]["maxtemp_c"].get<float>();
				fd.maxF         = d["day"]["maxtemp_f"].get<float>();
				fd.minC         = d["day"]["mintemp_c"].get<float>();
				fd.minF         = d["day"]["mintemp_f"].get<float>();
				fd.chanceOfRain = d["day"]["daily_chance_of_rain"].get<int>();
				fd.condition    = d["day"]["condition"]["text"].get<std::string>();
				forecast.push_back(fd);
			}
		}

		// Download the condition icon (API gives "//cdn.weatherapi.com/...")
		std::string iconUrl = cur["condition"]["icon"].get<std::string>();
		if (ofIsStringInString(iconUrl, "64x64")) ofStringReplace(iconUrl, "64x64", "128x128");
		if (iconUrl.rfind("//", 0) == 0) iconUrl = "https:" + iconUrl;
		ofLoadURLAsync(iconUrl, "iconReq");

		hasData = true;
		hasError = false;
		statusMessage = "Data loaded at " + ofGetTimestampString("%H:%M");
	}
	catch (std::exception & e) {
		hasError = true;
		statusMessage = "Error: Could not read weather data.";
		ofLogError("urlResponse") << e.what();
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	// Auto refresh on a timer
	if (autoRefresh && !isLoading) {
		float interval = refreshMins * 60.0f;
		if (ofGetElapsedTimef() - lastFetchTime > interval) {
			fetchWeatherData();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	// ---- Theme colours ----
	ofColor bg   = darkTheme ? ofColor(25, 30, 40)    : ofColor(235, 240, 248);
	ofColor text = darkTheme ? ofColor(255)           : ofColor(30, 35, 45);
	ofColor sub  = darkTheme ? ofColor(170, 180, 200) : ofColor(90, 100, 120);
	ofColor card = darkTheme ? ofColor(40, 48, 64)    : ofColor(255);
	ofColor accent(90, 170, 255);
	ofBackground(bg);

	float marginX = 80.0f;
	float startY = 80.0f;

	// ---- Header ----
	ofSetColor(text);
	titleFont.drawString("WEATHER FORECAST", marginX, startY);
	ofSetColor(accent);
	smallFont.drawString(greeting() + ", Tara", marginX, startY + 40);

	// ---- GUI panel (left) ----
	float guiY = startY + 70.0f;
	gui.setPosition(marginX, guiY);
	gui.draw();

	// ---- Results card (right) ----
	float textX = marginX + 560.0f;
	float cardW = 920.0f;
	float cardH = 560.0f;

	ofSetColor(card);
	ofDrawRectRounded(textX - 20, guiY, cardW, cardH, 12);

	float currentY = guiY + 50.0f;
	float lineSpacing = 48.0f;

	// Location + icon
	ofSetColor(text);
	titleFont.drawString(cityName, textX, currentY);
	if (hasIcon) {
		ofSetColor(255);
		conditionIcon.draw(textX + cardW - 180, guiY + 10, 128, 128);
	}
	currentY += 36;
	ofSetColor(sub);
	std::string place = region.empty() ? country : region + ", " + country;
	smallFont.drawString(place, textX, currentY);
	currentY += 30;
	if (!localTime.empty()) smallFont.drawString("Local time: " + localTime, textX, currentY);
	currentY += lineSpacing;

	// Big temperature
	ofSetColor(text);
	titleFont.drawString(fmtTemp(tempC, tempF), textX, currentY);
	bodyFont.drawString(conditionText, textX + 200, currentY);
	currentY += lineSpacing + 10;

	// Two columns of details
	float col2 = textX + 440;
	std::string wind = windInKph ? ofToString(windKph, 0) + " km/h" : ofToString(windMph, 0) + " mph";

	bodyFont.drawString("Feels like: " + fmtTemp(feelsLikeC, feelsLikeF), textX, currentY);
	bodyFont.drawString("Humidity: " + ofToString(humidity) + "%", col2, currentY);
	currentY += lineSpacing;

	bodyFont.drawString("Wind: " + wind + " " + windDir, textX, currentY);
	bodyFont.drawString("Cloud: " + ofToString(cloud) + "%", col2, currentY);
	currentY += lineSpacing;

	bodyFont.drawString("Pressure: " + ofToString(pressureMb, 0) + " mb", textX, currentY);
	bodyFont.drawString("UV index: " + ofToString(uv, 1), col2, currentY);
	currentY += lineSpacing;

	bodyFont.drawString("Visibility: " + ofToString(visMiles, 0) + " miles", textX, currentY);
	if (showAirQuality && airQualityIndex > 0) {
		bodyFont.drawString("Air: " + aqiLabel(airQualityIndex), col2, currentY);
	}
	currentY += lineSpacing;

	if (!lastUpdated.empty()) {
		ofSetColor(sub);
		smallFont.drawString("API last updated: " + lastUpdated, textX, currentY);
	}
	currentY += lineSpacing;

	// ---- Status banner ----
	ofColor bannerColor = ofColor::darkGreen;
	if (isLoading) bannerColor = ofColor::orange;
	if (hasError)  bannerColor = ofColor::darkRed;

	std::string status = "Status: " + statusMessage;
	if (autoRefresh && !isLoading) {
		int secsLeft = (int)(refreshMins * 60 - (ofGetElapsedTimef() - lastFetchTime));
		status += "  (next refresh " + ofToString(std::max(0, secsLeft)) + "s)";
	}
	float statusW = smallFont.stringWidth(status) + 30.0f;
	float statusH = smallFont.stringHeight(status) + 20.0f;
	ofSetColor(bannerColor);
	ofDrawRectRounded(textX - 10, currentY - statusH + 8, statusW, statusH, 6);
	ofSetColor(255);
	smallFont.drawString(status, textX + 5, currentY);

	// ---- 3-day forecast cards ----
	if (showForecast && !forecast.empty()) {
		float fy = guiY + cardH + 30;
		float fw = 290, fh = 190, gap = 25;

		for (size_t i = 0; i < forecast.size(); i++) {
			auto & d = forecast[i];
			float fx = textX - 20 + i * (fw + gap);

			ofSetColor(card);
			ofDrawRectRounded(fx, fy, fw, fh, 12);

			// Day name, e.g. "Thu 01"
			std::string label = (i == 0) ? "Today" : d.date.substr(5);
			ofSetColor(accent);
			bodyFont.drawString(label, fx + 20, fy + 40);

			ofSetColor(text);
			bodyFont.drawString(fmtTemp(d.maxC, d.maxF) + " / " + fmtTemp(d.minC, d.minF), fx + 20, fy + 85);

			ofSetColor(sub);
			std::string cond = d.condition.size() > 22 ? d.condition.substr(0, 21) + "..." : d.condition;
			smallFont.drawString(cond, fx + 20, fy + 125);
			smallFont.drawString("Rain: " + ofToString(d.chanceOfRain) + "%", fx + 20, fy + 160);
		}
	}

	// ---- Footer hint ----
	ofSetColor(sub);
	smallFont.drawString("Tip: press ENTER to search", marginX, ofGetHeight() - 30);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_RETURN) fetchWeatherData();
}

//--------------------------------------------------------------
std::string ofApp::fmtTemp(float c, float f) {
	return useFahrenheit ? ofToString(f, 0) + "F" : ofToString(c, 0) + "C";
}

std::string ofApp::greeting() {
	int h = ofGetHours();
	if (h < 12) return "Good morning";
	if (h < 18) return "Good afternoon";
	return "Good evening";
}

std::string ofApp::aqiLabel(int index) {
	switch (index) {
		case 1: return "Good";
		case 2: return "Moderate";
		case 3: return "Unhealthy (sensitive)";
		case 4: return "Unhealthy";
		case 5: return "Very unhealthy";
		case 6: return "Hazardous";
		default: return "--";
	}
}

// Makes city names with spaces/accents safe for a URL (e.g. "New York")
std::string ofApp::urlEncode(const std::string & s) {
	std::ostringstream out;
	for (unsigned char c : s) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out << c;
		else out << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
	}
	return out.str();
}
