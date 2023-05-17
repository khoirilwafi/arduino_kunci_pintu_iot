void websocket_event(WebsocketsEvent event, String data) 
{
    if(event == WebsocketsEvent::ConnectionClosed) 
    {
        system_log("SOCK", "terputus");
        socket_is_connected = false;
    }
}

void websocket_message(WebsocketsMessage websocket_message) 
{
    // ubah ke bentuk json
    JSONVar message = JSON.parse(websocket_message.data());

    // periksa event pusher
    if(strcmp(message["event"], "pusher:connection_established") == 0)
    {
        // update nilai socket_id
        JSONVar pusher_data = JSON.parse((const char*) message["data"]);
        socket_id = (const char*) pusher_data["socket_id"];

        // update status
        system_log("SOCK", "socket terhubung");
        socket_is_connected = true;
    }

    // subscribe berhasil
    else if(strcmp(message["event"], "pusher_internal:subscription_succeeded") == 0)
    {
        system_log("SOCK", "subscribe berhasil");
        device_is_subscribe = true;
    }

    // {"channel":"presence-office.ac132825-ac3a-42f1-b7d8-4300fd090125","event":"door-command","data":"{\"user_id\":\"e08fde8e-af6b-41a4-b9dd-176de7d0a796\",\"door_id\":\"c4c95198-a42e-48e6-9f9e-074af393db78\",\"locking\":\"open\",\"key\":\"ew7t7lgbu4ogqifd21ti\"}"}

    // door command
    else if(strcmp(message["event"], "door-command") == 0)
    {
        system_log("DOOR", "perintah masuk");

        // ambil data
        JSONVar pusher_data = JSON.parse((const char*) message["data"]);

        // proses perintah
        proses_perintah((const char*)pusher_data["door_id"], (const char*)pusher_data["user_id"], (const char*)pusher_data["locking"], (const char*)pusher_data["key"]);
    }

    // pesan pong
    else if(strcmp(message["event"], "pusher:pong") == 0)
    {
        system_log("SOCK", "pesan pong");
    }
}

