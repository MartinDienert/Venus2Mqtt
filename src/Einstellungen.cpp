#include <ESP8266WebServer.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <Einstellungen.h>
#include <Main.h>

Einstellungen::Einstellungen(ESP8266WebServer* s){
  server = s;
}

/* void Einstellungen::einst_speichern(const char* dateiname, String daten[], int l){
  File datei = LittleFS.open(dateiname, "w");
  if(datei){
    for (int i = 0; i < l; i++){
      datei.print(daten[i]);
      datei.print(";");
    }
    datei.close();
  }
}
 */
/* boolean Einstellungen::einst_lesen(const char* dateiname, String daten[], int l){
  File datei = LittleFS.open(dateiname, "r");
  if(datei){
    for (int i = 0; i < l; i++){
      daten[i] = datei.readStringUntil(';');
    }
    datei.close();
    return true;
  }
  return false;
}
 */
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
// https://arduinojson.org/
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if(error){
//    Serial.print(F("deserializeJson() failed: "));
//    Serial.println(error.f_str());
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
/*  sprintf(json, "{\"NtzIp\":\"%s\",\"Wlan\":%d,\"SSId\":\"%s\",\"PWD\":\"%s\",\
  \"Mqtt\":%d,\"MqttSp\":%d,\"MqttIp\":\"%s\",\"MqttPo\":\"%s\",\"MqttBe\":\"%s\",\"MqttPw\":\"%s\",\
  \"MqttTp\":\"%s\",\"MqttIv\":%s}",
    ntzIp.c_str(), wlan, ssid.c_str(), pwd.c_str(),
    mqtt, mqttSp, mqttIp.c_str(), mqttPo.c_str(), mqttBe.c_str(), mqttPw.c_str(), mqttTp.c_str(), mqttIv.c_str());*/

// https://arduinojson.org/
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
/*    String tmp[8];
    if(einst_lesen("einst", tmp, 3)){
        ntzIp = tmp[2];
    }
    if(einst_lesen("wlan", tmp, 3)){
        wlan = (tmp[0].equals("on"))? true: false;
        ssid = tmp[1];
        pwd = tmp[2];
    }
    if(einst_lesen("mqtt", tmp, 8)){
        mqtt = (tmp[0].equals("on"))? true: false;
        mqttSp = (tmp[1].equals("on"))? true: false;
        mqttIp = tmp[2];
        mqttPo = tmp[3];
        mqttBe = tmp[4];
        mqttPw = tmp[5];
        mqttTp = tmp[6];
        mqttIv = tmp[7].toInt();
    }
    genJson();*/
  if(json_lesen()) parseJson(); else genJson();

}

void Einstellungen::setEinst(){
    if(server->hasArg("save")){
        if(server->arg("save").equals("ei") && server->hasArg("ntip")){
            ntzIp = server->arg("ntip");
//            String daten[] = {ntzIp};
//            einst_speichern("einst", daten, 1);
        }else if(server->arg("save").equals("wl") && server->hasArg("ssid") && server->hasArg("pwd")){
            wlan = (server->hasArg("wl") && server->arg("wl").equals("on"))? true: false;
            ssid = server->arg("ssid");
            if(!server->arg("pwd").equals("") || (ssid.equals("") && server->arg("pwd").equals("")))
              pwd = server->arg("pwd");
//            String daten[] = {(wlan)? "on": "off", ssid, pwd};
//            einst_speichern("wlan", daten, 3);
        }else if(server->arg("save").equals("mq") && server->hasArg("mqip") && server->hasArg("mqpo") && server->hasArg("mqtp")){
            mqtt = (server->hasArg("mq") && server->arg("mq").equals("on"))? true: false;
            mqttSp = (server->hasArg("mqsp") && server->arg("mqsp").equals("on"))? true: false;
            mqttIp = server->arg("mqip");
            mqttPo = server->arg("mqpo");
            (server->hasArg("mqbe"))? mqttBe = server->arg("mqbe"): mqttBe = "";
            (server->hasArg("mqpw"))? mqttPw = server->arg("mqpw"): mqttPw = "";
            mqttTp = server->arg("mqtp");
            (server->hasArg("mqiv"))? mqttIv = server->arg("mqiv").toInt(): mqttIv = 0;
//            String daten[] = {(mqtt)? "on": "off", (mqttSp)? "on": "off", mqttIp, mqttPo, mqttBe, mqttPw, mqttTp, mqttIv};
//            einst_speichern("mqtt", daten, 8);
            setMqttPubTimer();
        }
        genJson();
        json_speichern();
    }
}
