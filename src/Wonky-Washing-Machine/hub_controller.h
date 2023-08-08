#ifndef HUB_CONTROLLER_H
#define HUB_CONTROLLER_H

#include <ArduinoJson.h>

void processMessage(String incomingMessage);

void addAction(JsonArray &actions, String actionId, String actionName, boolean enabled);

String getRegisterStr(String room);

#endif 
