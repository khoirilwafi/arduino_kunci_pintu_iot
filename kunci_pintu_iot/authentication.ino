void login(void)
{
    // login menggunakan device_name dan device_pass
    String response = post_request("/login", "device_name=" + WiFi.macAddress() + "&device_pass=" + eeprom_read(login_pass_addr));

    if(response != "error")
    {
        // ubah data ke json
        JSONVar result = JSON.parse(response);
    
        // cek jika login sukses
        if(strcmp(result["status"], "success") == 0)
        { 
            eeprom_write(office_id_addr, (const char*) result["data"]["office_id"]);
            eeprom_write(door_id_addr, (const char*) result["data"]["door_id"]);
            eeprom_write(token_addr, (const char*) result["token"]);
    
            device_is_login = true;
        }
    }
}

void subscribe(void)
{
    String signature;

    system_log("SOCK", "mencoba subscribe ...");
    
    // request get signature
    String response = post_request("/get-signature", "socket_id=" + socket_id + "&office_id=" + eeprom_read(office_id_addr) + "&channel_data={\"user_id\":\"" + eeprom_read(door_id_addr) + "\",\"user_info\":true}");

    if(response != "error")
    {
        JSONVar result = JSON.parse(response);

        // jika belum login
        if(result.hasOwnProperty("message"))
        {
            device_is_login = false;
        }

        if(strcmp(result["status"], "success") == 0)
        {
            signature = (const char*) result["data"]["signature"];
            client.send("{\"event\":\"pusher:subscribe\",\"data\":{\"auth\":\"aNmB0bkbrE1PS6K07nrt:" + signature + "\",\"channel_data\":\"{\\\"user_id\\\":\\\"" + eeprom_read(door_id_addr) + "\\\",\\\"user_info\\\":true}\",\"channel\":\"presence-office." + eeprom_read(office_id_addr) + "\"}}");
        }
    }
}

bool device_register(String id)
{
    system_log("HTTP", "register pintu ...");

    String response = post_request("/register", "id=" + id + "&device_name=" + WiFi.macAddress());
    
    if(response != "error")
    {
        JSONVar result = JSON.parse(response);

        if(strcmp(result["status"], "success") == 0)
        {
            system_log("HTTP", "register berhasil");

            // update parameter
            eeprom_write(login_pass_addr, result["data"]["device_pass"]);
            eeprom_write(door_key_addr, result["data"]["key"]);
            return true;
        }
    }
    
    return false;
}

