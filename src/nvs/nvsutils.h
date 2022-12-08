/*
 * nvsutils.h
 *
 * Created on Sun Oct 23 2022
 *
 * Copyright (c) 2022 Your Company
 * 
 * Based on:
 *  https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/CPPNVS.cpp
 *  https://github.com/espressif/esp-idf/blob/0a678ebe8c8bb78053f2a2dc4bd26124adde9122/examples/storage/nvs_rw_blob/main/nvs_rw_blob.c
 */

#ifndef _NVS_UTILS_H_
#define _NVS_UTILS_H_
#include <nvs.h>
#include <string>

class NVSUtils {

private:
    std::string mNamespace;
    nvs_open_mode mMode;
    nvs_handle_t mHandle;

public:

    /**
     * @brief Constructor
     * 
     * @param[in] name The namespace
     * @param[in] mode The open mode (NVS_READWRITE, NVS_READONLY)
     */
    NVSUtils(std::string name, nvs_open_mode mode=NVS_READWRITE);

    /**
     * @brief Destructor
     */
    ~NVSUtils();

    /**
     * @brief Init NVS handle
     */
    esp_err_t init();

    /**
     * @brief Store string data by key
     * 
     * @param[in] key   The key (whats the name of saved data)
     * @param[in] data  The data to save
     */
    void set(std::string key, std::string data);

    /**
     * @brief Store uint8_t blob by key
     * 
     * @param[in] key   The key (whats the name of saved data)
     * @param[in] data  The uint8_t blob data to save
     */
    void set(std::string key, uint8_t* data_blob, size_t length);

    /**
     * @brief Get string data by key
     * 
     * @param[in]   key     The key (whats the name of saved data)
     * @param[out]  result  The string read from NVS storage
     */
    int get(std::string key, std::string* result);\

    /**
     * @brief Get uint8_t blob data by key
     * 
     * @param[in]   key     The key (whats the name of saved data)
     * @param[out]  result  The uint8_t blob read from NVS storage
     * @param[in]   length  A non-zero pointer to the variable holding the length of out_value.
     */
    int get(std::string key, uint8_t* result_blob, size_t &length);

    /**
     * @brief Commit (save) namespace
     */
    esp_err_t commit();

    /**
     * @brief Close handle
     */
    void close();

};

#endif
