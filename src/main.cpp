#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <time.h>
#include <Main.h>
#include <Einstellungen.h>
#include <VenusLib.h>

// allgemeine Einstellungen ----------------------------
const char* ssidap = "AP-Venus";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

// Objekte ---------------------------------------------
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient mqttClient = PubSubClient(wifiClient);
Venus venus = Venus(1, Serial);
Einstellungen einst = Einstellungen(&server);

// Log -------------------------------------------------
const uint16 ll = 1000;                   // Loglänge
uint16 pos = 0;                           // Position erstes freies Zeichen
char log1[ll + 1] = {'\0'};

void logloeschen(int a){                  // a = benötigter Platz im Puffer, -1 = Puffer löschen, -2 = halben Puffer löschen
  if(a == -1){
      log1[0] = '\0';
      pos = 0;
  }else{
      if(a == -2) a = ll / 2;
      char* x = strstr(log1 + a, "\r\n") + 2;
      log1[0] = '\0';
      strcpy(log1, x);
      pos = strlen(log1);
  }
}

void addLog(const char *lo){
  char datum[36];
  char zeit[36];
  getDatumZeitStr(datum, zeit);
  uint16 ld = strlen(datum);
  uint16 lz = strlen(zeit);
  uint16 le = strlen(lo);
  uint16 lg = le + ld + lz + 4;
  if(pos + lg > ll)
    logloeschen(-2);                  // löscht die Hälfte des Puffers
  if(pos + lg < ll + 1){
    strcat(log1, datum);
    strcat(log1, " ");
    strcat(log1, zeit);
    strcat(log1, " ");
    strcat(log1, lo);
    strcat(log1, "\r\n");
    pos += lg;
  }
}

char* getLog(){
  return log1;
}

// Wifi Client -----------------------------------------
boolean apModus = false;

void setupWifi(){
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  if(einst.wlan){
    WiFi.begin(einst.ssid, einst.pwd);
    int i = 0;
    while(i < 60 && WiFi.status() != WL_CONNECTED){   //warten auf Wlan connect, langsames flackern der LED
      digitalWrite(LED_BUILTIN,HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN,LOW);
      delay(500);
      i++;
    }
  }
}

void setupAP(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssidap);
    for(int i = 0; i < 10; i++){                        //Accesspoint aktiviert, 10 mal schnelles flackern der LED
      digitalWrite(LED_BUILTIN,HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN,LOW);
      delay(50);
    }
    apModus = true;
  }
}

// Webserver -------------------------------------------
void dateiSenden(const char *dn, const char *typ = "text/html"){
  File datei = LittleFS.open(dn, "r");
  if(!datei){
    server.send(404, "text/plain", "Datei nicht gefunden.");
    return;
  }
  server.streamFile(datei, typ);
  datei.close();
}

void fehlerseite(){
  server.send(404, "text/plain", "Link wurde nicht gefunden!");
}

void hauptseite(){
  dateiSenden("/index.html");
}

void einstellungsmenue(){
  dateiSenden("/einst.html");
  einst.setEinst();
}

void einstellungAll(){
  dateiSenden("/einstAll.html");
}

void einstellungWl(){
  dateiSenden("/einstWl.html");
}

void einstellungMq(){
  dateiSenden("/einstMq.html");
}

void einstellungOt(){
  dateiSenden("/einstOt.html");
}

void einstellungDlUl(){
  dateiSenden("/einstDlUl.html");
}

void regSeite(){
  dateiSenden("/register.html");
}

void setReg(){
  if(server.hasArg("reg") && server.hasArg("wert")){
    int r = atoi(server.arg("reg").c_str());
    int w = atoi(server.arg("wert").c_str());
    boolean r32 = (server.hasArg("reg32") && server.arg("reg32").equals("on"))? true: false;
    if(r > 0){
      venus.setReg(r, r32, w);
      server.send(200, "application/json", venus.getRegJson(r, r32));
    }else{
      addLog("Fehler beim Register setzen.");
    }
  }
}

void getReg(){
  if(server.hasArg("reg")){
    int r = atoi(server.arg("reg").c_str());
    boolean r32 = (server.hasArg("reg32") && server.arg("reg32").equals("on"))? true: false;
    if(r > 0){
      server.send(200, "application/json", venus.getRegJson(r, r32));
    }else{
      addLog("Fehler bei der Registerabfrage.");
    }
  }
}

void scriptDatei(){
  dateiSenden("/script.js", "application/javascript");
}

void cssDatei(){
  dateiSenden("/einst.css", "text/css");
}

void sendeDaten(){
  server.send(200, "application/json", venus.json);
}

