#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <VenusLib.h>

Venus::Venus(int id, Stream &serial){
    modbusMaster.begin(id, serial);
    setAbfrageTimer(abfrage_Interval);
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

void Venus::run(){
    timerRun();
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
            doc[d.name] = (int16_t)(werte[i] * d.faktor);
        }
    }
    serializeJson(doc, json);
}

void Venus::pollen(){
//    long z = millis();
//    char s[50];
    boolean geaendert = false;
    uint8_t result;
    for(int i = 0; i < arrayGr; i++){
        int ii = 0;
        while(i + ii + 1 < arrayGr && reg[i + ii + 1].typ == reg[i + ii].typ && reg[i + ii + 1].reg - reg[i + ii + 1].typ == reg[i + ii].reg && ii <= maxReg)
//        while(i + ii + 1 < arrayGr && reg[i + ii + 1].reg - 1 == reg16[i + ii].reg && ii <= maxReg)
            ii++;
        int gr = reg[i].typ;
        result = modbusMaster.readHoldingRegisters(reg[i].reg, ii + 1 * gr);
//        Serial.print("Reg: ");Serial.print(reg16[i].reg);Serial.print(", Anzahl: ");Serial.print(ii + 1);
//        delay(100);
        if(result == modbusMaster.ku8MBSuccess){
            for(int iii = 0; iii <= ii; iii++){
                int w = modbusMaster.getResponseBuffer(iii * gr);
                if(gr == 2){
                    w <<= 16;
                    w += modbusMaster.getResponseBuffer(iii * gr + 1);
                }
                if(w != werte[i + iii]){
                    werte[i + iii] = w;
                    geaendert = true;
                }
            }
        }
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

//Daten Venus::getDaten(){
//    return daten;
//}

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
