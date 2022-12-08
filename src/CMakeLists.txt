# idf_component_register(SRCS "main.cpp" "screens/screens.h" "screens/screens.cpp" "display/ws2812b/ws2812b.h" "display/ws2812b/ws2812b.cpp" "display/ws2812b/ws2812b_driver.S" "display/display.h" "display/display.cpp" "display/font.h" "wifiprovisioning/wifiprovisioning.cpp" "wifiprovisioning/wifiprovisioning.h" "nvs/nvsutils.cpp" "nvs/nvsutils.h" "wifimanager/wifimanager.cpp" "wifimanager/wifimanager.h" "wifiutils/wifiutils.cpp" "wifiutils/wifiutils.h")
#idf_component_register(SRC_DIRS ".")


idf_component_register(SRCS
"config.h"
"main.cpp"
"main.h"

"display/ws2812b/ws2812b_driver.S"
"display/ws2812b/ws2812b.cpp"
"display/ws2812b/ws2812b.h"
"display/display.cpp"
"display/display.h"
"display/font.h"

"EasyButton/EasyButton.cpp"
"EasyButton/EasyButton.h"

"nvs/nvsutils.cpp"
"nvs/nvsutils.h"


"screens/default/ScreenLoading.cpp"
"screens/default/ScreenLoading.h"

"screens/default/ScreenAPConfig.cpp"
"screens/default/ScreenAPConfig.h"

"screens/default/ScreenSTAFailed.cpp"
"screens/default/ScreenSTAFailed.h"

"screens/default/ScreenTime.cpp"
"screens/default/ScreenTime.h"

"screens/default/ScreenText.cpp"
"screens/default/ScreenText.h"


"screens/Screens.h"
"screens/ScreensManager.cpp"
"screens/ScreensManager.h"

"wifiprovisioning/WifiProvisioning.AP.cpp"
"wifiprovisioning/WifiProvisioning.cpp"
"wifiprovisioning/WifiProvisioning.h"
"wifiprovisioning/WifiProvisioning.STA.cpp"
"wifiprovisioning/WifiProvisioning.WifiManager.cpp"

"wifiutils/wifiutils.cpp"
"wifiutils/wifiutils.h"

)

