#ifndef EINSTELLUNGEN_H_
#define EINSTELLUNGEN_H_
#include <Arduino.h>
#include <ESP8266WebServer.h>


class Einstellungen  // Class Declaration
{
    public:
        Einstellungen(ESP8266WebServer* s);
        boolean master = false;
        boolean mDaten = false;
        String ntzIp = "de.pool.ntp.org";
        boolean wlan = false;
        String ssid = "";
        String pwd = "";
        boolean mqtt = false;
        boolean mqttSp = true;
        String mqttIp = "";
        String mqttPo = "";
        String mqttBe = "";
        String mqttPw = "";
        String mqttTp = "";
        String mqttIv = "30";
        char json[250] = {'\0'};
        void genJson();
        void alle_einst_laden();
        void setEinst();
    
    private:
        ESP8266WebServer* server;
        void einst_speichern(const char* dateiname, String daten[], int l);
        boolean einst_lesen(const char* dateiname, String daten[], int l);
};

#endif