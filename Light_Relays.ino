/*
 * Creator: AKEB
 * Date: 13.06.2017 13:10:41
 * Encoding: UTF-8
 * COPYRIGHT 2017 AKEB.
 * Contact: akeb@akeb.ru
 * 
 */
#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>
#include <PubSubClient.h>
#include <EasyWebServer.h>

// Включить Вывод в консоль
#define DEBUG_LEVEL 1
// Порт для подключения по Telnet к Arduino, для просмотра логов для просмотра логов
#define DEBUG_SERVER_PORT 8000
// ПИН на который нужно подать питание при необходимости отключиться от сети!
#define DEBUG_PIN 14


// Pin для холодной воды
#define C_WATER_PIN1 15
#define C_WATER_PIN2 16

// Pin для горячей воды
#define H_WATER_PIN1 17
#define H_WATER_PIN2 18

// Pin для АКВА_СТОРОЖА
#define W_WATER_PIN1 19
#define W_WATER_PIN2 20
#define W_WATER_PIN3 21


// Кол-во попыток связаться с сервером, после чего система перейдет в Дебаг режим (Без сети)
#define SERVER_CONNECT_MAX_COUNT 5

// MAC Адрес Ethernet Shild Arduino MEGA
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// IP Адрес Arduino
IPAddress ip(192,168,1,100);

// Настройки сервера MQTT
//IPAddress mqtt_server(192,168,1,200);
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "lightshild"

// IP адрес сервера MajorDomo куда надо посылать команду на переключение света, при назатии на выключатель
char Server_IP[14] = "192.168.1.200";
const int Server_PORT = 80;

// Скрипты для переключения светильников, список взят из MajorDomo
const String Light_switch_url[] = {"/objects/?object=Light_01&op=m&m=switch&", "/objects/?object=Light_02&op=m&m=switch&", "/objects/?object=Light_03&op=m&m=switch&", "/objects/?object=Light_04&op=m&m=switch&", "/objects/?object=Light_05&op=m&m=switch&", "/objects/?object=Light_06&op=m&m=switch&", "/objects/?object=Light_07&op=m&m=switch&", 
                                   "/objects/?object=Light_08&op=m&m=switch&", "/objects/?object=Light_09&op=m&m=switch&", "/objects/?object=Light_10&op=m&m=switch&", "/objects/?object=Light_11&op=m&m=switch&", "/objects/?object=Light_12&op=m&m=switch&", "/objects/?object=Light_13&op=m&m=switch&", "/objects/?object=Light_14&op=m&m=switch&"};

// Пины для Вывода на РЕЛЕ
const int light[] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49};

// Пины для Выклюателей, должны быть подтянуты к земле через резистор 10K
const int btn[] =  {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48};

