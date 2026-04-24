#include <ModbusMaster.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <VenusLib.h>

Venus::Venus(int id, Stream &serial){
    modbusMaster.begin(id, serial);
    setAbfrageTimer(abfrage_Interval);
}

Venus::~Venus(){
    if(reg != nullptr) free(reg);
    if(werte != nullptr) free(werte);
}

void Venus::callbackLesenSenden(void (*l)(), void (*s)()){
    modbusMaster.preTransmission(s);
    modbusMaster.postTransmission(l);
} 

void Venus::callbackNeueDaten(NeueDaten d){
    neueDaten = d;
}

void Venus::callbackDatumZeit(DatumZeit d){
    datumZeit = d;
}

void Venus::callbackLogeintrag(Logeintrag l){
    logeintrag = l;
}

String Venus::json_lesen(const char* d){
    File datei = LittleFS.open(d, "r");
    if(datei){
        String s = datei.readString();
        datei.close();
        return s;
    }
    return "";
}

void Venus::genRegister(){
    if(reg != nullptr) free(reg);
    if(werte != nullptr) free(werte);
    reg = nullptr;
    werte = nullptr;
    arrayGr = 0;
    JsonDocument doc;
    String s = json_lesen("register.json");
    if(s.length() == 0){
        logeintrag("Fehler beim lesen der register.json.");
        return;
    }
    DeserializationError error = deserializeJson(doc, s.c_str());
    if(error){
        logeintrag("Fehler beim deserialisieren der register.json.");
        return;
    }
    JsonArray ja = doc["reg"];
    arrayGr = ja.size();
    reg = (Reg*)malloc(arrayGr * sizeof(Reg));
    werte = (int*)malloc(arrayGr * sizeof(int));
    for(int i = 0; i < arrayGr; i++){
        reg[i].reg = ja[i]["reg"];
        reg[i].typ = ja[i]["typ"];
        reg[i].faktor = ja[i]["faktor"];
        strcpy(reg[i].name, ja[i]["name"]);
        werte[i] = 0;
    }
}

void Venus::run(){
    timerRun();
}

void Venus::setReg(int r, boolean r32, int w){
    modbusFehler = modbusMaster.writeSingleRegister(r, lowWord(w));
    if(modbusFehler == modbusMaster.ku8MBSuccess){
        if(r32) modbusFehler = modbusMaster.writeSingleRegister(r + 1, highWord(w));
        if(modbusFehler != modbusMaster.ku8MBSuccess){
            logeintrag("Fehler beim Register schreiben, HighWord.");
        }
    }else{
        logeintrag("Fehler beim Register schreiben, LowWord.");
    }
}

char* Venus::getRegJson(int r, boolean r32){
    int w = getReg(r, r32);
    JsonDocument doc;
    char b[10];
    itoa(r, b, 10);
    if(modbusFehler != modbusMaster.ku8MBSuccess){
        switch(modbusFehler){
            case modbusMaster.ku8MBIllegalFunction:
                doc["fehler"] = "illegale Funktion";
                break;
            case modbusMaster.ku8MBIllegalDataAddress:
                doc["fehler"] = "falsche Adresse";
                break;
            case modbusMaster.ku8MBIllegalDataValue:
                doc["fehler"] = "fehlerhafter Wert";
                break;
            case modbusMaster.ku8MBSlaveDeviceFailure:
                doc["fehler"] = "Gerätefehler";
                break;
        }
    }else
        doc[b] = (r32)? (int32_t)w: (int16_t)w;
    serializeJson(doc, wertJson, 40);
    return wertJson;
}

int Venus::getReg(int r, boolean r32){
    int w = 0;
    getRegs(r, r32, 1, 0, &w);
    return w;
}

boolean Venus::getRegs(int r, boolean r32, int a, int p, int* werte){
    uint8_t gr = (r32)? 2: 1;
    boolean g = false;
    modbusFehler = modbusMaster.readHoldingRegisters(r, a * gr);
    if(modbusFehler == modbusMaster.ku8MBSuccess){
        for(int i = 0; i < a; i++){
            int w = modbusMaster.getResponseBuffer(i * gr);
            if(r32){
                w <<= 16;
                w += modbusMaster.getResponseBuffer(i * gr + 1);
            }
            if(w != werte[p + i]){
                werte[p + i] = w;
                g = true;
            }
        }
    }
    return g;
}

void Venus::genJson(){
    JsonDocument doc;
    char datum[36];
    char zeit[36];
    if(datumZeit){
        datumZeit(datum, zeit);
        doc["Datum"] = datum;
        doc["Zeit"] = zeit;
    }
    for(int i = 0; i < arrayGr; i++){
        Reg d = reg[i];
        if(d.faktor < 1){
            doc[d.name] = (float_t)(werte[i] * d.faktor);
        }else{
            if(d.typ == 1)
                doc[d.name] = (int16_t)(werte[i] * d.faktor);
            else
                doc[d.name] = werte[i] * d.faktor;
        }
    }
    serializeJson(doc, json, 250);
}

void Venus::pollen(){
//    long z = millis();
//    char s[50];
    boolean geaendert = false;
    for(int i = 0; i < arrayGr; i++){
        int ii = 0;
        while(i + ii + 1 < arrayGr && reg[i + ii + 1].typ == reg[i + ii].typ && reg[i + ii + 1].reg - reg[i + ii + 1].typ == reg[i + ii].reg && ii <= maxReg)
            ii++;
        if(getRegs(reg[i].reg, (reg[i].typ == 2)? true: false, ii + 1, i, werte))
            geaendert = true;
        i += ii;
    }
//    sprintf(s, "Dauer Modbus: %d.\n", millis() - z);
//    logeintrag(s);
    if(geaendert){
        genJson();
        if(neueDaten) neueDaten();
    }
//    sprintf(s, "Dauer Gesamt: %d.\n", millis() - z);
//    logeintrag(s);
}

// --------------------- Timer ---------------------
void Venus::timerRun(){
    unsigned long zeit = millis();
    if(abfrageZeit > 0 && zeit - abfrageZeit > abfrageInterval){
        abfrageZeit = 0;
        setAbfrageTimer(abfrage_Interval);
        pollen();
    }
}

void Venus::setAbfrageTimer(unsigned long wz){
    abfrageZeit = millis();
    if(abfrageZeit == 0) abfrageZeit = 1;
    abfrageInterval = wz;
}
