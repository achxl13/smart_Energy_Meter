#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include "time.h"

// -------- PZEM SETUP --------
#define RXD2 16
#define TXD2 17
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);

// -------- WIFI --------
const char* ssid = "Achal_iPhone";
const char* password = "1234567890";

// -------- AWS ENDPOINT --------
const char* awsEndpoint = "aqd4fxvam3n0p-ats.iot.ap-south-1.amazonaws.com";

// -------- CERTIFICATES --------

// Root CA
const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Device Certificate
const char* client_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUUbu/0v8yQXt8l2/DT5oRLs5XI0swDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQxMjEzMTEz
NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOY2cyPVw9n1wnAa/E7I
BIDBxQf4VPPm302YawrZ70g6xrH0IW2ckT1KPH8wonOdWWqPWvLymsN/iL3e6d01
dkz0LagrE4UmSs2YuehMFskdKO7Yh+D0r5xj599+QbaXkcYMmDuWLVwx6ESvMA7A
0vqBgfz/Yr+QAsdrfKWtnDy+fn1sFec1HmDDJhU1QGbth4kRNX0xwyhex3P3Hu+s
6VVfrctNcpz/5SXVa2V/FbIiHzOhztkr1AseLdq12LDIKXfkCY+cQBlQRsA1Vtt2
Ls0HI8HbKgVTc8A4/6ke2CBA7JO88xcgdl6vFTDAbE6jywOu5XV26/ZQiDiiKZBW
cukCAwEAAaNgMF4wHwYDVR0jBBgwFoAUCwRCFpW6vtSODEbwbWWilZ5leE0wHQYD
VR0OBBYEFFTn7i8S/eCiR4TBaZSnmOHLrJGtMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBdE7A4Sly1bbTLZfe3vi0z4+kq
6Ez6SssdRftvWYvAcIpGs0aN5y91B61ZjuhnTN4MRlaUxRLnydvMHSFvViFs0qYw
FmrlW5YPcxBgH87StWsZJNrCdpoP3+EIFBtr1jBE28UVvGV3KmEp1ccDePvP0QOv
MZ3lh2ulGEXQa6z1wo+fNYqmuMmpRj/qnJSphOQNnOSm3w99q1tjbt58TR2QwodA
N6GIqbf945D9ELtUqJSs7TdmNI0G5GngM49suKsPgaOlT6LeafocP/kFypCdg7ws
Tz91lQoplhF58DgUsuuoT3mBe/aEXdR57PdsQiZ1GP78x3livQwwbt6QbNkx
-----END CERTIFICATE-----

)EOF";

