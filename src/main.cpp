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
  Serial.println(log1);
}

char* getLog(){
  return log1;
}

// Json -----------------------------------------
char json[200] = {'\0'};

 void generiereJson(Daten daten){
//   char datum[36];
//   char zeit[36];
//   getDatumZeitStr(datum, zeit);
//   char sp[5] = {'\0'};
//   dtostrf(daten.spannung, 3, 1, sp);
//   char st[5] = {'\0'};
//   dtostrf(daten.stromakku, 3, 1, st);
//   sprintf(json, "{\"Spannung\":%s,\"Ladezustand\":%d,\"StromAkku\":%s,\"Typ\":%d",
//       sp, daten.soc, st, daten.typ);
//   char s[150] = {'\0'};
//   if(daten.typ == 2){
//       dtostrf(daten.strompv, 3, 1, st);
//       sprintf(s, ",\"StromPV\":%s,\"Temperatur\":%d", st, daten.temperatur);
//   }else{
//       strcat(s, ",\"StromPV\":0,\"Temperatur\":0");
//   }
//   strcat(json, s);
//   sprintf(s, ",\"Datum\":\"%s\",\"Zeit\":\"%s\",\"Laden\":%s,\"Entladen\":%s",
//       datum, zeit, (daten.laden)? "\"ein\"": "\"aus\"", (daten.entladen)? "\"ein\"": "\"aus\"");
//   strcat(json, s);
//   strcat(json, "}");
 }

