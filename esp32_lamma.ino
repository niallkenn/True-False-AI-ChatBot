#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "eir89830742";
const char* password = "BTR84CX3wE";

const char* serverIP = "192.168.1.103";
const int serverPort = 11434;

const int redPin = 32;
const int greenPin = 13;

void showAnswer(bool answer) {
  if (answer) {
    digitalWrite(greenPin, HIGH);
    delay(4000);
    digitalWrite(greenPin, LOW);
  } else {
    digitalWrite(redPin, HIGH);
    delay(4000);
    digitalWrite(redPin, LOW);
  }
}

void askLlama(String prompt) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    JsonDocument doc;

    String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/chat";

    http.begin(url);

    http.useHTTP10(true);

    http.setTimeout(20000);

    http.addHeader("content-type", "application/json");

    doc["model"] = "llama3.2:1b";
    doc["stream"] = false;

    JsonArray messages = doc["messages"].to<JsonArray>();
    JsonObject msg1 = messages.add<JsonObject>();
    msg1["role"] = "user";
    msg1["content"] = prompt;

    JsonObject format = doc["format"].to<JsonObject>();
    format["type"] = "object";

    JsonObject properties = format["properties"].to<JsonObject>();
    JsonObject answerProp = properties["answer"].to<JsonObject>();
    answerProp["type"] = "boolean";

    JsonArray required = format["required"].to<JsonArray>();
    required.add("answer");

    String requestBody;
    serializeJson(doc, requestBody);

    int responseCode = http.POST(requestBody);
    
    if (responseCode > 0) {
      String responseBody = http.getString();

      JsonDocument responseDoc;
      DeserializationError err1 = deserializeJson(responseDoc, responseBody);

      if (err1) {
        Serial.print("outer json failed: ");
        Serial.println(err1.c_str());

        return;
      }

      const char* content = responseDoc["message"]["content"];

      JsonDocument innerDoc;
      DeserializationError err2 = deserializeJson(innerDoc, content);

      if (err2) {
        Serial.print("inner json faild: ");
        Serial.println(err2.c_str());
        return;
      }
      bool answer = innerDoc["answer"];

      showAnswer(answer);
    } else {
      Serial.println(http.errorToString(responseCode));
    }

    http.end();
  } else {
    Serial.println("Wifi not connected.");
  }
}

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("connecting...");
    delay(1000);
  }
  Serial.println("\nConnected!!");
}

void loop() {
  String prompt = Serial.readStringUntil('\n');

  prompt.trim();
  if (prompt.length() > 0) {
      askLlama(prompt);
  }
}