// Private Key
const char* private_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA5jZzI9XD2fXCcBr8TsgEgMHFB/hU8+bfTZhrCtnvSDrGsfQh
bZyRPUo8fzCic51Zao9a8vKaw3+Ivd7p3TV2TPQtqCsThSZKzZi56EwWyR0o7tiH
4PSvnGPn335BtpeRxgyYO5YtXDHoRK8wDsDS+oGB/P9iv5ACx2t8pa2cPL5+fWwV
5zUeYMMmFTVAZu2HiRE1fTHDKF7Hc/ce76zpVV+ty01ynP/lJdVrZX8VsiIfM6HO
2SvUCx4t2rXYsMgpd+QJj5xAGVBGwDVW23YuzQcjwdsqBVNzwDj/qR7YIEDsk7zz
FyB2Xq8VMMBsTqPLA67ldXbr9lCIOKIpkFZy6QIDAQABAoIBAQDOz52SVMh0Nxas
1WFSeNeqv+h8Z3gSAJccw09uWShgEAVN+o5JRT0o/VbpwExAZtvcIdrHpHCqLhku
0WHwysVzDm0xxTjtBeBvYMOqTyitFZgfMcCns8lLSmkU2vy3xuQ7MjdUWG55g5Aw
m90lc85VIyEeomMYyovkkMhSomtYGOlf5qSyji9qxXyeyeGrPCPKws1Yh20oNYgz
W6YVP9/7ZqSAcrWDEQK9OCLoN7GIhYisWUJswHe0Bq0P7WUuw1XypHrRk1oi2wCt
Wi+YZDWU0Occ1k8E3OLVqGOmC0ugOFdKu+Ch1tVxjBTG3r39hlptBKdZnXKVkNiC
9xdGHjHdAoGBAPt5CtkHC/ViVfoqyyQ7t/UbaY/HLQ7NSKwEzMuRxYZUQBjkfeBs
n2zCFLmH3LXNm1f+V219hofeDEBcZkDEb1KJLpQVd+FUOOldqAuNp7Ltu0Sgld5r
Bhw4z6hTVQYuD1X+2GoTUruUCJI+O/CF/F+3QaRX42Pu4OKXwIKWS3N3AoGBAOpb
bSCkVZ/5RRjMwNZPZ4uxg7sSfy1Zq5H74d2LUQwACbcJ9NS2hmwTFwKgqmXo3rpv
BZanXZpABsmJGeSL44Wthe5dixJMGiUzTPnTLxi3ROW8arQW0UR5CQ7HfDcGvTPC
hZ3q+TFnhJtzGrxsRDGLUHsSSynZ9xWQmoGL6ySfAoGBAMDE0tn3GuWp0fqJX1uR
I70SnGMI7bNRDgo/S7YpMl98qf3q+ctVuQgn5rcqPLhoulJDuEVIWh5AyudzbTbs
3eFJI5mEs3bZlx7dGnpwxkLVFh7PctMi+3Ya3qRL7Ptlx3/GcaMu2NbN7ZwddNk1
/HZrBjiy3q62aVGJvprcF06VAoGBAOkw/d4cBuM+SmXJ3xugOR0EDlrHpeo/Lnv9
ERq79HInW4NZEdfSouDJAf1oVwU+eNkzfXERqg7BJL/GfxAtrcAnEHxFcgtL1Dwt
1qtDWrqSKakbGWWGysYCVWPyVQP/3aWyKfgfhcOGCKoRF4pjhFt0DynW4xmKu9Qm
Zcb/rYNrAoGAOZEyDPw33xapEIJdK6csc03/LyiCxBWbG/5RHofoArZDtn/VNIlk
47VEwlrgRSYTj9/JrLLl+N0ctANxLuuIdIVpHWOVt6YSLu3gMeHcfGcR8ejuk6Da
TRcdv+1tUod/yol44V7DNB6mHMq+TTSyc9fkJ3u03V7jwzW6YQVQSK8=
-----END RSA PRIVATE KEY-----
)EOF";

// -------- MQTT --------
WiFiClientSecure net;
PubSubClient client(net);

// -------- VARIABLES --------
float voltage, current, power, energy;

// -------- CONNECT AWS --------
void connectAWS() {

  net.setCACert(root_ca);
  net.setCertificate(client_cert);
  net.setPrivateKey(private_key);

  client.setServer(awsEndpoint, 8883);

  while (!client.connected()) {
    Serial.println("Connecting to AWS...");

    if (client.connect("ESP32_Client_001")) {
      Serial.println("CONNECTED TO AWS ✅");
    } else {
      Serial.print("FAILED, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  // Start PZEM
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected ✅");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // -------- TIME SYNC --------
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Syncing time");
  time_t now = time(nullptr);

  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\nTime synced ✅");

  // -------- CONNECT AWS --------
  connectAWS();
}

// -------- LOOP --------
void loop() {

  if (!client.connected()) {
    connectAWS();
  }

  client.loop();

  // -------- READ PZEM DATA --------
  voltage = pzem.voltage();
  current = pzem.current();
  power  = pzem.power();
  energy = pzem.energy(); // kWh

  // -------- VALIDATION --------
  if (isnan(voltage) || isnan(current) || isnan(power)) {
    Serial.println("Error reading PZEM data");
    delay(2000);
    return;
  }

  // -------- PRINT --------
  Serial.println("------ LIVE DATA ------");
  Serial.print("Voltage: "); Serial.println(voltage);
  Serial.print("Current: "); Serial.println(current);
  Serial.print("Power: "); Serial.println(power);
  Serial.print("Energy (Units): "); Serial.println(energy);

  // -------- JSON PAYLOAD --------
  String payload = "{";
  payload += "\"voltage\": " + String(voltage) + ",";
  payload += "\"current\": " + String(current) + ",";
  payload += "\"power\": " + String(power) + ",";
  payload += "\"units\": " + String(energy);
  payload += "}";

  // -------- SEND TO AWS --------
  client.publish("esp32/data", payload.c_str());

  Serial.println("Data sent to AWS 📡");

  delay(5000);
}