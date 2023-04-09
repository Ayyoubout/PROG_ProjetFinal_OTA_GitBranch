#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define SEALEVELPRESSURE_HPA (1013.25)


// Remplacez par vos identifiants WiFi
const char* ssid = "Rogers8443";
const char* password = "connect8443";

unsigned long previousMillis = 0;
const unsigned long interval = 3000; // Save data every 3000 milliseconds (3 seconds)

const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>

    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 WEATHER STATION</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha1/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-KyZXEAg3QhqLMpG8r+Knujsl7/1L_dstPt3HV5HzF6Gvk/e3SHT+5wD1Kq9X5D5z" crossorigin="anonymous">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.3/font/bootstrap-icons.css">
   <link rel="stylesheet" href="css/esp.css">

 
</head>
<body>
  <!--Insertion du container globale -->
    <div class="container d-flex justify-content-center">
        <div class="row">
            <div class="col-12">
                <h1>ESP8266 WEATHER STATION</h1>
           </div>
        </div>

        <div class="row">
            <div class="col"><i class="bi bi-thermometer-half temp-icon"></i></div>
            <div class="col"><h2 class="meteo temperature"> </h2></div> <!--Insertion du titre température via JS -->
            <div class="col"><h2><span class="valueT"></span></h2></div>  <!--Insertion de la valeur température via Js --> 
        </div>
        
        <div class="row">
            <div class="col"><i class="bi bi-cloud-drizzle humid-icon"></i></div>
            <div class="col"><h2 class="humidity"> </h2></div>
            <div class="col"><h2><span class="valueH"></span></h2></div>   
        </div>

        <div class="row">
            <div class="col"><i class="bi bi-clock-history pressure-icon"></i></div>
            <div class="col"><h2 class="pressure"> </h2></div>
            <div class="col"><h2><span class="valueP"></span></h2></div>   
        </div>

        <div class="row">
            <div class="col"><i class="bi bi-triangle altitude-icon"></i></div>
            <div class="col"><h2 class="altitude"> </h2></div>
            <div class="col"><h2><span class="valueA"></span></h2></div>   
        </div>

    </div>
    <script src="js/esp.js"></script>
</body>
</html>
)rawliteral";

const char* css_content = R"rawliteral(
    .temp-icon {
  font-size: 48px; 
  color: #d2891c; 
  list-style: none;
}

.humid-icon {
  font-size: 48px; 
  color: #5a8cc9; 
}

.pressure-icon {
  font-size: 48px; 
  color: #22a12f; 
}

.altitude-icon {
  
  font-size: 48px; 
  color: #d21cc0; 
}

body {

  font-family: "Gill Sans", "Gill Sans MT", Calibri, "Trebuchet MS", sans-serif;
  
  display: inline-block;
  text-align: center;
  display: flex;
  justify-content: center;
  align-items: center;
  
}

h1 {
  text-align: center;
  font-size: 45px;
}
h2 {
  display: inline-block;
  font-size: 25px;
  padding-left: 35px;
}



.valueT {
  padding-left: 25px;
  color: #d2891c;
}
.valueH {
  padding-left: 55px;
  color: #5a8cc9;
}
.valueP {
  padding-left: 50px;
  color: #22a12f;
}
.valueA {
  padding-left: 50px;
  color: #d21cc0;
}
.col{
  display: inline-block;
}

@media (max-width: 576px) { 
  
  body{
    text-align: left;
    margin: 20px;
	
    
  }

  h1 {
    text-align: center;
    font-size: 25px;
    margin: 50px auto 30px;
  }
  h2 {
    font-size: 15px;
    display: inline-block;
    padding: 10px;
  }
  span{
    font-size: 35px;
    margin-right: -20px;
  }
  
 }


)rawliteral";

const char* js_content = R"rawliteral(
   
    // Changez l'URL pour pointer vers la route /data de votre serveur web Arduino
const url = window.location.protocol + '//' + window.location.hostname + '/data';

function fetchData() {
  fetch(url)
    .then((response) => {
      if (!response.ok) {
        throw new Error(`Erreur HTTP ${response.status}`);
      }
      return response.json();
    })
    .then((data) => {
      // Accédez directement aux propriétés de l'objet JSON
      const temperatureValeur = data.temperature;
      const humidityValeur = data.humidity;
      const pressureValeur = data.pressure;
      const altitudeValeur = data.altitude;

      // Mettez à jour les éléments HTML avec les nouvelles valeurs
      document.querySelector(".meteo").textContent = "Température";
      document.querySelector(
        ".valueT"
      ).innerHTML = `${temperatureValeur.toFixed(2)}<sup>°C</sup>`;

      document.querySelector(".humidity").textContent = "Humidité";
      document.querySelector(
        ".valueH"
      ).innerHTML = `${humidityValeur.toFixed(2)}<sup>%</sup>`;

      document.querySelector(".pressure").textContent = "Pression";
      document.querySelector(
        ".valueP"
      ).innerHTML = `${pressureValeur.toFixed(2)}<sup>hPa</sup>`;

      document.querySelector(".altitude").textContent = "Altitude";
      document.querySelector(
        ".valueA"
      ).innerHTML = `${altitudeValeur.toFixed(2)}<sup>m</sup>`;
    })
    .catch((error) => {
      console.error("Erreur lors de la récupération des données JSON:", error);
    });
}

// Appelez la fonction fetchData immédiatement pour charger les données initiales
fetchData();

// Mettez à jour les données toutes les 3 secondes
setInterval(fetchData, 3000);

  
)rawliteral";




// Créez une instance de BME280
Adafruit_BME280 bme;

// Créez une instance de serveur web asynchrone
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);

  

  // Initialisez le capteur BME280
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Connectez l'ESP32 au réseau WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  // Servez la page HTML, CSS et JavaScript
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/html", index_html);
});

server.on("/css/esp.css", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/css", css_content);
});

server.on("/js/esp.js", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "application/javascript", js_content);
});


  // Créez une route pour envoyer les données du capteur au format JSON
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temperature\": " + String(bme.readTemperature()) + ",";
    json += "\"humidity\": " + String(bme.readHumidity()) + ",";
    json += "\"pressure\": " + String(bme.readPressure() / 100.0F) + ",";
    json += "\"altitude\": " + String(bme.readAltitude(SEALEVELPRESSURE_HPA));
    json += "}";

    
    request->send(200, "application/json", json);
  });
	AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  // Démarrez le serveur web
  server.begin();
}

void loop() {
  // branch teste   
  // Le serveur web asynchrone gère les requêtes sans blocage
  // Save data to data.json periodically
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
  }
}


