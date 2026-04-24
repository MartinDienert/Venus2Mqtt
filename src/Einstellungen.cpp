#include <ESP8266WebServer.h>
#include "LittleFS.h"
#include <ArduinoJson.h>              // https://arduinojson.org/
#include <Einstellungen.h>
#include <Main.h>

Einstellungen::Einstellungen(ESP8266WebServer* s){
  server = s;
}

void Einstellungen::json_speichern(){
  File datei = LittleFS.open("einst.json", "w");
  if(datei){
      datei.print(json);
    datei.close();
  }
}

boolean Einstellungen::json_lesen(){
  File datei = LittleFS.open("einst.json", "r");
  if(datei){
      strcpy(json, datei.readString().c_str());
    datei.close();
    return true;
  }
  return false;
}

void Einstellungen::parseJson(){
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if(error){
    addLog("Einstellungen: Fehler beim JSON parsen.");
    return;
  }
  ntzIp = doc["NtzIp"].as<String>();
  wlan = doc["Wlan"];
  ssid = doc["SSId"].as<String>();
  pwd = doc["PWD"].as<String>();
  mqtt = doc["Mqtt"];
  mqttSp = doc["MqttSp"];
  mqttIp = doc["MqttIp"].as<String>();
  mqttPo = doc["MqttPo"].as<String>();
  mqttBe = doc["MqttBe"].as<String>();
  mqttPw = doc["MqttPw"].as<String>();
  mqttTp = doc["MqttTp"].as<String>();
  mqttIv = doc["MqttIv"].as<int>();

}

void Einstellungen::genJson(){
  JsonDocument doc;
  doc["NtzIp"] = ntzIp;
  doc["Wlan"] = wlan;
  doc["SSId"] = ssid;
  doc["PWD"] = pwd;
  doc["Mqtt"] = mqtt;
  doc["MqttSp"] = mqttSp;
  doc["MqttIp"] = mqttIp;
  doc["MqttPo"] = mqttPo;
  doc["MqttBe"] = mqttBe;
  doc["MqttPw"] = mqttPw;
  doc["MqttTp"] = mqttTp;
  doc["MqttIv"] = mqttIv;
  serializeJson(doc, json);
}

void Einstellungen::alle_einst_laden(){
  if(json_lesen())
    parseJson();
  else{
    genJson();
    json_speichern();
  }
}

void Einstellungen::setEinst(){
    if(server->hasArg("save")){
        if(server->arg("save").equals("ei") && server->hasArg("ntip")){
            ntzIp = server->arg("ntip");
        }else if(server->arg("save").equals("wl") && server->hasArg("ssid") && server->hasArg("pwd")){
            wlan = (server->hasArg("wl") && server->arg("wl").equals("on"))? true: false;
            ssid = server->arg("ssid");
            if(!server->arg("pwd").equals("") || (ssid.equals("") && server->arg("pwd").equals("")))
              pwd = server->arg("pwd");
        }else if(server->arg("save").equals("mq") && server->hasArg("mqip") && server->hasArg("mqpo") && server->hasArg("mqtp")){
            mqtt = (server->hasArg("mq") && server->arg("mq").equals("on"))? true: false;
            mqttSp = (server->hasArg("mqsp") && server->arg("mqsp").equals("on"))? true: false;
            mqttIp = server->arg("mqip");
            mqttPo = server->arg("mqpo");
            (server->hasArg("mqbe"))? mqttBe = server->arg("mqbe"): mqttBe = "";
            (server->hasArg("mqpw"))? mqttPw = server->arg("mqpw"): mqttPw = "";
            mqttTp = server->arg("mqtp");
            (server->hasArg("mqiv"))? mqttIv = server->arg("mqiv").toInt(): mqttIv = 0;
            setMqttPubTimer();
        }
        genJson();
        json_speichern();
    }
}
