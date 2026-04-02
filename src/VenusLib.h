#ifndef VENUS_LIB_H_
#define VENUS_LIB_H_
#include <Arduino.h>
#include <ModbusMaster.h>
//#include <ArduinoJson.h>

struct Reg{
    int reg;
    int typ;
    float faktor;
    char name[11];
};

const int maxReg = 10;

const Reg reg[] = {{30001, 1, 1, "Batt_P"}, {30006, 1, 1, "Netz_P"}, {30100, 1, 0.01, "Batt_U"}, {35000, 1, 0.1, "Batt_T"}, {37005, 1, 1, "Batt_Soc"},
                   {33000, 2, 0.01, "Ges_E_gel"}, {33002, 2, 0.01, "Ges_E_ent"}, {33004, 2, 0.01, "Tag_E_gel"}, {33006, 2, 0.01, "Tag_E_ent"}};
const int arrayGr = sizeof(reg) / sizeof(reg[0]);

const int regAkkuU = 30100;             // Register Batterie Spannung
const int regAkkuI = 30101;             // Register Batterie Strom
const int regAkkuP = 30001;             // Register Batterie Leistung
const int regAkkuSoc = 37005;           // Register Batterie SOC
const int regNetzP = 30006;             // Register Netz Leistung
const int regTemp = 35000;              // Register Temperatur
const long abfrage_Interval = 1000;     // Abfrageintervall in Millisekunden, (1 Sekunde)

/* struct Daten{
    float spannungAkku = 0;
    int stromAkku = 0;
    int leistungAkku;
    int soc = 0;
    int leistungAc = 0;
    int temperatur = 0;
    boolean laden = true;
    boolean entladen = true;
};
 */
class Venus{                     // Class Declaration
    protected:
//        using LesenSenden = void (*)(uint8_t);
        using NeueDaten = void (*)();
        NeueDaten neueDaten = nullptr;
        using DatumZeit = void (*)(char*, char*);
        DatumZeit datumZeit = nullptr;
        using Logeintrag = void (*)(const char *);
        Logeintrag logeintrag = nullptr;

    public:
        Venus(int id, Stream &serial);  // Constructor
        void callbackLesenSenden(void (*)(), void (*)());
        void callbackNeueDaten(NeueDaten);
        void callbackDatumZeit(DatumZeit);
        void callbackLogeintrag(Logeintrag);
        void run();
//        Daten getDaten();
        char json[250] = {'\0'};

    private:
        ModbusMaster modbusMaster;
        void reg16Lesen();
        void reg32Lesen();
        void pollen();
        void genJson();
        int werte[arrayGr];
//        Daten daten;
        // ------ Timer -------
        unsigned long abfrageZeit = 0;
        unsigned long abfrageInterval;
//        int teleParam;
        void timerRun();
        void setAbfrageTimer(unsigned long);
};

#endif
