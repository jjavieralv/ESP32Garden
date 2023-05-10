// ##WIFI
#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#endif

// ## INFLUXDB
// ###CONFIG
#include "configs.h"
#ifndef CONFIGS_H // You could create a file called with same structure and add it to gitignore to not upload it to git
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
// ###DEPENDENCIES
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
// ###INITIALIZERS
//  Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// Declare Data points with measurements https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/blob/master/src/Point.h
Point sensor("wifi_status");
Point weather("weather");

// ##SENSORS
// ###DHT11
// ####CONFIG
String dht_name = "sensor_dht_";
int dht_pin[] = {4};
#define DHTTYPE DHT11
// ####DEPENDENCIES
#include "DHT.h"
// ###MOISURE
// ####CONFIG
String moisure_name = "sensor_moisture_";
int moisure_pin[] = {33, 32, 35, 34};

// ##PUMPS
// ###CONFIG
int pump_pin[] = {19, 18, 5, 17};
// 1 is pump on, 0 is pump off
int pump_state[sizeof(pump_pin)/sizeof(int)];
String pump_name="pump";

// ##WIFI
// ###CONFIG
IPAddress local_IP(192, 168, 1, 40);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// ##mDNS
// ### CONFIG
const char* dns_device_name="esp32garden";
const char* dns_service_name[]={"webpage","esp32ota"};
const char* dns_protocol[]={"tcp","tcp"};
const uint16_t dns_port[]={80,1000};
// ### DEPENDENCIES
#include <ESPmDNS.h>

// ##WEB
// ###CONFIG
String header;
WiFiServer server(80);

// ##OTA
// ###CONFIG
#define OTA_PORT 1000
// ###DEPENDENCIES
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer ota(OTA_PORT);

// ##MISC
// ###DEBUG
// select whether you want to send the data to the InfluxDB servers or just display it on the monitor without sending it to InfluxDB
#define SEND_TO_INFLUX false
// ###TIME MANAGEMENT
// time since program starts
unsigned long time_up = 0;
// ###ERROR MANAGEMENT
int error_count = 0;
#define MAX_ERROR_TO_RESTART 3


void point_wifi_setup(String device)
{
  sensor.clearTags();
  sensor.clearFields();
  sensor.addTag("device", device);
  sensor.addTag("SSID", WiFi.SSID());
  sensor.addField("rssi", WiFi.RSSI());

  Serial.println(sensor.toLineProtocol());
  if (SEND_TO_INFLUX)
  {
    if (!client.writePoint(sensor))
    {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}

void wifi_show_variables(){
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
}

void point_weather_sensor_DHT(String device, String sensor_name, int pin){
  weather.clearTags();
  weather.clearFields();
  DHT dht(pin, DHTTYPE);
  float h = dht.readHumidity();
  //delay(2000);
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)){
    Serial.println(F("Failed to read from DHT sensor!"));
  }else{
    weather.addTag("device", device);
    weather.addTag("name", sensor_name);
    weather.addTag("type", "DHT11");
    weather.addField("humidity", h);
    weather.addField("temperature", t);
    Serial.println(weather.toLineProtocol());
    if (SEND_TO_INFLUX)
    {
      if (!client.writePoint(weather))
      {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
      }
    }
  }
}