void sendeEinst(){
  server.send(200, "application/json", einst.json);
}

void sendeReg(){
  dateiSenden("/register.json", "application/json");
}

void sendeRegTab(){
  dateiSenden("/regTab.html");
}

void logdaten(){
  server.send(200, "text/plain", getLog());
}

void befehle(){
  server.send(200, "text/plain", "Ok");
//  if(server.hasArg("bef")){
//  }else 
  if(server.hasArg("rst")){
    server.client().stop();
    delay(100);
    ESP.restart();
  }
}

File datei; 
boolean upload(const char* d){
  boolean erg = false;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = (d && d[0] != '\0')? d: upload.filename;
    LittleFS.remove(filename);
    datei = LittleFS.open(filename, "w");
  }else if(upload.status == UPLOAD_FILE_WRITE){
    if(datei) datei.write(upload.buf, upload.currentSize);
  }else if(upload.status == UPLOAD_FILE_END){
    if(datei){
      datei.close();
      erg = true;
    }else{
      addLog("Upload fehlgeschlagen.");
    }
  }
  return erg;
}

void dateiUpload(){
  upload(NULL);
}

void einstUpload(){
  if(upload("einst.json"))
    einst.alle_einst_laden();
}

void regUpload(){
  if(upload("register.json"))
    venus.genRegister();
}

void setupWS(){
  server.onNotFound(fehlerseite);
  server.on("/",HTTP_GET, hauptseite);
  server.on("/",HTTP_POST, hauptseite);
  server.on("/einst",HTTP_POST, einstellungsmenue);
  server.on("/einstAll",HTTP_POST, einstellungAll);
  server.on("/einstWl",HTTP_POST, einstellungWl);
  server.on("/einstMq",HTTP_POST, einstellungMq);
  server.on("/einstOt",HTTP_POST, einstellungOt);
  server.on("/einstDlUl",HTTP_POST, einstellungDlUl);
  server.on("/register",HTTP_POST, regSeite);
  server.on("/setReg", setReg);
  server.on("/getReg", getReg);
  server.on("/script.js", scriptDatei);
  server.on("/einst.css", cssDatei);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/register.json", sendeReg);
  server.on("/regTab.html", sendeRegTab);
  server.on("/log", logdaten);
  server.on("/befehle",HTTP_GET, befehle);
  server.on("/upload",HTTP_POST, einstellungOt, dateiUpload);
  server.on("/uploadEinst",HTTP_POST, einstellungDlUl, einstUpload);
  server.on("/uploadReg",HTTP_POST, einstellungDlUl, regUpload);
  server.begin();
}

// Mqtt Client -------------------------------------------
void setWert(char* s, int* r, boolean* r32, int* w){
    char* s1 = strtok(s, "=");
    char* s2 = strtok(NULL, "=");
    if(s1 != NULL && s2 != NULL){
        if(strcmp(s1, "reg") == 0)
            *r = atoi(s2);
        else if(strcmp(s1, "reg32") == 0)
            *r32 = strcmp(s2, "on") == 0;
        else if(strcmp(s1, "wert") == 0)
            *w = atoi(s2);
    }
}

void callback(char* topic, byte* payload, unsigned int length){
  int i = strlen(einst.mqttTp.c_str());
  if(strncmp(topic, einst.mqttTp.c_str(), i) == 0 && strcmp(topic + i, "/Befehl") == 0){
    char pl[length + 1];
    pl[length] = '\0';
    memcpy(pl, payload, length);
//    if(strcmp(pl, "abc") == 0) ...
//    else
    char* b = strtok(pl, "?");
    if(b != NULL){
      int r = 0;
      int w = 0;
      boolean r32 = false;
      char* s[3] = {NULL, NULL, NULL};
      s[0] = strtok(NULL, "&");
      int i = 0;
      while(i < 3 && s[i] != NULL){
          i++;
          s[i] = strtok(NULL, "&");
      }
      i = 0;
      while(i < 3 && s[i] != NULL){
          setWert(s[i], &r, &r32, &w);
          i++;
      }
      if(strcmp(b, "getReg") == 0)
        mqttPub("Register", venus.getRegJson(r, r32));
      else if(strcmp(b, "setReg") == 0){
        venus.setReg(r, r32, w);
        mqttPub("Register", venus.getRegJson(r, r32));
      }
    }
  }
}

void setupMqtt(){
  mqttClient.setCallback(callback);
  reconnectMqtt();
}

