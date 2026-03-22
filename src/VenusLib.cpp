#include <VenusLib.h>
#include <ModbusMaster.h>

Venus::Venus(int id, Stream &serial){
//    modbusMaster.begin(id, serial);
    setAbfrageTimer(abfrage_Interval);
}

void Venus::callbackLesenSenden(void (*l)(), void (*s)()){
    modbusMaster.preTransmission(s);
    modbusMaster.postTransmission(l);
} 

void Venus::callbackNeueDaten(NeueDaten d){
    neueDaten = d;
}

void Venus::callbackLogeintrag(Logeintrag l){
    logeintrag = l;
}

void Venus::run(){
    timerRun();
}

void Venus::pollen(){
//   uint8_t result;
//   result = modbusMaster.readHoldingRegisters(regAkkuP, 1);
//   if(result == modbusMaster.ku8MBSuccess){
//     Serial.print("Vbatt: ");
//     Serial.println(modbusMaster.getResponseBuffer(0x04)/100.0f);
//     Serial.print("Vload: ");
//     Serial.println(modbusMaster.getResponseBuffer(0xC0)/100.0f);
//     Serial.print("Pload: ");
//     Serial.println((modbusMaster.getResponseBuffer(0x0D) + (modbusMaster.getResponseBuffer(0x0E) << 16))/100.0f);
//   }
    if(logeintrag) logeintrag("pollen---");
}

Daten Venus::getDaten(){
    return daten;
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
