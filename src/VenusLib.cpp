#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <VenusLib.h>

Venus::Venus(int id, Stream &serial){
    modbusMaster.begin(id, serial);
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

void Venus::callbackLogeintrag(AddLog l){
    addLog = l;
}

void Venus::logeintrag(const char* s){
    addLog(s);
}

void Venus::logeintrag(const char* s1, const char* f, const char* s2, int r){
    int l = strlen(s1) + strlen(f) + strlen(s2) + 5 + 1;
    char tx[l];
    snprintf(tx, l,"%s%s%s%d", s1, f, s2, r);
    addLog(tx);
}

void Venus::setIntervall(unsigned long a){
    setAbfrageTimer(a);
}

void Venus::genRegister(const char* s){
    if(reg != nullptr) free(reg);
    if(werte != nullptr) free(werte);
    reg = nullptr;
    werte = nullptr;
    arrayGr = 0;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, s);
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
            logeintrag("Fehler Register schreiben: ", getFehler(), ", Register: ", r);
        }
    }else{
        logeintrag("Fehler Register schreiben: ", getFehler(), ", Register: ", r);
    }
}

const char* Venus::getFehler(){
    switch(modbusFehler){
        case modbusMaster.ku8MBIllegalFunction:
            return "illegale Funktion";
        case modbusMaster.ku8MBIllegalDataAddress:
            return "ungültige Adresse";
        case modbusMaster.ku8MBIllegalDataValue:
            return "fehlerhafter Wert";
        case modbusMaster.ku8MBSlaveDeviceFailure:
            return "Gerätefehler";
        case modbusMaster.ku8MBInvalidSlaveID:
            return "ungültige Slave ID";
        case modbusMaster.ku8MBInvalidFunction:
            return "ungültige Funktion";
        case modbusMaster.ku8MBResponseTimedOut:
            return "Zeitüberschreitung";
        case modbusMaster.ku8MBInvalidCRC:
            return "ungültige CRC";
    }
    return "";
}

char* Venus::getRegJson(int r, boolean r32){
    int w = getReg(r, r32);
    JsonDocument doc;
    if(modbusFehler != modbusMaster.ku8MBSuccess){
        doc["fehler"] = getFehler();
    }else{
        char b[10];
        itoa(r, b, 10);
        doc[b] = (r32)? (int32_t)w: (int16_t)w;
    }
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
    }else{
        logeintrag("Fehler Register lesen: ", getFehler(), ", Register: ", r);
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
            doc[d.name] = (d.typ == 1)? (int16_t)(werte[i] * d.faktor): (int32_t)(werte[i] * d.faktor);
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
        if(modbusFehler == modbusMaster.ku8MBResponseTimedOut)
          return;
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
    if(abfrageZeit > 0 && zeit - abfrageZeit > abfrageIntervall){
        abfrageZeit = zeit;
        if(abfrageZeit == 0) abfrageZeit = 1;
        pollen();
    }
}

void Venus::setAbfrageTimer(unsigned long i){
    abfrageIntervall = i;
    if(abfrageIntervall > 0){
        abfrageZeit = millis();
        if(abfrageZeit == 0) abfrageZeit = 1;
    }else
        abfrageZeit = 0;
}
