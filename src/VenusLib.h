#ifndef VENUS_LIB_H_
#define VENUS_LIB_H_
#include <Arduino.h>
#include <ModbusMaster.h>

const int regAkkuU = 30100;             // Register Batterie Spannung
const int regAkkuI = 30101;             // Register Batterie Strom
const int regAkkuP = 30001;             // Register Batterie Leistung
const int regAkkuSoc = 37005;           // Register Batterie SOC
const int regNetzP = 30006;             // Register Netz Leistung
const int regTemp = 35000;              // Register Temperatur
const long abfrage_Interval = 1000;     // Abfrageintervall in Millisekunden, (1 Sekunde)

struct Daten{
    float spannungAkku = 0;
    int stromAkku = 0;
    int leistungAkku;
    int soc = 0;
    int leistungAc = 0;
    int temperatur = 0;
    boolean laden = true;
    boolean entladen = true;
};

class Venus{                     // Class Declaration
    protected:
        using LesenSenden = void (*)(uint8_t);
        using Schreiben = void (*)(byte *, int);
        Schreiben schreiben = nullptr;
        using NeueDaten = void (*)();
        NeueDaten neueDaten = nullptr;
        using Logeintrag = void (*)(const char *);
        Logeintrag logeintrag = nullptr;

    public:
        static LesenSenden lesensenden;
        Venus(int id, Stream &serial);  // Constructor
        void callbackLesenSenden(LesenSenden);
        void callbackSchreiben(Schreiben);
        void callbackNeueDaten(NeueDaten);
        void callbackLogeintrag(Logeintrag);
        void run();
        Daten getDaten();

    private:
        ModbusMaster node;
        static void preTransmission();
        static void postTransmission();
        void pollen();
        Daten daten;
        // ------ Timer -------
        unsigned long abfrageZeit = 0;
        unsigned long abfrageInterval;
        int teleParam;
        void timerRun();
        void setAbfrageTimer(unsigned long);
};

#endif
