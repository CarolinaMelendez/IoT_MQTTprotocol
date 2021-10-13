#include <WiFi.h>
#include <PubSubClient.h>

const int ECHO_PIN = 26;
const int TRIGGER_PIN = 27;

const char* WIFI_SSID = "FamT&A";
const char* WIFI_PASS = "40186467";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const int LED_RED = 14;

const char* CLIENT_ID = "Cliente_ucb_Grupo2_57469"; // unique client id

// Topics of Leds
const char* IN_TOPIC_LED_BLUE = "ucb/PruebaArduino/Grupo5/led_Blue";
const char* IN_TOPIC_LED_WHITE = "ucb/PruebaArduino/Grupo5/led_White";

// Topic of sensor
const char* OUT_TOPIC_SENSOR_GRAPH = "ucb/PruebaArduino/Grupo5/sensor";
const char* OUT_TOPIC_SENSOR_TEXT = "ucb/PruebaArduino/Grupo5/sensor_text";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  // Receive message
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }
  
  if(String(topic) != OUT_TOPIC_SENSOR_GRAPH && String(topic) != OUT_TOPIC_SENSOR_TEXT ){
    Serial.println("Message from topic " + String(topic) + ":" + message);
  }
  
  takeActions_onLeds(topic,message);
  
  int cm = 0.01723 * readUltrasonicDistance(TRIGGER_PIN, ECHO_PIN);
  publishMessages_Distance_toGraph(cm);
  publishMessage_Warning(cm);

}
void takeActions_onLeds(const char* topic, String message ){
  int message_int =  message.toInt();
  if (String(topic) == IN_TOPIC_LED_WHITE) {
    actionOnLed(message_int);
  }
  if (String(topic) == IN_TOPIC_LED_BLUE) {
    actionOnLed(message_int);
  }
}

void publishMessages_Distance_toGraph(int cm){
  String cm_str = String(cm);
  mqttClient.publish(OUT_TOPIC_SENSOR_GRAPH, cm_str.c_str());
}

void publishMessage_Warning(int cm){
  String messageWarning;
  if (cm <  15 ){
    turnOnLed(LED_RED); 
    messageWarning = "An object is too close ¡¡ Be careful ¡¡ \n Red led is ON ";
  }else{
    turnOffLed(LED_RED); 
    messageWarning = " Red led is OFF ";
  }
  mqttClient.publish(OUT_TOPIC_SENSOR_TEXT, messageWarning.c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while(1) delay(100);
  }
  Serial.println("Connected to " + String(WIFI_SSID));

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)){
    Serial.println("Connected to " + String(MQTT_BROKER));

    mqttClient.subscribe(IN_TOPIC_LED_BLUE);
    mqttClient.subscribe(IN_TOPIC_LED_WHITE);
    mqttClient.subscribe(OUT_TOPIC_SENSOR_GRAPH);
    mqttClient.subscribe(OUT_TOPIC_SENSOR_TEXT);
    Serial.println("Subscribed to topics ");
  } else {
    Serial.println("Couldn't connect to MQTT broker.");
  }
  return mqttClient.connected();
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

void loop(){
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    // Connect to MQTT broker
    if (now - previousConnectMillis >= 5000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) {
        previousConnectMillis = 0;
      } else {
        delay(1000);
        }
    }
  } else {
    // This should be called regularly to allow the client to process incoming 
    // messages and maintain its connection to the server
    mqttClient.loop(); // Here go to function "callback"
    delay(100);
  }
}

void turnOnLed(int ledPin)
{
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
}
void turnOffLed(int ledPin)
{
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
}

void actionOnLed(int ledPin)
{
  if (ledPin > 0){
    turnOnLed(ledPin);
  }else{
    ledPin = ledPin * -1 ;
    turnOffLed(ledPin);
  }
}

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  //-Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  //- Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}
