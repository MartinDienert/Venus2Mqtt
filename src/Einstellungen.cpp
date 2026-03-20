#include <ESP8266WebServer.h>
#include "LittleFS.h"
#include <Einstellungen.h>
#include <Main.h>



Einstellungen::Einstellungen(ESP8266WebServer* s){
  server = s;
  genJson();
}

void Einstellungen::einst_speichern(const char* dateiname, String daten[], int l){
  File datei = LittleFS.open(dateiname, "w");
  if(datei){
    for (int i = 0; i < l; i++){
      datei.print(daten[i]);
      datei.print(";");
    }
    datei.close();
  }
}

boolean Einstellungen::einst_lesen(const char* dateiname, String daten[], int l){
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

void Einstellungen::genJson(){
  sprintf(json, "{\"Master\":%d,\"mDaten\":%d,\"NtzIp\":\"%s\",\"Wlan\":%d,\"SSId\":\"%s\",\"PWD\":\"%s\",\
  \"Mqtt\":%d,\"MqttSp\":%d,\"MqttIp\":\"%s\",\"MqttPo\":\"%s\",\"MqttBe\":\"%s\",\"MqttPw\":\"%s\",\
  \"MqttTp\":\"%s\",\"MqttIv\":\"%s\"}",
    master, mDaten, ntzIp.c_str(), wlan, ssid.c_str(), pwd.c_str(),
    mqtt, mqttSp, mqttIp.c_str(), mqttPo.c_str(), mqttBe.c_str(), mqttPw.c_str(), mqttTp.c_str(), mqttIv.c_str());
}

void Einstellungen::alle_einst_laden(){
    String tmp[8];
    if(einst_lesen("einst", tmp, 3)){
        master = (tmp[0].equals("on"))? true: false;
        mDaten = (tmp[1].equals("on"))? true: false;
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
        mqttIv = tmp[7];
    }
    genJson();
}

void Einstellungen::setEinst(){
    if(server->hasArg("save")){
        if(server->arg("save").equals("ei") && server->hasArg("ntip")){
            boolean m = master;
            master = (server->hasArg("ma") && server->arg("ma").equals("on"))? true: false;
            if(m != master) speicher.setMaster(master);
            m = mDaten;
            mDaten = (server->hasArg("md") && server->arg("md").equals("on"))? true: false;
            if(m != mDaten) speicher.setMDaten(mDaten);
            ntzIp = server->arg("ntip");
            String daten[] = {(master)? "on": "off", (mDaten)? "on": "off", ntzIp};
            einst_speichern("einst", daten, 3);
        }else if(server->arg("save").equals("wl") && server->hasArg("ssid") && server->hasArg("pwd")){
            wlan = (server->hasArg("wl") && server->arg("wl").equals("on"))? true: false;
            ssid = server->arg("ssid");
            if(!server->arg("pwd").equals("") || (ssid.equals("") && server->arg("pwd").equals("")))
              pwd = server->arg("pwd");
            String daten[] = {(wlan)? "on": "off", ssid, pwd};
            einst_speichern("wlan", daten, 3);
        }else if(server->arg("save").equals("mq") && server->hasArg("mqip") && server->hasArg("mqpo") && server->hasArg("mqtp")){
            mqtt = (server->hasArg("mq") && server->arg("mq").equals("on"))? true: false;
            mqttSp = (server->hasArg("mqsp") && server->arg("mqsp").equals("on"))? true: false;
            mqttIp = server->arg("mqip");
            mqttPo = server->arg("mqpo");
            (server->hasArg("mqbe"))? mqttBe = server->arg("mqbe"): mqttBe = "";
            (server->hasArg("mqpw"))? mqttPw = server->arg("mqpw"): mqttPw = "";
            mqttTp = server->arg("mqtp");
            (server->hasArg("mqiv"))? mqttIv = server->arg("mqiv"): mqttIv = "";
            String daten[] = {(mqtt)? "on": "off", (mqttSp)? "on": "off", mqttIp, mqttPo, mqttBe, mqttPw, mqttTp, mqttIv};
            einst_speichern("mqtt", daten, 8);
            setMqttPubTimer();
        }
        genJson();
    }
}
