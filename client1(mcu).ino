#include <DHT.h> // Including library for dht
// #include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#define DHTPIN 0 //pin where the dht11 is connected

String apiKey = "WH44RPOBWB81KXHN";            //  Enter your Write API key from ThingSpeak
const char ssid[] = "No free wifi for you :P"; // replace with your wifi ssid and wpa2 key
const char pass[] = "srini@123456789";
const char server[] = "api.thingspeak.com";
//for flow sensor
#define SENSOR 12

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
// boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

DHT dht(DHTPIN, DHT11);
// LiquidCrystal_I2C lcd(0x27, 16, 2);
//for waterlevel
#define sensorPower 13
#define sensorPin A0
int val = 0;

WiFiClient client;
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}
void setup()
{

  Serial.begin(115200);
  // lcd.clear();
  // lcd.begin();
  // lcd.print("Initializing");
  Serial.println("Initializing");
  delay(10);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //for level sensor
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);

  //for flow SENSOR
  pinMode(SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop()
{

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    // Serial.println("Failed to read from DHT sensor!");
    dht.begin();
    // lcd.clear();
    delay(1000);
  }
  int level = readSensor();

  // lcd.setCursor(0, 0);
  // lcd.print("temp: ");
  // lcd.print(t);
  // lcd.setCursor(0, 1);
  // lcd.print("humidity: ");
  // lcd.print(h);

  //for flow rate
  pulse1Sec = pulseCount;
  pulseCount = 0;
  flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
  previousMillis = millis();
  flowMilliLitres = (flowRate / 60) * 1000;
  flowLitres = (flowRate / 60);
  totalMilliLitres += flowMilliLitres;
  totalLitres += flowLitres;

  if (client.connect(server, 80)) //   "184.106.153.149" or api.thingspeak.com
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(flowRate);
    postStr += "&field4=";
    postStr += String(level);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius, Humidity: ");
    Serial.print(h);
    Serial.println("%, Flow rate: ");
    Serial.print(flowRate);
    Serial.print("L/min, Water Level: ");
    Serial.println(level);
  }
  client.stop();

  Serial.println("Waiting...");

  // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
  delay(1000);
}

int readSensor()
{
  digitalWrite(sensorPower, HIGH); // Turn the sensor ON
  delay(100);                      // wait 10 milliseconds
  val = analogRead(sensorPin);     // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);  // Turn the sensor OFF
  // return val;                      // send current reading
  if (val < 100)
    return 0;
  if (val < 225)
    return 1;
  if (val < 250)
    return 2;
  else
    return 3;
}