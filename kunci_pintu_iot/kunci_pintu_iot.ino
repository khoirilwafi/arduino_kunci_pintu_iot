#include <WiFi.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ArduinoWebsockets.h>


using namespace websockets;
WebsocketsClient client;

#define wifi_status     21
#define data_status     19
#define error_status    18

#define wifi_ssid_addr  0
#define wifi_pass_addr  30
#define login_pass_addr 60  
#define token_addr      90

const char* ssid = "OPPO A5s";
const char* pass = "helloworld";

String device_name = "cc:db:a7:5a:34:70";
String device_pass = "knti236qpdtwdvn8idu6";

String token     = "27|8eToR4pSmmsOQVRAtaspxRNaDekhDZ3ASoIBZNOX";
String office_id = "2b970e87-6968-428c-a616-27cd0fcb9498";
String door_id   = "e0f367fc-5ae5-456d-90ea-3824a6bc1938";
String socket_id = "";

// tick millis
uint32_t led_wifi_blink;
uint32_t socket_ping_interval;
uint32_t socket_connect_interval;

// status holder
bool device_is_login     = false;
bool wifi_is_connected   = false;
bool socket_is_connected = false;
bool device_is_subscribe = false;

const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
"-----END CERTIFICATE-----\n";


void eeprom_write(uint16_t address, String text)
{
    // hitung panjang data
    uint16_t text_length = text.length();

    // tulis data
    for (uint16_t i=0; i<text_length; i++) 
    {
        EEPROM.write(address + i, text[i]);
    }

    // tulis tanda akhir data
    EEPROM.write(address + text_length, '\0');

    // commit data
    EEPROM.commit();
}

String eeprom_read(uint16_t address)
{
    String text;
    char character;
    
    while((character = EEPROM.read(address)) != '\0') 
    {
        text += character;
        address++;
    }
    
    return text;
}

void system_log(String tag, String text)
{
    Serial.println("[" + tag + "] " + text);
}

String get_request(String endpoint)
{
    system_log("HTTP", "get /" + endpoint);
    
    // inisialisasi data dan objek
    HTTPClient http;
    String response;

    // mulai koneksi
    digitalWrite(data_status, HIGH);
    http.begin("https://smartdoorlock.my.id/door" + endpoint);

    // kirimkan request
    http.addHeader("Authorization", "Bearer " + eeprom_read(token_addr));
    http.addHeader("Accept", "application/json");
    int httpResponseCode = http.GET();

    system_log("HTTP", "response " + String(httpResponseCode));

    // olah respon
    if (httpResponseCode > 0) 
    {
        response = http.getString();
    } 
    else 
    {
        response = "error";
    }

    // request selesai
    http.end();
    digitalWrite(data_status, LOW);

    system_log("HTTP", "http closed");

    return response;
}

String post_request(String endpoint, String payload)
{
    system_log("HTTP", "post /" + endpoint);
    
    HTTPClient http;
    String response;

    digitalWrite(data_status, HIGH);
    http.begin("https://smartdoorlock.my.id/door" + endpoint);

    // kirimkan request
    http.addHeader("Authorization", "Bearer " + eeprom_read(token_addr));
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Accept", "application/json");
    int httpResponseCode = http.POST(payload);

    system_log("HTTP", "response " + String(httpResponseCode));

    if (httpResponseCode > 0) 
    {
        response = http.getString();
    } 
    else 
    {
        response = "error";
    }

    http.end();
    digitalWrite(data_status, LOW);

    system_log("HTTP", "http closed");

    return response;
}

bool login(String username, String password)
{
    // login menggunakan device_name dan device_pass
    String response = post_request("/login", "device_name=" + ESP.getDeviceId() + "&device_pass=" + password);

    if(response == "error")
    {
        return false;
    }

    // ubah data ke json
    JSONVar result = JSON.parse(response);

    // cek jika login sukses
    if(strcmp(result["status"], "success") == 0)
    {
        office_id = (const char*) result["data"]["office_id"];
        door_id   = (const char*) result["data"]["door_id"];
        token     = (const char*) result["token"];

        return true;
    }

    digitalWrite(wifi_status, LOW);
    return false;
}

