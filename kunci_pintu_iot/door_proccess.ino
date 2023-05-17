bool update_door_status(String actor_id)
{
    system_log("HTTP", "update status pintu ...");

    // baca kondisi pintu
    uint8_t door_status = digitalRead(error_status);

    // kirim data
    String response = post_request("/update-status", "door_id=" + eeprom_read(door_id_addr) + "&office_id=" + eeprom_read(office_id_addr) + "&socket_id=" + socket_id + "&lock_status=" + door_status + "&user_id=" + actor_id);

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
            system_log("HTTP", "update status berhasil");
            
            eeprom_write(door_key_addr, result["data"]["key"]);
            return true;
        }
    }
    
    return false;
}

void proses_perintah(String door_id, String user_id, String command, String key)
{
    // cek apakah perintah untuk pintu ini
    if(door_id == eeprom_read(door_id_addr) && key == eeprom_read(door_key_addr))
    {
        system_log("DOOR", "validasi perintah sukses");

        // laksanakan perintah
        if(command == "open")
        {
            servo_open();
        }
        else if(command == "lock")
        {
            servo_close();
        }

        // update kondisi pintu
        actor_id = user_id;
        lock_status_change = true;
    }
}

