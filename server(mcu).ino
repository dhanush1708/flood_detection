#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <base64.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char ssid[] = "No free wifi for you :P";
const char pass[] = "srini@123456789";
WiFiClient client;

//---------Channel Details---------//
unsigned long int interval = 86400000, prevSent2 = 0, prevSent1 = 0;
unsigned long counterChannelNumber1 = 1209982;
unsigned long counterChannelNumber2 = 1217400;         // Channel ID
const char *myCounterReadAPIKey = "98D2M11Y3EXC3IZJ";  // Read API Key
const char *myCounterReadAPIKey1 = "V6CPNBLFP7DZUWW7"; // Read API Key
const int FieldNumber1 = 1;                            // The field you wish to read
const int FieldNumber2 = 2;
const int FieldNumber3 = 3;
const int FieldNumber4 = 4;
bool safe1 = true;
bool safe2 = true;
//nearby numbers
String nearby = "9502215191";
//-------------------------------//
const char *account_sid = "AC05464dcb1ee0cf3258172f62eeffd0aa";
const char *auth_token = "1de03482e67b470032c297801d658c4c";
String from_number = "+18312176586";
String to_number = "+919502215191";
String message_body1 = "Alert: Go to 2";
String message_body2 = "Alert: Go to 1";
const char fingerprint[] = "BC B0 1A 32 80 5D E6 E4 A2 29 66 2B 08 C8 E0 4C 45 29 3F D0";
String urlencode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            code2 = '\0';
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
        yield();
    }
    return encodedString;
}

String get_auth_header(const String &user, const String &password)
{
    size_t toencodeLen = user.length() + password.length() + 2;
    char toencode[toencodeLen];
    memset(toencode, 0, toencodeLen);
    snprintf(toencode, toencodeLen, "%s:%s", user.c_str(), password.c_str());
    String encoded = base64::encode((uint8_t *)toencode, toencodeLen - 1);
    String encoded_string = String(encoded);
    std::string::size_type i = 0;
    // Strip newlines (after every 72 characters in spec)
    while (i < encoded_string.length())
    {
        i = encoded_string.indexOf('\n', i);
        if (i == -1)
        {
            break;
        }
        encoded_string.remove(i, 1);
    }
    return "Authorization: Basic " + encoded_string;
}

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    ThingSpeak.begin(client);
    lcd.begin();
    lcd.clear();
    lcd.print("Initializing...");
}

void loop()
{

    Serial.println();
    //----------------- Network -----------------//
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Connecting to ");
        Serial.print(ssid);
        Serial.println(" ....");
        while (WiFi.status() != WL_CONNECTED)
        {
            WiFi.begin(ssid, pass);
            delay(5000);
        }
        Serial.println("Connected to Wi-Fi Succesfully.");
    }
    //--------- End of Network connection--------//
    Serial.println("For location 1:");
    //---------------- Channel 1 ----------------//
    float temp = ThingSpeak.readLongField(counterChannelNumber1, FieldNumber1, myCounterReadAPIKey);
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.println("°C");
    }
    else
    {
        Serial.println("Unable to read channel / No internet connection");
    }
    delay(100);
    //-------------- End of Channel 1 -------------//

    //---------------- Channel 2 ----------------//
    float humidity = ThingSpeak.readLongField(counterChannelNumber1, FieldNumber2, myCounterReadAPIKey);
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
    }
    else
    {
        Serial.println("Unable to read channel / No internet connection");
    }
    delay(100);
    //-------------- End of Channel 2 -------------//

    float flowRate1 = ThingSpeak.readLongField(counterChannelNumber1, FieldNumber3, myCounterReadAPIKey);
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.print("Water Flow Rate: ");
        Serial.print(flowRate1);
        Serial.println("L/min");
    }
    else
    {
        Serial.println("Unable to read channel / No internet connection");
    }
    delay(100);

    float waterLevel = ThingSpeak.readLongField(counterChannelNumber1, FieldNumber4, myCounterReadAPIKey);
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.print("Water Level: ");
        Serial.println(waterLevel);
    }
    else
    {
        Serial.println("Unable to read channel / No internet connection");
    }
    delay(100);
    Serial.println("\nFor Location 2:");
    float flowRate2 = ThingSpeak.readLongField(counterChannelNumber2, FieldNumber1, myCounterReadAPIKey1);
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.print("Water Flow Rate: ");
        Serial.print(flowRate2);
        Serial.println("L/min");
    }
    else
    {
        Serial.println("Unable to read channel / No internet connection");
    }
    delay(100);

    //temp,humidity,flowRate1,waterLevel,flowRate2

    if (waterLevel >= 2 || flowRate1 >= 10)
    {
        if (millis() - prevSent1 > interval || safe1)
        {
            sendsms(message_body1);
            prevSent1 = millis();
        }
        safe1 = false;
    }
    else
        safe1 = true;
    if (flowRate2 >= 10)
    {
        if (millis() - prevSent2 > interval || safe2)
        {
            sendsms(message_body2);
            prevSent2 = millis();
        }
        safe2 = false;
    }
    else
        safe2 = true;
    lcd.clear();
    if (safe1)
    {
        lcd.setCursor(0, 0);
        lcd.print("Place-1:SAFE");
    }
    else
    {
        lcd.setCursor(0, 0);
        lcd.print("Place-1:NOT SAFE");
    }
    if (safe2)
    {
        lcd.setCursor(0, 1);
        lcd.print("Place-2:SAFE");
    }
    else
    {
        lcd.setCursor(0, 1);
        lcd.print("Place-2:NOT SAFE");
    }
}

void sendsms(String message_body)
{
    WiFiClientSecure client;
    client.setFingerprint(fingerprint);
    Serial.printf("+ Using fingerprint '%s'\n", fingerprint);
    const char *host = "api.twilio.com";
    const int httpsPort = 443;
    Serial.print("+ Connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort))
    {
        Serial.println("- Connection failed.");
        return; // Skips to loop();
    }
    Serial.println("+ Connected.");
    Serial.println("+ Post an HTTP send SMS request.");
    String post_data = "To=" + urlencode(to_number) + "&From=" + urlencode(from_number) + "&Body=" + urlencode(message_body);
    String auth_header = get_auth_header(account_sid, auth_token);
    String http_request = "POST /2010-04-01/Accounts/" + String(account_sid) + "/Messages HTTP/1.1\r\n" + auth_header + "\r\n" + "Host: " + host + "\r\n" + "Cache-control: no-cache\r\n" + "User-Agent: ESP8266 Twilio Example\r\n" + "Content-Type: application/x-www-form-urlencoded\r\n" + "Content-Length: " + post_data.length() + "\r\n" + "Connection: close\r\n" + "\r\n" + post_data + "\r\n";
    client.println(http_request);
    // Read the response.
    String response = "";
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        response += (line);
        response += ("\r\n");
    }
    Serial.println("+ Connection is closed.");
    Serial.println("+ Response:");
    Serial.println(response);
};

