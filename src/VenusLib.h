#ifndef VENUS_LIB_H_
#define VENUS_LIB_H_
#include <Arduino.h>
#include <ModbusMaster.h>

struct Reg{
    int reg;
    int typ;                            // 1 - 16 Bit Register, 2 - 32 Bit Register
    float faktor;
    char name[11];
};

const int maxReg = 10;

class Venus{                            // Class Declaration
    protected:
        using NeueDaten = void (*)();
        NeueDaten neueDaten = nullptr;
        using DatumZeit = void (*)(char*, char*);
        DatumZeit datumZeit = nullptr;
        using AddLog = void (*)(const char*);
        AddLog addLog = nullptr;

    public:
        Venus(int id, Stream &serial);  // Constructor
        ~Venus();                       // Destructor
        void callbackLesenSenden(void (*)(), void (*)());
        void callbackNeueDaten(NeueDaten);
        void callbackDatumZeit(DatumZeit);
        void callbackLogeintrag(AddLog);
        void setIntervall(unsigned long);
        void genRegister(const char*);
        void run();
        void setReg(int, boolean, int);
        char* getRegJson(int, boolean);
        int getReg(int, boolean);
        boolean getRegs(int, boolean, int, int, int*);
        char json[250] = {'\0'};

    private:
        ModbusMaster modbusMaster;
        void logeintrag(const char*);
        void logeintrag(const char*, const char*, const char*, int);
        String json_lesen(const char*);
        void pollen();
        void genJson();
        Reg* reg = nullptr;
        int* werte = nullptr;
        int arrayGr = 0;
        char wertJson[40] = {'\0'};
        uint8_t modbusFehler;
        const char* getFehler();
        // ------ Timer -------
        unsigned long abfrageZeit = 0;
        unsigned long abfrageIntervall = 0;             // Abfrageintervall in Millisekunden (0 = keine Abfrage)
        void timerRun();
        void setAbfrageTimer(unsigned long);
};

#endif
