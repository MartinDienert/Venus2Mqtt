#ifndef MAIN_H_
#define MAIN_H_

//#include <SpeicherLib.h>

//extern Speicher speicher;
extern tm dat;

void reconnectMqtt();
//void getDatumZeit(Zeit *);
void getDatumZeitStr(char *, char *);
void setMqttPubTimer();

#endif
