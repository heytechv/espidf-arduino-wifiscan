#ifndef _WIFIUTILS_H_
#define _WIFIUTILS_H_


class WifiUtils {

public:

    /**
     * @brief Parse uri and get value by key e.g. (uri="ssid=network&pass=1234", key="ssid") => network
     * 
     * @param[in] uri The uri parameters
     * @param[in] key The key we want to look for
     * @param[out] value The value found in uri by key
     */
    static std::string parseUri(std::string uri, std::string key);

};

#endif
