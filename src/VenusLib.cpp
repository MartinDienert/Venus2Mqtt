#include <VenusLib.h>
#include <ModbusMaster.h>

Venus::Venus(int id, Stream &serial){
    node.begin(id, serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
}

void Venus::preTransmission(){
    if(lesensenden) lesensenden(1);                 // umschalten auf schreiben
}

void Venus::postTransmission(){
    if(lesensenden) lesensenden(0);                 // umschalten auf lesen
}

void Venus::callbackLesenSenden(LesenSenden l){
    lesensenden = l;
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
  uint8_t result;
  result = node.readHoldingRegisters(regAkkuP, 1);
  if(result == node.ku8MBSuccess){
    Serial.print("Vbatt: ");
    Serial.println(node.getResponseBuffer(0x04)/100.0f);
    Serial.print("Vload: ");
    Serial.println(node.getResponseBuffer(0xC0)/100.0f);
    Serial.print("Pload: ");
    Serial.println((node.getResponseBuffer(0x0D) +
                    node.getResponseBuffer(0x0E) << 16)/100.0f);
  }

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
