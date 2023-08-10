#ifndef HUB_CONTROLLER_H
#define HUB_CONTROLLER_H

#include <ArduinoJson.h>

class HubController {
public:

    boolean hubPuzzleResetRequest = false;

    void setupHub();

    void processMessage(String incomingMessage);

    void addAction(JsonArray &actions, String actionId, String actionName, boolean enabled);

    String getRegisterStr(String room);

    void checkHub();
};

#endif
