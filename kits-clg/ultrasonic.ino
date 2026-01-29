/**** ESP32 Ultrasonic + Firebase (FIXED AUTH) *******/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* ================= WIFI DETAILS ================= */
#define WIFI_SSID       "raspberry"
#define WIFI_PASSWORD   "123456789"

/* ================= FIREBASE DETAILS ================= */
#define API_KEY         "AIzaSyA3NluveWnDnGu549rP_vUbWaQRIfyTS80"
#define DATABASE_URL    "https://kitsultrasonic-default-rtdb.firebaseio.com/"

/* ================= ULTRASONIC PINS ================= */
#define TRIG_PIN 18
#define ECHO_PIN 19

/* ================= VARIABLES ================= */
long duration;
float distance;

/* ================= FIREBASE OBJECTS ================= */
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

/* ===== TOKEN STATUS CALLBACK ===== */
void tokenStatusCallback(TokenInfo info) {
  Serial.print("Token status: ");
  Serial.println(info.status == token_status_ready ? "READY" : "NOT READY");
}

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
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  /* ===== ANONYMOUS SIGN-UP (CRITICAL FIX) ===== */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    signupOK = true;
  } else {
    Serial.print("SignUp Error: ");
    Serial.println(config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
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
    if (signupOK && Firebase.ready()) {
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
