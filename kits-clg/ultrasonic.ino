/**** ESP32 Ultrasonic + Firebase (FIXED AUTH) *******/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>


// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* ================= WIFI DETAILS ================= */
#define WIFI_SSID       "rajasri28"
#define WIFI_PASSWORD   "123456789"

/* ================= FIREBASE DETAILS ================= */
#define API_KEY         "AIzaSyAPVFIlUk9g59BWUIoi7PASjd4rqJ4QEfs"
#define DATABASE_URL    "https://kits-c6149-default-rtdb.firebaseio.com"

/* ================= ULTRASONIC PINS ================= */
#define TRIG_PIN 25
#define ECHO_PIN 26


#define USER_EMAIL "example@gmail.com"
#define USER_PASSWORD "123456789"

/* ================= VARIABLES ================= */
long duration;
float distance;

/* ================= FIREBASE OBJECTS ================= */
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;


void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  /* ===== WIFI ===== */
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* ===== FIREBASE CONFIG ===== */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;
   config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h


  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
}

void loop() {

  /* ===== ULTRASONIC ===== */
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    Serial.println("No Echo Received");
  } else {
    distance = (duration * 0.0343) / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    /* ===== FIREBASE SEND ===== */
    if (Firebase.ready()) {
      if (Firebase.RTDB.setFloat(&fbdo, "/ultrasonic/distance_cm", distance)) {
        Serial.println("Firebase Update OK ");
      } else {
        Serial.print("Firebase Error : ");
        Serial.println(fbdo.errorReason());
      }
    } else {
      Serial.println("Firebase not ready yet...");
    }
  }

  delay(2000);
}
