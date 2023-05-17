void wifi_event(WiFiEvent_t event)
{
    switch (event) 
    {
        // event jika wifi terputus
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            system_log("WIFI", "menghubungkan kembali ...");

            // wifi menghubungkan kembali
            WiFi.begin(eeprom_read(wifi_ssid_addr).c_str(), eeprom_read(wifi_pass_addr).c_str());

            // update status
            wifi_is_connected = false;
        break;

        // event jika wifi sudah terhubung
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            system_log("WIFI", "terhubung");
            
            wifi_is_connected   = true;
            socket_is_connected = false;
            device_is_subscribe = false;
        break;
    }
}

