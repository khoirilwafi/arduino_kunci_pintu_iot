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

#define wifi_ssid_addr  0   // 21 char
#define wifi_pass_addr  21  // 21 char
#define login_pass_addr 42  // 21 char
#define office_id_addr  79  // 37 char
#define door_id_addr    116 // 37 char
#define door_key_addr   153 // 21 char
#define token_addr      174 // 62 char

String socket_id = "";
String actor_id = "";

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

const char server_url [] PROGMEM = "https://smartdoorlock.my.id";
const char socket_url [] PROGMEM = "wss://door.smartdoorlock.my.id";

// tick millis
uint32_t led_wifi_blink = 0;
uint32_t socket_ping_interval = 0;
uint32_t socket_connect_interval = 0;

// status holder
bool device_is_login     = true;
bool lock_status_change  = true;
bool wifi_is_connected   = false;
bool socket_is_connected = false;
bool device_is_subscribe = false;


void setup(void)
{
    // inisialisasi peripheral
    Serial.begin(115200);
    EEPROM.begin(250);
    delay(500);

    system_log("SYST", "mengatur peripheral ...");

    pinMode(wifi_status,  OUTPUT);
    pinMode(data_status,  OUTPUT);
    pinMode(error_status, OUTPUT);

    actor_id = eeprom_read(door_id_addr);
    
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
    WiFi.begin(eeprom_read(wifi_ssid_addr).c_str(), eeprom_read(wifi_pass_addr).c_str());

    // led_wifi_blink = millis();
    // socket_ping_interval = millis();
    // socket_connect_interval = millis();

//    while(WiFi.status() != WL_CONNECTED)
//    {
//        Serial.println("-");
//        delay(500);
//    }
//
//    device_register("c4c95198-a42e-48e6-9f9e-074af393db78");
//
//    while(1);
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

    // login jika unathorized
    if(wifi_is_connected == true && device_is_login == false && (millis() - socket_connect_interval) > 3000)
    {
        system_log("HTTP", "mencoba login ...");
        login();
        socket_connect_interval = millis();
    }

    // memulai koneksi websocket
    if(wifi_is_connected == true && socket_is_connected == false && (millis() - socket_connect_interval) > 3000)
    {
        digitalWrite(data_status, HIGH);

        system_log("SOCK", "menghubungkan ...");

        // coba hubungkan ke server websocket
        client.close();
        socket_is_connected = client.connect("ws://192.168.43.156:6001/app/aNmB0bkbrE1PS6K07nrt");
        socket_connect_interval = millis();

        digitalWrite(data_status, LOW);
    }

    // websocket pooling
    if(client.available()) 
    {
        client.poll();
    }

    // kirim pesan ping ke server setiap 30 detik (protokol pusher)
    if(wifi_is_connected == true && socket_is_connected == true && (millis() - socket_ping_interval) > 30000)
    {
        digitalWrite(data_status, HIGH);
        
        // kirimkan ping
        client.send("{\"event\":\"pusher:ping\",\"data\":{}}");
        socket_ping_interval = millis();

        digitalWrite(data_status, LOW);
    }

    // subscribe ke channel
    if(wifi_is_connected == true && device_is_login == true && socket_is_connected == true && device_is_subscribe == false && (millis() - socket_connect_interval) > 3000)
    {
        subscribe();
        socket_connect_interval = millis();
    }

    // update status door lock
    if(wifi_is_connected == true && device_is_login == true && socket_is_connected == true && device_is_subscribe == true && lock_status_change == true)
    {
        lock_status_change = !update_door_status(actor_id);
    }
}