void wifi_event(WiFiEvent_t event)
{
    switch (event) 
    {
        // event jika wifi terputus
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            system_log("WIFI", "menghubungkan kembali ...");

            // wifi menghubungkan kembali
            WiFi.begin(ssid, pass);
            wifi_is_connected = false;
        break;

        // event jika wifi sudah terhubung
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            system_log("WIFI", "terhubung");

            // setting status holder
            wifi_is_connected   = true;
            socket_is_connected = false;
            device_is_subscribe = false;
        break;
    }
}

void websocket_event(WebsocketsEvent event, String data) 
{
    if(event == WebsocketsEvent::ConnectionOpened) 
    {
        system_log("SOCK", "terhubung");
        socket_is_connected = true;
    } 
    else if(event == WebsocketsEvent::ConnectionClosed) 
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
        String id = (const char*) pusher_data["socket_id"];
        socket_id = id.substring(1, id.length()-1);
    }
    else if(strcmp(message["event"], "pusher:pong") == 0)
    {
        system_log("SOCK", "pesan pong");
    }

    Serial.println(websocket_message.data());
}


void setup(void)
{
    // inisialisasi peripheral
    Serial.begin(115200);
    EEPROM.begin(175);
    delay(500);

    system_log("SYST", "mengatur peripheral ...");

    pinMode(wifi_status,  OUTPUT);
    pinMode(data_status,  OUTPUT);
    pinMode(error_status, OUTPUT);
    
    system_log("SYST", "selesai");
    
    // hapus konfigurasi lama dan daftarkan event handler
    WiFi.disconnect(true);
    WiFi.onEvent(wifi_event);

    // websocket handler
    client.onMessage(websocket_message);
    client.onEvent(websocket_event);

    // sertifikat untuk ssl
    client.setCACert(echo_org_ssl_ca_cert);

    system_log("WIFI", "memulai koneksi ...");

    // hubungkan ke akses point
    WiFi.begin(ssid, pass);

    led_wifi_blink = millis();
    socket_ping_interval = millis();
    socket_connect_interval = millis();
}

void loop(void)
{
    // led wifi kedip jika wifi terputus
    if(wifi_is_connected == false && (millis() - led_wifi_blink) > 500)
    {
        digitalWrite(wifi_status, !digitalRead(wifi_status));
        led_wifi_blink = millis();
    }

    // led wifi menyala jika wifi terhubung
    if(wifi_is_connected == true)
    {
        digitalWrite(wifi_status, HIGH);
    }

    // websocket pooling
    if(client.available()) 
    {
        client.poll();
    }

    // kirim pesan ping ke server setiap 30 detik (protokol pusher)
    if(client.available() && socket_is_connected == true && (millis() - socket_ping_interval) > 30000)
    {
        digitalWrite(data_status, HIGH);
        
        // kirimkan ping
        client.send("{\"event\":\"pusher:ping\",\"data\":{}}");
        socket_ping_interval = millis();

        digitalWrite(data_status, LOW);
    }

    // memulai koneksi websocket
    if(socket_is_connected == false && wifi_is_connected == true && (millis() - socket_connect_interval) > 1000)
    {
        digitalWrite(data_status, HIGH);

        system_log("SOCK", "menghubungkan ...");

        // coba hubungkan ke server websocket
        socket_is_connected = client.connect("wss://door.smartdoorlock.my.id/app/aNmB0bkbrE1PS6K07nrt");
        socket_connect_interval = millis();

        digitalWrite(data_status, LOW);
    }

    // subscribe ke channel
    if(device_is_login == true && socket_is_connected == true && device_is_subscribe == false)
    {
        
    }
}


