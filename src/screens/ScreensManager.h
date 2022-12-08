/*
 * ScreensManager.h
 *
 * Created on Thu Dec 01 2022
 *
 * Copyright (c) 2022 Your Company
 */
#ifndef _SCREENS_MANAGER_H_
#define _SCREENS_MANAGER_H_

#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"

#include "Screens.h"

extern QueueHandle_t screens_manager_wifi_queue;
extern QueueHandle_t screens_manager_btn_queue;

void screens_manager_init();
void screens_manager_start();
void screens_manager_register_basic_screen(Screens *screensRob, uint8_t pos);
void screens_manager_register_screen(Screens *screensRob);
void screens_manager_task(void *argp);

std::string screensManager_get_config_json_str();
void screensManager_set_config(std::string screen_name, std::vector<std::string> conf);


#endif
