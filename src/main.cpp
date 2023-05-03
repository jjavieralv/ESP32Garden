//##WIFI
  #if defined(ESP32)
    #include <WiFiMulti.h>
    WiFiMulti wifiMulti;
  #elif defined(ESP8266)
    #include <ESP8266WiFiMulti.h>
    ESP8266WiFiMulti wifiMulti;
  #endif

//## INFLUXDB
  //###CONFIG
    #include "configs.h"
    #ifndef CONFIGS_H //You could create a file called with same structure and add it to gitignore to not upload it to git
      #define CONFIGS_H
      #define WIFI_SSID ""
      #define WIFI_PASSWORD ""
      #define INFLUXDB_URL ""
      #define INFLUXDB_TOKEN ""
      #define INFLUXDB_ORG ""
      #define INFLUXDB_BUCKET ""
      // Time zone info https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
      #define TZ_INFO ""
      #define DEVICE ""
    #endif
  //###DEPENDENCIES
    #include <InfluxDbClient.h>
    #include <InfluxDbCloud.h>
  //###INITIALIZERS
    // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
    InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
    // Declare Data points with measurements https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/blob/master/src/Point.h
    Point sensor("wifi_status");
    Point weather("weather");

//##SENSORS
  //###DHT11
    //####CONFIG
      String dht_name="sensor_dht_";
      int dht_pin[]={4};
      #define DHTTYPE DHT11
    //####DEPENDENCIES
      #include "DHT.h"
  //###MOISURE
    //####CONFIG
      String moisure_name="sensor_moisture_";
      int moisure_pin[]={33,32,35,34};

//##PUMPS
  //###CONFIG
    int pump_pin[]={19,18,5,17};
    int pump_state[] = {1,1,1,1};

//##WIFI
  //###CONFIG
    IPAddress local_IP(192, 168, 1, 40);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8);
    IPAddress secondaryDNS(8, 8, 4, 4);

//##WEB
  //###CONFIG
    String header;
    WiFiServer server(80);
  
//##OTA
  //###CONFIG
    #define OTA_PORT 1000
  //###DEPENDENCIES
    #include <ESPAsyncWebServer.h>
    #include <AsyncElegantOTA.h>
    AsyncWebServer ota(OTA_PORT);


//##MISC
  //###DEBUG
    //select whether you want to send the data to the InfluxDB servers or just display it on the monitor without sending it to InfluxDB
    #define SEND_TO_INFLUX false
    //time since program starts
    unsigned long time_up=0;

void setup() {
  //config Serial baudrate
  Serial.begin(115200);

  //set PUMPS pin mode
  for (int i = 0; i < sizeof(pump_pin)/sizeof(int); i++)
  {
    pinMode(pump_pin[i],OUTPUT);
    digitalWrite(pump_pin[i], HIGH);
  }

  //config wifi and check
   if (!WiFi.config(local_IP, gateway, subnet, primaryDNS,secondaryDNS)) {
     Serial.println("STA Failed to configure");
   }
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();

  //sync clocks (data and certificates)
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection with InfluxDB
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //##WIFI
  server.begin();

  //##OTA
  ota.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/update");
  });

  AsyncElegantOTA.begin(&ota);
  ota.begin();
}

void point_wifi_setup(String device){
  sensor.clearTags();
  sensor.clearFields();
  sensor.addTag("device", device);
  sensor.addTag("SSID", WiFi.SSID());
  sensor.addField("rssi", WiFi.RSSI());

  Serial.println(sensor.toLineProtocol());
  if(SEND_TO_INFLUX){
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}

void point_weather_sensor_DHT(String device,String sensor_name,int pin){
  weather.clearTags();
  weather.clearFields();
  DHT dht(pin, DHTTYPE);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }else{
    weather.addTag("device",device);
    weather.addTag("name",sensor_name);
    weather.addTag("type","DHT11");
    weather.addField("humidity",h);
    weather.addField("temperature",t);
    Serial.println(weather.toLineProtocol());
  }
  if(SEND_TO_INFLUX){
    if (!client.writePoint(weather)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}

void point_weather_sensor_moisture(String device,String sensor_name,int pin){
  weather.clearTags();
  weather.clearFields();
  weather.addTag("device", device);
  weather.addTag("name", sensor_name);
  weather.addTag("type","moisturev2");
  weather.addField("moisture", analogRead(pin));

  Serial.println(weather.toLineProtocol());

  if(SEND_TO_INFLUX){
    if (!client.writePoint(weather)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }

}

void web_page(){
  WiFiClient client = server.available();   // Listen for incoming clients
  String aux="";

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /pump0/on") >= 0) {
              Serial.println("pump0 on");
              pump_state[0] = 1;
              digitalWrite(pump_pin[0], HIGH);
            } else if (header.indexOf("GET /pump0/off") >= 0) {
              Serial.println("pump0 off");
              pump_state[0] = 0;
              digitalWrite(pump_pin[0], LOW);
            } else if (header.indexOf("GET /pump1/on") >= 0) {
              Serial.println("pump1 on");
              pump_state[1] = 1;
              digitalWrite(pump_pin[1], HIGH);
            } else if (header.indexOf("GET /pump1/off") >= 0) {
              Serial.println("pump1 off");
              pump_state[1] = 0;
              digitalWrite(pump_pin[1], LOW);
            }  else if (header.indexOf("GET /pump2/on") >= 0) {
              Serial.println("pump2 on");
              pump_state[2] = 1;
              digitalWrite(pump_pin[2], HIGH);
            } else if (header.indexOf("GET /pump2/off") >= 0) {
              Serial.println("pump2 off");
              pump_state[2] = 0;
              digitalWrite(pump_pin[2], LOW);
            } else if (header.indexOf("GET /pump3/on") >= 0) {
              Serial.println("pump3 on");
              pump_state[3] = 1;
              digitalWrite(pump_pin[3], HIGH);
            } else if (header.indexOf("GET /pump3/off") >= 0) {
              Serial.println("pump3 off");
              pump_state[3] = 0;
              digitalWrite(pump_pin[3], LOW);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32Garden</h1>");

            for (int i = 0; i < sizeof(pump_pin)/sizeof(int); i++)
            {
              aux = !pump_state[i] ? "ON" : "OFF";
              client.println("<p>PUMP" + String(i) + "- State " + aux + "</p>");
              if (!pump_state[i]) {
                client.println("<p><a href=\"/pump" + String(i) + "/on\"><button class=\"button\">ON</button></a></p>");               
              } else {
                client.println("<p><a href=\"/pump" + String(i) + "/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
            }
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    header = "";
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void sensors_to_influx(){
  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED){
    Serial.println("Wifi connection lost");
  }else{
    point_wifi_setup(DEVICE);
    //read the value for each sensor in the specific array indicated and publish it
    for (int i = 0; i < sizeof(dht_pin)/sizeof(int); i++) {point_weather_sensor_DHT(DEVICE, dht_name+String(i), dht_pin[i] );}
    for (int i = 0; i < sizeof(moisure_pin)/sizeof(int); i++ ) point_weather_sensor_moisture(DEVICE, moisure_name+String(i), moisure_pin[i]);
  }
}

void loop(){
  web_page();
  /*manage sensors each time METRIC_PERIOD is smaller than time passed since last sensor management. This is used to be able to interact
  with the webpage without being blocked by a big delay. This should be managed by coretask but it "probably" will be added in futher versions :)
  */ 
  if (millis() - time_up > METRIC_PERIOD){
    Serial.println("Sensor readings");
    time_up = millis();
    sensors_to_influx();   
    delay(100);
  }
}
