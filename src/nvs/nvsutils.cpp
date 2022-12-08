/*
 * nvsutils.cpp
 *
 * Created on Sun Oct 23 2022
 *
 * Copyright (c) 2022 Your Company
 */

#include "nvsutils.h"

#include <esp_log.h>
#include <esp_err.h>
#include <nvs_flash.h>


static const char TAG[] = "nvsutils";


NVSUtils::NVSUtils(std::string name, nvs_open_mode mode) {
    mNamespace = name;
    mMode = mode;
}

NVSUtils::~NVSUtils() {
    nvs_close(mHandle);
}

esp_err_t NVSUtils::init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(TAG, "ERROR: Earsing nvs: %d", err);
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open and attach handle
    err = nvs_open(mNamespace.c_str(), mMode, &mHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ERROR: Error while attaching handle: %d", err);
        return err;
    }

    return ESP_OK;
}

esp_err_t NVSUtils::commit() {
    // Save all changes
    esp_err_t res = nvs_commit(mHandle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "ERROR: %d", res);
        return res;
    }

    return ESP_OK;
}

void NVSUtils::close() {
    nvs_close(mHandle);
}

void NVSUtils::set(std::string key, std::string data) {
    ESP_LOGI(TAG, "Saving, key: %s, data: %s", key.c_str(), data.c_str());
    esp_err_t err = nvs_set_str(mHandle, key.c_str(), data.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "(set) Error while saving: %d", err);
    }
}

void NVSUtils::set(std::string key, uint8_t* data_blob, size_t length) {
    ESP_LOGI(TAG, "Saving, key: %s, blob_length: %d", key.c_str(), length);
    esp_err_t err = nvs_set_blob(mHandle, key.c_str(), data_blob, length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "(set) Error while saving (uint8_t_blob): %d", err);
    }
}

int NVSUtils::get(std::string key, std::string* result) {
    // Read the size of memory space required for blob
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(mHandle, key.c_str(), NULL, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "(get) Error while reading: %d", err);
        return err;
    }

    // Read if available
    char* data = (char*) malloc(required_size);
    nvs_get_str(mHandle, key.c_str(), data, &required_size);

    *result = std::string(data);
    free(data);

    return ESP_OK;
}

int NVSUtils::get(std::string key, uint8_t* result_blob, size_t &length) {
    esp_err_t err = nvs_get_blob(mHandle, key.c_str(), result_blob, &length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "(get) Error while reading (uint8_t_blob): %d", err);
        return err;
    }
    
    return ESP_OK;
}