void reconnectMqtt(){
  if(!apModus){
    mqttClient.setServer(einst.mqttIp.c_str(), einst.mqttPo.toInt());       // Mqtt Server Ip, Port
    int i = 0;
    while(i < 2 && !mqttClient.connected()){
      char id[19] = "ESP8266Client-";
      ltoa(random(0xffff), id + 14, HEX);
      boolean c = false;
      if(einst.mqttBe != "" && einst.mqttPw != "")
        c = mqttClient.connect(id, einst.mqttBe.c_str(), einst.mqttPw.c_str());
      else
        c = mqttClient.connect(id);
      if(c){
        mqttClient.subscribe((einst.mqttTp + "/Befehl").c_str());
      }else{
        delay(500);
      }
      i++;
    }
  }
}

void mqttPub(String topic, char* daten){
  if(!apModus){
    if(!mqttClient.connected()){
      reconnectMqtt();
    }
    if(mqttClient.connected()){
      mqttClient.publish((einst.mqttTp + "/" + topic).c_str(), daten);
    }
  }
}

void mqttPubSpontan(){
  if(einst.mqtt && einst.mqttSp)
    mqttPub("Daten", venus.json);
}

// NTP Zeitserver -------------------------------------------
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"
tm dat;

void setupNTP(){
  if(!apModus){
    configTime(MY_TZ, einst.ntzIp.c_str());
  }
}

void getZeit(){
  if(!apModus){
    time_t now;
    time(&now);
    localtime_r(&now, &dat);
  }
}

void getDatumZeitStr(char *datum, char *zeit){
  getZeit();
  sprintf(datum, "%02d.%02d.%04d", dat.tm_mday, dat.tm_mon + 1, dat.tm_year + 1900);
  sprintf(zeit, "%02d:%02d:%02d", dat.tm_hour, dat.tm_min, dat.tm_sec);
}

// Timer -------------------------------------------
unsigned long mqttPubZeit = 0;
unsigned long mqttPubInterval = 30000;                    // 30 Sekunden
unsigned long mDatenZeit = 0;
const unsigned long mDatenInterval = 240000;              // 4 Minuten
unsigned long testZeit = 0;
unsigned long testInterval = 30000;
boolean testB = false;
int testI = 0;

void timerRun(){
  unsigned long zeit = millis();
  if(mqttPubZeit > 0 && zeit - mqttPubZeit > mqttPubInterval){
    mqttPubZeit = zeit;
    if(mqttPubZeit == 0) mqttPubZeit = 1;
    mqttPub("Daten", venus.json);
  }
  if(testZeit > 0 && zeit - testZeit > testInterval){
    testZeit = zeit;
    if(testZeit == 0) testZeit = 1;
    char t[] = "SpeicherM02/Befehl";
    byte p[5] = {'l','a','d','e','n'};
    callback(t, p, 5);
  }
}

void setMqttPubTimer(){
  if(einst.mqttIv != 0){
    mqttPubInterval = einst.mqttIv * 1000;
  }
  if(einst.mqtt && mqttPubInterval > 0){
    mqttPubZeit = millis();
    if(mqttPubZeit == 0) mqttPubZeit = 1;
  }else{
    mqttPubZeit = 0;
  }
}

void setTestTimer(){
  testZeit = millis();
  if(testZeit == 0) testZeit = 1;
}

void setupTimer(){
  setMqttPubTimer();
//  setTestTimer();
}

// Venus -------------------------------------------
void senden(){
  digitalWrite(D1, HIGH);
}

void lesen(){
  digitalWrite(D1, LOW);
}

void neueDaten(){
  mqttPubSpontan();
}

void logEintrag(const char *s){
  addLog(s);
}

void setupVenus(){
   venus.callbackLesenSenden(lesen, senden);
   venus.callbackNeueDaten(neueDaten);
   venus.callbackDatumZeit(getDatumZeitStr);
   venus.callbackLogeintrag(logEintrag);
   venus.genRegister();
}

// OTA Update ----------------------------------------
void setupOta(){
  httpUpdater.setup(&server);
}

// Arduino -------------------------------------------
void setup(){
  pinMode(D1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.begin(115200, SERIAL_8N1);
  if(LittleFS.begin()){
    einst.alle_einst_laden();
    setupWifi();
  }
  setupAP();
  setupWS();
  setupMqtt();
  setupNTP();
  setupTimer();
  setupVenus();
  setupOta();
  addLog("Programm gestartet.");
}


void loop(){
  digitalWrite(LED_BUILTIN,LOW);      // LED leuchtet
  timerRun();
  yield();
  venus.run();
  yield();
  server.handleClient();
  yield();
  mqttClient.loop();
  yield();
  digitalWrite(LED_BUILTIN,HIGH);     // LED ist in der Pause aus
  delay(100);
}