char* getJson(){
  return json;
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

void hauptseiteBef(){
   if(server.hasArg("bef")){
  //   if(server.arg("bef").equals("lad")){
  //     speicher.sendeTel(telLa, true);
  //   }else if(server.arg("bef").equals("ent")){
  //     speicher.sendeTel(telEl, true);
  //   }else if(server.arg("bef").equals("spa")){
  //     speicher.sendeTel(telSa);
  //   }else if(server.arg("bef").equals("md")){
  //     speicher.sendeTel(telMD);
  //   }
  //   delay(100);
  //   dateiSenden("/index.html");
  }else if(server.hasArg("rst")){
    server.client().stop();
    delay(100);
    ESP.restart();
  }else{
    dateiSenden("/index.html");
  }
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

void einstellungCSS(){
  dateiSenden("/einst.css", "text/css");
}

void sendeDaten(){
  server.send(200, "application/json", getJson());
}

void sendeEinst(){
  server.send(200, "application/json", einst.json);
}

void befehle(){
  server.send(200, "text/plain", "Ok");
  // if(server.hasArg("bef")){
  //   if(server.arg("bef").equals("laden")){
  //       speicher.sendeTel(telLa, true);
  //   }else if(server.arg("bef").equals("entladen")){
  //     speicher.sendeTel(telEl, true);
  //   }else if(server.arg("bef").equals("speicher_aus")){
  //     speicher.sendeTel(telSa);
  //   }else if(server.arg("bef").equals("laden_aus")){
  //     speicher.sendeTel(telLa);
  //   }else if(server.arg("bef").equals("laden_ein")){
  //     speicher.sendeTel(telLa + 1);
  //   }else if(server.arg("bef").equals("entladen_aus")){
  //     speicher.sendeTel(telEl);
  //   }else if(server.arg("bef").equals("entladen_ein")){
  //     speicher.sendeTel(telEl + 1);
  //   }else if(server.arg("bef").equals("mehr_daten")){
  //     speicher.sendeTel(telMD);
  //   }
  // }
}

void logdaten(){
  server.send(200, "text/plain", getLog());
}

File datei; 
void upload(){
  HTTPUpload& upload = server.upload();
  Serial.print("Upload Status:"); Serial.println(upload.status);
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    LittleFS.remove(filename);
    datei = LittleFS.open(filename, "w");
    filename = String();
  }
  else if(upload.status == UPLOAD_FILE_WRITE){
    if(datei) datei.write(upload.buf, upload.currentSize);
  } 
  else if(upload.status == UPLOAD_FILE_END){
    if(datei){
      datei.close();
      Serial.print("Upload Size: "); Serial.println(upload.totalSize);
    }else{
      Serial.println("Upload fehlgeschlagen.");
    }
  }
}

void setupWS(){
  server.onNotFound(fehlerseite);
  server.on("/",HTTP_GET, hauptseite);
  server.on("/",HTTP_POST, hauptseiteBef);
  server.on("/einst",HTTP_POST, einstellungsmenue);
  server.on("/einstAll",HTTP_POST, einstellungAll);
  server.on("/einstWl",HTTP_POST, einstellungWl);
  server.on("/einstMq",HTTP_POST, einstellungMq);
  server.on("/einstOt",HTTP_POST, einstellungOt);
  server.on("/einst.css", einstellungCSS);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/befehle",HTTP_GET, befehle);
  server.on("/log", logdaten);
  server.on("/upload",HTTP_POST, einstellungOt, upload);
  server.begin();
}

// Mqtt Client -------------------------------------------
void callback(char* topic, byte* payload, unsigned int length){
  int i = strlen(einst.mqttTp.c_str());
  if(strncmp(topic, einst.mqttTp.c_str(), i) == 0 && strcmp(topic + i, "/Befehl") == 0){
    char pl[length + 1];
    pl[length] = '\0';
    memcpy(pl, payload, length);
    // if(strcmp(pl, "laden") == 0){
    //   speicher.sendeTel(telLa, true);
    // }else if(strcmp(pl, "entladen") == 0){
    //   speicher.sendeTel(telEl, true);
    // }else if(strcmp(pl, "speicher_aus") == 0){
    //   speicher.sendeTel(telSa);
    // }else if(strcmp(pl, "laden_aus") == 0){
    //   speicher.sendeTel(telLa);
    // }else if(strcmp(pl, "laden_ein") == 0){
    //   speicher.sendeTel(telLa + 1);
    // }else if(strcmp(pl, "entladen_aus") == 0){
    //   speicher.sendeTel(telEl);
    // }else if(strcmp(pl, "entladen_ein") == 0){
    //   speicher.sendeTel(telEl + 1);
    // }else if(strcmp(pl, "mehr_daten") == 0){
    //   speicher.sendeTel(telMD);
    // }
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

void mqttPub(){
  if(!apModus){
    if(!mqttClient.connected()){
      reconnectMqtt();
    }
    if(mqttClient.connected()){
      mqttClient.publish((einst.mqttTp + "/Daten").c_str(), getJson());
    }
  }
//  addLog("Mqtt publish.");
}

void mqttPubSpontan(){
  if(einst.mqtt && einst.mqttSp)
    mqttPub();
}

// NTP Zeitserver -------------------------------------------
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"
tm dat;

void setupNTP(){
  if(!apModus){
    configTime(MY_TZ, einst.ntzIp.c_str());
//    speicher.callbackGetDatumZeit(getDatumZeit);
  }
}

void getZeit(){
  if(!apModus){
    time_t now;
    time(&now);
    localtime_r(&now, &dat);
  }
}

void getDatumZeit(){
// void getDatumZeit(Zeit *z){
//   getZeit();
//   z->jahr     = dat.tm_year - 100;
//   z->monat    = dat.tm_mon + 1;
//   z->tag      = dat.tm_mday;
//   z->stunde   = dat.tm_hour;
//   z->minute   = dat.tm_min;
//   z->sekunde  = dat.tm_sec;
//   z->tagWoche = dat.tm_wday;
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
    mqttPub();
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
  if(einst.mqttIv != ""){
    mqttPubInterval = einst.mqttIv.toInt() * 1000;
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
  generiereJson(venus.getDaten());
  mqttPubSpontan();
}

void logEintrag(const char *s){
  addLog(s);
}

void setupVenus(){
   venus.callbackLesenSenden(lesen, senden);
   venus.callbackNeueDaten(neueDaten);
   venus.callbackLogeintrag(logEintrag);
   generiereJson(venus.getDaten());
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
  Serial.setTimeout(40);
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
//  while(Serial.available()){          // Puffer leeren
//    Serial.read();
//    delay(5);
//  }
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
  delay(50);  
}
