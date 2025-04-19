

void makeHTTPRequest()
{
    Serial.println(F("\nSunucuya bağlanılıyor..."));

    if (!client.connect(server, 80))
    {
        Serial.println(F("Sunucuya bağlanılamadı!"));
        return;
    }

    Serial.println(F("Bağlandı!"));

    // HTTP GET isteği
    client.println("GET /api/gate/str HTTP/1.1");
    client.println("Host: otoparkapi2.hayrat.dev");
    client.println("Connection: close");
    client.println();

    // Yanıt gelene kadar bekle
    unsigned long timeout = millis();
    while (!client.available())
    {
        if (millis() - timeout > 5000)
        {
            Serial.println(F(">>> Client Timeout !"));
            client.stop();
            return;
        }
    }

    String response = "";
    while (client.available())
    {
        String line = client.readStringUntil('\r');
        response += line;
        Serial.print(line);
    }

    int dataIndex = response.indexOf("x-gate-data:");
    if (dataIndex >= 0)
    {
        String gateData = response.substring(dataIndex + 12); // "x-gate-data:" uzunluğu 12

        // Yanıtı işle
        sscanf(gateData.c_str(), "%d:%d:%d;%d:%d:%d;%d:%d:%d",
               &kapaliId, &kapaliAcikmi, &kapaliDoluMu,
               &bahceId, &bahceAcikmi, &bahceDoluMu,
               &konakId, &konakAcikmi, &konakDoluMu);
        // Değerleri kontrol et
        Serial.print(F("Kapali ID: "));
        Serial.println(kapaliId);
        Serial.print(F("Kapali Acik mi: "));
        Serial.println(kapaliAcikmi == 1 ? F("Açık") : F("Kapalı"));
        Serial.print(F("Kapali Dolu mu: "));
        Serial.println(kapaliDoluMu == 1 ? F("Dolu") : F("Boş"));

        Serial.print(F("Konak ID: "));
        Serial.println(konakId);
        Serial.print(F("Konak Acik mi: "));
        Serial.println(konakAcikmi == 1 ? F("Açık") : F("Kapalı"));
        Serial.print(F("Konak Dolu mu: "));
        Serial.println(konakDoluMu == 1 ? F("Dolu") : F("Boş"));
        lastResponseTime = millis();

        // Kapıyı aç - burada kontrol ekleyebilirsin
        if (konakAcikmi == 1 && konakDoluMu == 0)
        {
            Serial.println(F("Konak otopark kapısı açılıyor..."));
            otoparkKapiAc(konakOtopark);
        }
    }
    else
    {
        Serial.println(F("Gate veri formatı bulunamadı!"));
    }

    // Kapıyı aç

    Serial.println(F("Bağlantı kapatıldı"));
    client.stop();
}

void loop()
{
    uint32_t currentTime = millis();

    if (currentTime - getTimer > 2000)
    {

        getTimer = currentTime;

        makeHTTPRequest();
    }

    // Yanıt alınmadıysa ve zaman aşımı olduysa, yeniden başlat
    if (currentTime - lastResponseTime > RESPONSE_TIMEOUT)
    {
        Serial.println(F("Response timeout, restarting..."));
        wdt_enable(WDTO_15MS);
        while (1)
        {
        } // Watchdog'un cihazı yeniden başlatmasını bekle
    }

    wdt_reset(); // Her döngüde watchdog'u sıfırla
}