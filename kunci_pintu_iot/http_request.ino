String get_request(String endpoint)
{
    // inisialisasi data dan objek
    HTTPClient http;
    String response;

    // mulai koneksi
    digitalWrite(data_status, HIGH);
    http.begin("http://192.168.43.156:8000/door" + endpoint);

    // kirimkan request
    http.addHeader("Authorization", "Bearer " + eeprom_read(token_addr));
    http.addHeader("Accept", "application/json");
    int httpResponseCode = http.GET();

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

    return response;
}

String post_request(String endpoint, String payload)
{
    HTTPClient http;
    String response;

    digitalWrite(data_status, HIGH);
    http.begin("http://192.168.43.156:8000/door" + endpoint);

    // kirimkan request
    http.addHeader("Authorization", "Bearer " + eeprom_read(token_addr));
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Accept", "application/json");
    int httpResponseCode = http.POST(payload);

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

    return response;
}

