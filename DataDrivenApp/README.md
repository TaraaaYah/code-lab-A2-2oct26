# 🌦️ Tara-Yah Weather — Data Driven App

**Module:** CodeLab II
**Assignment:** Data Driven App
**Author:** Tara
**Built with:** C++ · openFrameworks · ofxGui · WeatherAPI.com

---

## About the App

Tara-Yah Weather is an interactive desktop weather dashboard I built in C++ using openFrameworks. It connects to the **WeatherAPI.com** REST API, downloads live weather data in JSON format, and displays it in a clear, customisable interface.

The user can search for any city in the world, switch between measurement units and themes, and view a 3-day forecast. The app opens on Manchester by default.

---

## Features

### Live weather data
- Current temperature and "feels like" temperature
- Weather condition with a downloaded icon
- Humidity, cloud cover and UV index
- Wind speed and direction
- Air pressure and visibility
- Air quality rating (Good → Hazardous)
- Local time and when the data was last updated

### 3-day forecast
- Daily high and low temperatures
- Weather condition for each day
- Chance of rain

### Interactive controls
| Control | What it does |
|---|---|
| City Search | Type any city name |
| Get Forecast | Fetches the weather for that city |
| Favourite buttons | One click for Manchester, Salford or London |
| Use Fahrenheit | Switches between °C and °F |
| Wind in km/h | Switches between mph and km/h |
| Show Air Quality | Shows or hides the air quality rating |
| Show 3-Day Forecast | Shows or hides the forecast cards |
| Dark Theme | Switches between dark and light mode |
| Auto Refresh | Updates the weather automatically |
| Refresh Every (mins) | Sets the auto-refresh time (1–60 minutes) |
| **Enter key** | Searches without clicking the button |

### Error handling
The coloured status bar shows what is happening:
- 🟠 **Orange** — loading data
- 🟢 **Green** — data loaded successfully
- 🔴 **Red** — something went wrong

It gives clear messages for an empty search box, a city that can't be found, an invalid API key, no internet connection, or unreadable data.

---

## How It Works

1. The user enters a city or clicks a favourite.
2. The app builds a request URL and sends it with `ofLoadURLAsync()`, so the window doesn't freeze while it waits.
3. WeatherAPI.com sends back the weather data as **JSON**.
4. `urlResponse()` checks the HTTP status code, then reads the JSON with `ofJson`.
5. The values are stored in variables, and a second request downloads the weather icon.
6. `draw()` displays everything on screen every frame, using the user's chosen units and theme.

---

## How to Run

### You will need
- **openFrameworks** (Windows / Visual Studio version)
- **Visual Studio** with the C++ desktop workload
- A free API key from [weatherapi.com](https://www.weatherapi.com/)

### Steps
1. Download or clone this folder into `openFrameworks/apps/myApps/`.
2. Open the **openFrameworks Project Generator**.
3. Import this folder, make sure **ofxGui** is ticked under Addons, and click **Update / Generate**.
4. Open the `.sln` file in Visual Studio.
5. In `src/ofApp.h`, check the API key is set:
   ```cpp
   std::string apiKey = "YOUR_API_KEY_HERE";
   ```
6. Press **F5** to build and run.

---

## Project Structure

```
DataDrivenApp/
├── src/
│   ├── main.cpp      → creates the 1600 × 1000 window
│   ├── ofApp.h       → class, variables and GUI controls
│   └── ofApp.cpp     → API request, JSON parsing and drawing
├── bin/data/         → assets folder
├── addons.make       → lists the ofxGui addon
├── .gitignore        → stops build files being uploaded
└── README.md
```

---

## Testing

| Test | Expected result |
|---|---|
| Search "Manchester" | Weather and forecast load, status bar turns green |
| Search "New York" (with a space) | Loads correctly |
| Leave the search box empty | Red message: input cannot be empty |
| Search "asdfghjk" | Red message: location not found |
| Wrong API key | Red message: check your API key |
| Turn off Wi-Fi and search | Red message: no internet connection |
| Toggle Fahrenheit, km/h, theme | Display updates instantly |
| Turn on Auto Refresh (1 min) | Countdown shows and data refreshes |

---

## References

- openFrameworks (2024) *openFrameworks documentation*. Available at: https://openframeworks.cc/documentation/
- WeatherAPI.com (2024) *API documentation*. Available at: https://www.weatherapi.com/docs/
- Lohmann, N. (2024) *JSON for Modern C++*. Available at: https://github.com/nlohmann/json
