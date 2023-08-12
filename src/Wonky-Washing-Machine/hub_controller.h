#ifndef HUB_CONTROLLER_H
#define HUB_CONTROLLER_H

#include <ArduinoJson.h>

class HubController {
public:

    boolean hubEnabled = true;

    boolean hubPuzzleResetRequest = false;

    void setupHub();

    void processMessage(String jsonString);

    void addAction(JsonArray &actions, String actionId, String actionName, boolean enabled);

    void generateRegistrationJson(String room, String statusStr);

    void checkHub();

    bool isValidJSON(String jsonString);

    void updateStatus(String room, String statusStr);
};

#endif
