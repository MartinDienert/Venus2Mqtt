#ifndef MAIN_H_
#define MAIN_H_

void addLog(const char *);
void reconnectMqtt();
void mqttPub(String, char *);
void getDatumZeitStr(char *, char *);
void setMqttPubIntervall(unsigned long);
void setModbusIntervall(unsigned long);

#endif
