#ifndef EINSTELLUNGEN_H_
#define EINSTELLUNGEN_H_
#include <Arduino.h>
#include <ESP8266WebServer.h>


class Einstellungen  // Class Declaration
{
    public:
        Einstellungen(ESP8266WebServer* s);
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
        int mqttIv = 30;
        char json[250] = {'\0'};
        void genJson();
        void alle_einst_laden();
        void setEinst();
    
    private:
        ESP8266WebServer* server;
        void parseJson();
        void json_speichern();
        boolean json_lesen();

};

#endif