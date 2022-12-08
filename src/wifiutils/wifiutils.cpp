#include <string>
#include <string.h>
#include <esp_log.h>

#include "wifiutils.h"

std::string WifiUtils::parseUri(std::string uri, std::string key) {

    std::string res = "";

    int found = uri.find(key);
    
    bool isParsing = false;
    int parseI = 0;
    
    for (int i=found; i<uri.length(); i++) {
        
        // 3. Jak natrafimy na '&' lub '\0' to znaczy ze koniec
        if (isParsing == true) {
            if (uri.at(i) == '&' || uri.at(i) == '\0') {
                break;
            }
        }
        
        // 2. Zapisujemy wartosc
        if (isParsing == true) {
            res.push_back(uri.at(i));

            // ESP_LOGI("HEJSAFAS", "%c", res[parseI]);

            parseI ++;
        }
        
        // 1. Natrafimy na '=', wiec start (zapisujemy wartosc)
        if (isParsing == false) {
            if (uri.at(i) == '=') {
                isParsing = true;
            }
        }
    }

    return res;
}
