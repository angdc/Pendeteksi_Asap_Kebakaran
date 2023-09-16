#include <ESP8266WiFi.h> //sambungin ke WIFI
#include <BlynkSimpleEsp8266.h> //sambungin ke Blynk
#include <DHT.h> // sambungin ke sensor suhu

#define DHTPIN D5          // Pin sensor DHT11 terhubung ke D5
#define DHTTYPE DHT11      // DHT 11 (sensor suhu)

char auth[] = "8trlr3idn7mAoct4eskqXItUcc7Fa6Z9"; //Untuk menyambungkan alat ke Blynk
char ssid[] = "ADC"; //Nama WIFI
char pass[] = "24096214"; //Password WIFI

int gasPin = A0;         // Pin sensor MQ-2 terhubung ke A0
int flamePin = D1;       // Pin sensor flame terhubung ke D1
int relayPin = D7;       // Pin relay terhubung ke D7
int buzzerPin = D2;      // Pin buzzer terhubung ke D2
int ledPin = D4;        // Pin lampu terhubung ke D4
int relayStatus = LOW;   // Status relay (awalnya mati)

int manualRelayStatus = LOW;  // Manual relay status (awalnya mati)

DHT dht(DHTPIN, DHTTYPE); // Declare DHT object

void setup()
{
  Serial.begin(9600); // Memunculkan serial di monitor 
  Blynk.begin(auth, ssid, pass);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  dht.begin(); // Instalasi Sensor DHT
}

void loop()
{
  Blynk.run();

  // Membaca suhu dan kelembaban dari Sensor DHT
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Mengirim suhu dan kelembaban nilai ke aplikasi Blynk 
  Blynk.virtualWrite(V5, temperature);
  Blynk.virtualWrite(V6, humidity);

  int gasValue = analogRead(gasPin); // Membaca nilai sensor gas dari A0
  Serial.print("Gas Value: ");
  Serial.println(gasValue); // Mencetak nilai gas ke Serial Monitor

  int flameValue = analogRead(flamePin); // Membaca sensor api dari D1

  int fuzzyGasOutput = fuzzyMamdaniGas(gasValue);
  int fuzzyFlameOutput = fuzzyMamdaniFlame(flameValue);

  // Logika fuzzy mengontrol sensor gas 
  if (fuzzyGasOutput == 1) {
    // Apabila level gas adalah HIGH (Tinggi) 
    relayStatus = HIGH;
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V0, "Gas Terdeteksi!");
    Blynk.logEvent("gas", "Gas Terdeteksi!");

  } else if (fuzzyFlameOutput == 1) {
    // Apabila api mendeteksi level HIGH (Tinggi) 
    relayStatus = HIGH;
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V0, "Api Terdeteksi!");
    Blynk.logEvent("flame", "Api Terdeteksi!");

  } else {
    relayStatus = LOW;
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
    Blynk.virtualWrite(V1, 0);
  }

  // Untuk ON-Off manual Relay 
  if (manualRelayStatus == HIGH) {
    relayStatus = manualRelayStatus;
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V0, "Relay activated manually");
  }

  digitalWrite(relayPin, relayStatus);

  delay(1000);
}


int fuzzyMamdaniGas(int gasValue)
{
  // Tentukan perangkat linguistik dan variabel linguistik
  
  // Set linguistik untuk gasValue
  int low = 0;
  int high = 0;
  
  // Mendefinisikan batas-batas himpunan linguistik
  int lowThreshold = 0;   // Untuk GasLow
  int highThreshold = 200;  // Untuk GasHigh
  
  // Fuzzifikasi
  if (gasValue < lowThreshold) {
    low = 1;
  } else if (gasValue >= lowThreshold && gasValue < highThreshold) {
    high = (float)(gasValue - lowThreshold) / (highThreshold - lowThreshold); // Linear membership function
  } else {
    high = 1;
  }
  
  // Aturan Fuzzy (Inferensi)
  
  // Fuzzy Rule 1: If gasValue is low, then output = 0
  int rule1Output = low;
  
  // Fuzzy Rule 2: If gasValue is high, then output = 1
  int rule2Output = high;
  
  // Defuzzifikasi
  
  // Using the MAX method to get crisp output
  int fuzzyOutput = max(rule1Output, rule2Output);
  
  // Convert to 0 or 1
  if (fuzzyOutput >= 0.5) {
    fuzzyOutput = 1;
  } else {
    fuzzyOutput = 0;
  }
  
  return fuzzyOutput;
}

int fuzzyMamdaniFlame(int flameValue)
{
    // Himpunan linguistik untuk flameValue
  int low = 0;
  int medium = 0;
  int high = 0;
  
  // Tentukan batas-batas himpunan linguistik
  int lowThreshold = 100;
  int highThreshold = 500;
  
  // Fuzzifikasi
  if (flameValue < lowThreshold) {
    low = 1;
  } else if (flameValue >= lowThreshold && flameValue < highThreshold) {
    medium = 0.5;
  } else {
    high = 0;
  }
  
  // Tentukan aturan fuzzy
  
  // Aturan fuzzy 1: Jika flameValue rendah, maka output = 0
  int rule1Output = low;
  
  // Aturan fuzzy 2: Jika flameValue sedang, maka output = 0.5
  int rule2Output = medium;
  
  // Aturan fuzzy 3: Jika flameValue tinggi, maka output = 1
  int rule3Output = high;
  
  // Defuzzifikasi
  
  // Menggunakan metode MAX untuk mendapatkan nilai tegas
  int fuzzyOutput = max(max(rule1Output, rule2Output), rule3Output);
  
  // Konversi menjadi 0 atau 1
  if (fuzzyOutput >= 0.5) {
    fuzzyOutput = 1;
  } else {
    fuzzyOutput = 0;
  }
  
  return fuzzyOutput;
}

BLYNK_WRITE(V4)
{
  manualRelayStatus = param.asInt();
  if (manualRelayStatus == HIGH) {
    Blynk.virtualWrite(V0, "Relay activated manually");
  } else {
    Blynk.virtualWrite(V0, "Relay deactivated manually");
  }
}
