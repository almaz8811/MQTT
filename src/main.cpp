#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 2      // Назначить пин датчика температуры
#define DHTTYPE DHT22 // DHT 22, AM2302, AM2321

#define ssid "AlMaz"        // Имя вайфай точки доступа
#define password "Pk4CMqft" // Пароль от точки доступа

#define mqtt_server "tailor.cloudmqtt.com" // Имя сервера MQTT
#define mqtt_port 11995                    // Порт для подключения к серверу MQTT
#define mqtt_login "xnyqpfbu"              // Логин от сервер
#define mqtt_password "q94Tbl-0-WxH"       // Пароль от сервера
#define mqtt_topic "temp"

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0;

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic); // отправляем в монитор порта название топика
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  { // отправляем данные из топика
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  { // крутимся пока не подключемся.
    Serial.print("Attempting MQTT connection...");
    // создаем случайный идентификатор клиента
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // подключаемся, в client.connect передаем ID, логин и пасс
    if (client.connect(clientId.c_str(), mqtt_login, mqtt_password))
    {
      Serial.println("connected");  // если подключились
      client.subscribe(mqtt_topic); // подписываемся на топик, в который же пишем данные
    }
    else
    { // иначе ругаемся в монитор порта
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();                             // подключаемся к wifi
  client.setServer(mqtt_server, mqtt_port); // указываем адрес брокера и порт
  client.setCallback(callback);             // указываем функцию которая вызывается когда приходят данные от брокера
  dht.begin();
}

void loop()
{
  char msg[5];                                            // забераем температуру и конвертируем её в char
  float tmp = dht.readTemperature();  
  dtostrf(tmp, 6, 2, msg);
   
  if (!client.connected()) {                             // проверяем подключение к брокеру
    reconnect();                                            // еще бы проверить подкючение к wifi...
  }
  client.loop();

  long now = millis();                                   // каждые 10 секунд
  if (now - lastMsg > 10000) {
    lastMsg = now; 
    client.publish(mqtt_topic, msg);                     // пишем в топик 
  }
}