// https://github.com/LennartHennigs/Button2/blob/master/src/Button2.h

#include "EasyButton.h"

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "sdkconfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include "main.h"
#include "wifiprovisioning/WifiProvisioning.h"

// Loop instead of interrupt, so kicked this out
static void IRAM_ATTR gpio_interrupt_handler(void *args) {
    int pinNumber = (int)args;
}

void EasyButton::begin(uint8_t pin) {
    gpio_pad_select_gpio(pin);
    gpio_set_direction  ((gpio_num_t) pin, GPIO_MODE_INPUT);
    gpio_pulldown_en    ((gpio_num_t) pin);
    gpio_pullup_dis     ((gpio_num_t) pin);
    PIN = pin;

    // Loop instead of interrupt, so kicked this out
    // gpio_set_intr_type  ((gpio_num_t) pin, GPIO_INTR_POSEDGE);
    // gpio_install_isr_service(0);
    // gpio_isr_handler_add((gpio_num_t) pin, gpio_interrupt_handler, NULL);
}

void EasyButton::loop() {
    unsigned long now = millis();
    uint8_t state = gpio_get_level((gpio_num_t) PIN);

    /* Count clicks */
    if (state != lastBtnState && (now - lastBtnTime) > minClickTime) {
        if (state == 0) {
            multiClickCounter ++;
        }
        lastBtnState = state;
        lastBtnTime = now;
    }

    /* If time is up check click amount */
    if (((now - lastBtnTime) > multiClickMaxTime) && multiClickCounter > 0) {
        ButtonState btnState = IDLE;
        if (multiClickCounter == 1) {
            btnState = SINGLE_CLICK;
        } else if (multiClickCounter == 2) {
            btnState = DOUBLE_CLICK;
        } else if (multiClickCounter > 2) {
            btnState = MULTI_CLICK;
        }

        if (listener != NULL) listener(btnState);
        multiClickCounter = 0;
    }
}


void EasyButton::setOnClickListener(OnClickCallbackFunction f) {
    listener = f;
}


