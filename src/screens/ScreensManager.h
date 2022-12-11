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

void screensManager_init();
esp_err_t screensManager_register_screen(Screens *s);
void screensManager_start();
void screensManager_task(void *argp);

std::string screensManager_get_config_json_str();
esp_err_t screensManager_set_screen(std::string screen_name, std::vector<std::string> conf);
esp_err_t screensManager_set_screens(std::vector<std::string> screen_names);
int screensManager_get_screens_visible_amount();


#endif