// Состояние лампочек
int light_state[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Текущее состояние выключателей
int btn_state_prev[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// переменные для эмитации команды delay
unsigned long last_millis_btn = 0;
unsigned long last_millis_reconnect = 0;
unsigned long last_millis_check = 0;
unsigned long last_millis_send_status = 0;
unsigned long last_millis_water = 0;

// Текущее кол-во попыток связаться с сервером
int check_cnt = 0;
// Максимальное кол-во попыток связаться с сервером
const int check_cnt_max = SERVER_CONNECT_MAX_COUNT; 

const int debug_pin =  DEBUG_PIN;
int debug_state = 0;

const int c_water_pin =  C_WATER_PIN1;
const int h_water_pin =  H_WATER_PIN1;
const int w_water_pin =  W_WATER_PIN1;

int c_water_state = 0;
int h_water_state = 0;
int w_water_state = 0;

int c_water = 0;
int h_water = 0;
int w_water = 0;

EthernetClient net;
EthernetClient mqtt_net;

EthernetServer server(DEBUG_SERVER_PORT);

EthernetServer server_http(80);

// Кол-во клиентов способных подключиться по telnet
EthernetClient clients[10];

PubSubClient client(mqtt_net);

String readString;

// Преобразование IP Адреса
String DisplayAddress(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}

// Функция вывода в лог
void debug_log(String str) {
  if (DEBUG_LEVEL) {
    Serial.println(str);
  }
  for (byte i = 0; i < 10; i++) {
    if (clients[i]) {
      clients[i].println(str);
    }
  }
}

// Function made to millis() be an optional parameter
char *uptime() {
 return (char *)uptime(millis()); // call original uptime function with unsigned long millis() value
}

char *uptime(unsigned long milli) {
 static char _return[32];
 unsigned long secs=milli/1000, mins=secs/60;
 unsigned int hours=mins/60, days=hours/24;
 milli-=secs*1000;
 secs-=mins*60;
 mins-=hours*60;
 hours-=days*24;
 sprintf(_return,"Uptime %d days %2.2d:%2.2d:%2.2d.%3.3d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
 return _return;
}


// Проверка наличия сети, попытка подключиться к серверу MajorDomo по 80 порту
void check_net() {
  int debug = digitalRead(debug_pin);
  if (debug) {
    debug_state = 1;
    return;
  }

  if (millis() - last_millis_check > 1000) {
    last_millis_check = millis();

    net.setTimeout(500);
    net.stop();
    if (net.connect(Server_IP, MQTT_PORT)) {
      check_cnt = 0;
      debug_state = 0;
      return;
     } else {
      debug_log("Net problems");
      check_cnt++;
      if (check_cnt >= check_cnt_max) {
        debug_state = 1;
        return;
      }
     }
  }
  
  debug_state = 0;
  return;
}

// Функция подключения к MQTT серверу
void connect_mqtt() {
  debug_log("connect_mqtt function");
  // Проверяем сеть
  check_net();

  // Проверяем режим Debug
  if (debug_state) {
    debug_log("Debug_state=1");
    return;
  }
  
  debug_log("connecting ");

  // Пытаемся подключиться
  if (!client.connected()) {
    mqtt_net.stop();
    if (!client.connect(MQTT_CLIENT_ID)) {
      debug_log("error");
      return;
    }
  }
  
  debug_log("connected!");
  
  // Подписываемся на топики
  client.subscribe("home/Light/#");
}


// Отсылка данных по HTTP
void httpRequest(String url) {
  debug_log("httpRequest "+ url);
  // Проверяем сеть
  check_net();

  // Проверяем режим Debug
  if (debug_state) {
    debug_log("net not available");
    return;
  }

  // Подключаемся и отсылаем GET запрос на HTTP
  net.stop();
  if (net.connect(Server_IP, Server_PORT)) {
    debug_log("connecting...");
    // send the HTTP GET request:
    net.println("GET " + url + " HTTP/1.1");
    net.println("Host: " + (String)Server_IP);
    net.println("User-Agent: Light_Shild");
    net.println("Connection: close");
    net.println();
  } else {
    debug_log("connection failed");
  }
  
}


//Отправляем статусы ламп на сервер MQTT
void sendStatusLights() {
  debug_log("sendStatusLights");
  // Проверяем сеть
  check_net();
  
  for(int i=0; i<14; i++) {
    digitalWrite(light[i], light_state[i]);
  }

  // Проверяем режим Debug
  if (debug_state) {
    debug_log("net not available");
    return;
  }
  
  for(int i=0; i<14; i++) {
    httpRequest(Light_switch_url[i] + (light_state[i] ? "turnOn":"turnOff"));
  }
  
}

//  Функция вызывается, когда есть изменения по топику на который мы подписаны!
void callback(char* topic, byte* payload, unsigned int length) {
  String topic_String = (String) topic;
  String payload_String = (String)((char)payload[0]);
  
  debug_log("incoming: "+topic_String+" - "+payload_String);
  
  topic_String.remove(0, 11);
  int num = topic_String.toInt();
  
  if (num >=1 && num <=14) {
    --num;
    if (payload_String.equals("1")) {
      light_state[num] = 1;
    } else if(payload_String.equals("0")) {
      light_state[num] = 0;
    }
    // Изменяем состояние лампочки (Пина для реле)
    digitalWrite(light[num], light_state[num]);
  }
}

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  
  server.begin();
  server_http.begin();
  
  W5100.setRetransmissionTime(0x07D0);
  W5100.setRetransmissionCount(3);

  //client.setServer(mqtt_server, MQTT_PORT);
  client.setServer(Server_IP, MQTT_PORT);
  
  client.setCallback(callback);

  debug_log("My IP address: ");
  debug_log(DisplayAddress(Ethernet.localIP()));
  
  pinMode(debug_pin, INPUT_PULLUP);

  pinMode(c_water_pin, INPUT_PULLUP);
  pinMode(h_water_pin, INPUT_PULLUP);
  pinMode(w_water_pin, INPUT_PULLUP);
  
  pinMode(C_WATER_PIN2, OUTPUT);
  pinMode(H_WATER_PIN2, OUTPUT);
  pinMode(W_WATER_PIN2, OUTPUT);
  pinMode(W_WATER_PIN3, INPUT_PULLUP);
  
  digitalWrite(C_WATER_PIN2,LOW);
  digitalWrite(H_WATER_PIN2,LOW);
  digitalWrite(W_WATER_PIN2,LOW);
  
  for(int i=0; i<14; i++) {
    pinMode(light[i], OUTPUT);
    pinMode(btn[i], INPUT);
  }
  debug_log("Sleep 2 sec");
  delay(2000);
  
  connect_mqtt();
}

void loop() {
  
  if (millis() - last_millis_water > 500) {
    // Проверяем сеть
    check_net();
    
    last_millis_water = millis();
  
    int state = 0;

    state = digitalRead(c_water_pin);
    if (state != c_water_state) {
      c_water_state = state;
      if (c_water_state == LOW) {
        ++c_water;
       
      }
    }
    state = digitalRead(h_water_pin);
    if (state != h_water_state) {
      h_water_state = state;
      if (h_water_state == LOW) {
        ++h_water;
      
      }
    }
    state = digitalRead(w_water_pin);
    if (state != w_water_state) {
      w_water_state = state;
      if (w_water_state == LOW) {
        ++w_water;
        debug_log("WARNING: ЗАТОПЛЕНИЕ! КРАНЫ ПЕРЕКРЫТЫ!");
      }
    }
  }
  
  if (millis() - last_millis_btn > 500) {
    // Проверяем сеть
    check_net();
    
    last_millis_btn = millis();

    int btn_state[13];

    // Считываем состояние выключаетелй
    for(int i=0; i<14; ++i) {
      btn_state[i] = digitalRead(btn[i]);
      
      if (btn_state[i] != btn_state_prev[i]) {
        light_state[i] = !light_state[i];
        btn_state_prev[i] = btn_state[i];
        digitalWrite(light[i], light_state[i]);
        if (!debug_state) httpRequest(Light_switch_url[i] + (light_state[i] ? "turnOn":"turnOff"));
      }
      
    }
    
  }
  
  //Отправляем статусы лам на MQTT Server
  if (!debug_state && last_millis_send_status < 1) {
    last_millis_send_status = millis();
    sendStatusLights();
  }
  
  
  // Проверяем коннект к серверу MQTT раз в 5 секунд
  if (millis() - last_millis_reconnect > 5000) {
    last_millis_reconnect = millis();
    
    if (!debug_state && !client.connected()) {
      connect_mqtt();
    }
  }

  // Поддерживаем коннект к MQTT серверу
  if (!debug_state) client.loop();

  EthernetClient client_http = server_http.available();
  if (client_http) {
    // выводим сообщение о новом клиенте
    debug_log("new client_http");
    // HTTP-запрос заканчивается пустой линией
    boolean currentLineIsBlank = true;
    while (client_http.connected()) {
      if (client_http.available()) {
        char c = client_http.read();
        readString += c;
        Serial.write(c);
        // если добрались до конца строки (т.е. получили символ новой строки),
        // и эта строка – пустая, это значит, что это конец HTTP-запроса.
        // то есть, можно приступать к отправке ответа:
        if (c == '\n' && currentLineIsBlank) {
          Serial.println(readString);
          
          if (readString.lastIndexOf("GET /setWater")>-1) {
            client_http.println("HTTP/1.1 200 OK");
            client_http.println("Content-Type: text/html");
            client_http.println("Connection: close");
            client_http.println("<!DOCTYPE HTML>");
            client_http.println("<html><head></head><body>OK</body></html>");
            break;
          }
          
          // отсылаем стандартный заголовок для HTTP-ответа:
          client_http.println("HTTP/1.1 200 OK");
          client_http.println("Content-Type: text/html");
          // после выполнения ответа соединение будет разорвано
          client_http.println("Connection: close");
          // автоматически обновляем страницу каждую 1 секунду
          client_http.println("Refresh: 1");
          client_http.println();
          client_http.println("<!DOCTYPE HTML>");
          client_http.println("<html>");
          client_http.println("<head>");
          client_http.println("<meta http-equiv=\"refresh\" content=\"1\">");
          client_http.println("<meta charset=\"UTF-8\">");
          client_http.println("<title></title>");
          client_http.println("<style>.b {font-weight: bold;} .r {font-weight: bold; color:red;} .g {font-weight: bold; color:green;} table td {padding:4px 12px; text-align:right;}</style>");
          client_http.println("</head>");
          
          client_http.println("<body>");
          client_http.print(uptime());
          client_http.println("<br />");
          client_http.println("Debug State: " + String(debug_state ? "<span class=\"r\">True</span>" : "<span class=\"g\">False</span>")+"<br />");
          client_http.println("<br />");
          client_http.println("<table>");
          for (int i = 0; i < 14; ++i) {
            client_http.print("<tr><td>Button <span class=\"b\">" + String(i+1) + "</span> pin["+String(btn[i])+"] State: " + String(btn_state_prev[i] ? "<span class=\"g\">On</span>" : "<span class=\"r\">Off</span>") + "</td> ");
            client_http.println("<td>Light <span class=\"b\">" + String(i+1) + "</span> pin["+String(light[i])+"] State: " + String(light_state[i] ? "<span class=\"g\">On</span>" : "<span class=\"r\">Off</span>") + "</td></tr>");
          }
          client_http.println("</table>");

          client_http.println("<br />");
          client_http.println("<table>");
          client_http.println("<tr><td>ХВС</td><td>"+String(c_water)+"</td></tr>");
          client_http.println("<tr><td>ГВС</td><td>"+String(h_water)+"</td></tr>");
          client_http.println("<tr><td>WAR</td><td>"+String(w_water)+"</td></tr>");
          client_http.println("</table>");
          
          client_http.println("</body></html>");
          
          break;
        }
        if (c == '\n') {
          // начинаем новую строку
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // в текущей строке есть символ:
          currentLineIsBlank = false;
        }
      }
    }
  }
  
  // проверяем наличие нового клиента по Telnet
  EthernetClient client1 = server.available();
  if (client1) {
    boolean newClient = true;
    for (byte i = 0; i < 10; i++) {
      //check whether this client refers to the same socket as one of the existing instances:
      if (clients[i] == client1) {
        newClient = false;
        break;
      }
    }
    if (newClient) {
      //check which of the existing clients can be overridden:
      for (byte i = 0; i < 10; i++) {
        if (!clients[i] && clients[i] != client1) {
          clients[i] = client1;
          // clear out the input buffer:
          client1.flush();
          debug_log("We have a new client");
          client1.print("Hello, client number: ");
          client1.print(i);
          client1.println();
          break;
        }
      }
    }
  }
  // стираем отключенных клиентов
  for (byte i = 0; i < 10; i++) {
    if (!(clients[i].connected())) {
      clients[i].stop();
    }
  }



  


}
