#ifndef HUB_CONTROLLER_H
#define HUB_CONTROLLER_H

#include <ArduinoJson.h>

class HubController {
public:

    boolean hubPuzzleResetRequest = false;

    void setupHub();

    void processMessage(String jsonString);

    void addAction(JsonArray &actions, String actionId, String actionName, boolean enabled);

    void generateRegistrationJson(String room);

    String getRegisterStr();

    void checkHub();

    bool isValidJSON(String jsonString);
};

#endif