void point_weather_sensor_moisture(String device, String sensor_name, int pin)
{
  weather.clearTags();
  weather.clearFields();
  weather.addTag("device", device);
  weather.addTag("name", sensor_name);
  weather.addTag("type", "moisturev2");
  weather.addField("moisture", analogRead(pin));

  Serial.println(weather.toLineProtocol());

  if (SEND_TO_INFLUX)
  {
    if (!client.writePoint(weather))
    {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}

void web_page()
{
  WiFiClient client = server.available(); // Listen for incoming clients
  String aux = "";

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0){
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            //check if the get ask to update some pump status
            String tmp_route_first="GET /"+pump_name;
            String tmp_route="";

            if (header.indexOf(tmp_route_first) >= 0){
              //find where number starts
              //Where "GET /variable" starts + number of letters of "GET /"(5) + variable.length
              int start_index_number=header.indexOf(tmp_route_first) + 5 + pump_name.length();
              //find wherere number ends
              int end_index_number=header.indexOf("/",start_index_number);
              //get number and convert it to int
              int index_number=header.substring(start_index_number,end_index_number).toInt();
              Serial.print(pump_name);Serial.print(index_number);
              Serial.print("\n state before:");Serial.println(pump_state[index_number]);

              tmp_route=tmp_route_first+index_number+"/on";
              if( header.indexOf(tmp_route) >= 0){
                Serial.println(" on");
                pump_state[index_number] = 1;
                digitalWrite(pump_pin[index_number], LOW);
  
              }
              tmp_route=tmp_route_first+index_number+"/off";
              if(header.indexOf(tmp_route) >= 0){
                Serial.println(" off");
                pump_state[index_number] = 0;
                digitalWrite(pump_pin[index_number], HIGH);
              }
              Serial.print("\n state updated:");
              Serial.println(pump_state[index_number]);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            //green button
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            //Grey button
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32Garden</h1>");

            for (int i = 0; i < sizeof(pump_pin) / sizeof(int); i++){
              aux = pump_state[i] ? "ON" : "OFF";
              client.println("<p>PUMP" + String(i) + "- State " + aux + "</p>");
              if (pump_state[i]){
                client.println("<p><a href=\"/pump" + String(i) + "/off\"><button class=\"button\">ON</button></a></p>");
              }else{
                client.println("<p><a href=\"/pump" + String(i) + "/on\"><button class=\"button button2\">OFF</button></a></p>");
              }
            }

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
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

void restart_device_check(){
  if (error_count >= MAX_ERROR_TO_RESTART){
    ESP.restart();
  }
}

void sensors_to_influx(){
  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("Wifi connection lost");
    error_count++;
  }
  else
  {
    point_wifi_setup(DEVICE);
    // read the value for each sensor in the specific array indicated and publish it
    for (int i = 0; i < sizeof(dht_pin) / sizeof(int); i++)
    {
      point_weather_sensor_DHT(DEVICE, dht_name + String(i), dht_pin[i]);
    }
    for (int i = 0; i < sizeof(moisure_pin) / sizeof(int); i++)
      point_weather_sensor_moisture(DEVICE, moisure_name + String(i), moisure_pin[i]);
  }
}

void check_influx_connectivity(){
  // Check server connection with InfluxDB
  for (int i = 0; i < 3; i++){
    if (client.validateConnection()){
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
      i=10;
    }
    else{
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
      error_count++;
    }
  }
}

void wifi_config_and_connect(){
  // config wifi and check 
  for (int i = 0; i < 3; i++)
  {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
      Serial.println("STA Failed to configure");
      error_count++;
    }else{
      break;
    }
  }
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("WiFi connected.");  
}

void wifi_start_server(){
  server.begin();
}

void ota_start_service(){
  ota.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
       { request->redirect("/update"); });

  AsyncElegantOTA.begin(&ota);
  ota.begin();
}

void mdns_server_start(){
  if(!MDNS.begin( dns_device_name )) {
   Serial.println("Error starting mDNS");
   error_count++;
  }else{
    Serial.println("mDNS working");
    //add all services in array
    for (int i = 0; i < sizeof(dns_port)/sizeof(int); i++){
       MDNS.addService(dns_service_name[i], dns_protocol[i], dns_port[i]); 
    }
  }
}

void pumps_initialize(){
  // set PUMPS pin mode
  for (int i = 0; i < sizeof(pump_pin) / sizeof(int); i++)
  {
    pinMode(pump_pin[i], OUTPUT);
    digitalWrite(pump_pin[i], HIGH);
    pump_state[i]=0;
  }
}
void setup(){
  // config Serial baudrate
  Serial.begin(115200);
  pumps_initialize();
  wifi_config_and_connect();
  wifi_show_variables();

  // sync clocks (data and certificates)
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  check_influx_connectivity();

  //##Start web servers. Web and OTA
  wifi_start_server();
  ota_start_service();

  // start mDNS server 
  mdns_server_start();

}


void loop()
{
  restart_device_check();
  web_page();
  
  /*manage sensors each time METRIC_PERIOD is smaller than time passed since last sensor management. This is used to be able to interact
  with the webpage without being blocked by a big delay. This should be managed by coretask but it "probably" will be added in futher versions :)
  */
  if (millis() - time_up > METRIC_PERIOD)
  {
    Serial.println("Sensor readings");
    time_up = millis();
    //sensors_to_influx();
    delay(100);
  }
}
