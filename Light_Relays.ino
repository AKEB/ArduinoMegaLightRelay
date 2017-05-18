#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>
#include <PubSubClient.h>

// Включить Вывод в консоль
#define DEBUG_LEVEL 1
// Порт для подключения по Telnet к Arduino, для просмотра логов для просмотра логов
#define DEBUG_SERVER_PORT 8000
// ПИН на который нужно подать питание при необходимости отключиться от сети!
#define DEBUG_PIN 14

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
//const int Server_PORT = 80;

// Скрипты для переключения светильников, список взят из MajorDomo
const String Light_01_switch_url = "/objects/?object=Light_01&op=m&m=switch&";
const String Light_02_switch_url = "/objects/?object=Light_02&op=m&m=switch&";
const String Light_03_switch_url = "/objects/?object=Light_03&op=m&m=switch&";
const String Light_04_switch_url = "/objects/?object=Light_04&op=m&m=switch&";
const String Light_05_switch_url = "/objects/?object=Light_05&op=m&m=switch&";
const String Light_06_switch_url = "/objects/?object=Light_06&op=m&m=switch&";
const String Light_07_switch_url = "/objects/?object=Light_07&op=m&m=switch&";
const String Light_08_switch_url = "/objects/?object=Light_08&op=m&m=switch&";
const String Light_09_switch_url = "/objects/?object=Light_09&op=m&m=switch&";
const String Light_10_switch_url = "/objects/?object=Light_10&op=m&m=switch&";
const String Light_11_switch_url = "/objects/?object=Light_11&op=m&m=switch&";
const String Light_12_switch_url = "/objects/?object=Light_12&op=m&m=switch&";
const String Light_13_switch_url = "/objects/?object=Light_13&op=m&m=switch&";
const String Light_14_switch_url = "/objects/?object=Light_14&op=m&m=switch&";

// Пины для Вывода на РЕЛЕ
const int light_01 =  23;
const int light_02 =  25;
const int light_03 =  27;
const int light_04 =  29;
const int light_05 =  31;
const int light_06 =  33;
const int light_07 =  35;
const int light_08 =  37;
const int light_09 =  39;
const int light_10 =  41;
const int light_11 =  43;
const int light_12 =  45;
const int light_13 =  47;
const int light_14 =  49;

// Пины для Выклюателей, должны быть подтянуты к земле через резистор 10K
const int btn_01 =  22;
const int btn_02 =  24;
const int btn_03 =  26;
const int btn_04 =  28;
const int btn_05 =  30;
const int btn_06 =  32;
const int btn_07 =  34;
const int btn_08 =  36;
const int btn_09 =  38;
const int btn_10 =  40;
const int btn_11 =  42;
const int btn_12 =  44;
const int btn_13 =  46;
const int btn_14 =  48;

// Состояние лампочек
int light_01_state = 0;
int light_02_state = 0;
int light_03_state = 0;
int light_04_state = 0;
int light_05_state = 0;
int light_06_state = 0;
int light_07_state = 0;
int light_08_state = 0;
int light_09_state = 0;
int light_10_state = 0;
int light_11_state = 0;
int light_12_state = 0;
int light_13_state = 0;
int light_14_state = 0;

// Текущее состояние выключателей
int btn_01_state_prev = 0;
int btn_02_state_prev = 0;
int btn_03_state_prev = 0;
int btn_04_state_prev = 0;
int btn_05_state_prev = 0;
int btn_06_state_prev = 0;
int btn_07_state_prev = 0;
int btn_08_state_prev = 0;
int btn_09_state_prev = 0;
int btn_10_state_prev = 0;
int btn_11_state_prev = 0;
int btn_12_state_prev = 0;
int btn_13_state_prev = 0;
int btn_14_state_prev = 0;

// переменные для эмитации команды delay
unsigned long last_millis_btn = 0;
unsigned long last_millis_reconnect = 0;
unsigned long last_millis_check = 0;
unsigned long last_millis_send_status = 0;

// Текущее кол-во попыток связаться с сервером
int check_cnt = 0;
// Максимальное кол-во попыток связаться с сервером
const int check_cnt_max = SERVER_CONNECT_MAX_COUNT; 


const int debug_pin =  DEBUG_PIN;
int debug_state = 0;

EthernetClient net;
EthernetClient mqtt_net;
EthernetServer server(DEBUG_SERVER_PORT);
// Кол-во клиентов способных подключиться по telnet
EthernetClient clients[10];

PubSubClient client(mqtt_net);


// Преобразование IP Адреса
String DisplayAddress(IPAddress address) {
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}

// Функция вывода в лог
void debug_log(String str) {
  if (DEBUG_LEVEL) Serial.println(str);
  for (byte i = 0; i < 10; i++) {
    if (clients[i]) {
      clients[i].println(str);
    }
  }
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

//Отправляем статусы ламп на сервер MQTT
void sendStatusLights() {
  debug_log("sendStatusLights");
  // Проверяем сеть
  check_net();

  digitalWrite(light_01, light_01_state);
  digitalWrite(light_02, light_02_state);
  digitalWrite(light_03, light_03_state);
  digitalWrite(light_04, light_04_state);
  digitalWrite(light_05, light_05_state);
  digitalWrite(light_06, light_06_state);
  digitalWrite(light_07, light_07_state);
  digitalWrite(light_08, light_08_state);
  digitalWrite(light_09, light_09_state);
  digitalWrite(light_10, light_10_state);
  digitalWrite(light_11, light_11_state);
  digitalWrite(light_12, light_12_state);
  digitalWrite(light_13, light_13_state);
  digitalWrite(light_14, light_14_state);

// Проверяем режим Debug
  if (debug_state) {
    debug_log("net not available");
    return;
  }
  
  client.publish("home/Light/01", light_01_state ? "1":"0");
  client.publish("home/Light/02", light_02_state ? "1":"0");
  client.publish("home/Light/03", light_03_state ? "1":"0");
  client.publish("home/Light/04", light_04_state ? "1":"0");
  client.publish("home/Light/05", light_05_state ? "1":"0");
  client.publish("home/Light/06", light_06_state ? "1":"0");
  client.publish("home/Light/07", light_07_state ? "1":"0");
  client.publish("home/Light/08", light_08_state ? "1":"0");
  client.publish("home/Light/09", light_09_state ? "1":"0");
  client.publish("home/Light/10", light_10_state ? "1":"0");
  client.publish("home/Light/11", light_11_state ? "1":"0");
  client.publish("home/Light/12", light_12_state ? "1":"0");
  client.publish("home/Light/13", light_13_state ? "1":"0");
  client.publish("home/Light/14", light_14_state ? "1":"0");
  
  
}

//  Функция вызывается, когда есть изменения по топику на который мы подписаны!
void callback(char* topic, byte* payload, unsigned int length) {
  String topic_String = (String) topic;
  String payload_String = (String)((char)payload[0]);
  
  debug_log("incoming: "+topic_String+" - "+payload_String);
  
  if (topic_String.equals("home/Light/01")) {
    if (payload_String.equals("1")) {
      light_01_state = 1;
    } else if(payload_String.equals("0")) {
      light_01_state = 0;
    }
    // Изменяем состояние лампочки (Пина для реле)
    digitalWrite(light_01, light_01_state);
    
  } else if (topic_String.equals("home/Light/02")) {
    if (payload_String.equals("1")) {
      light_02_state = 1;
    } else if(payload_String.equals("0")) {
      light_02_state = 0;
    }
    digitalWrite(light_02, light_02_state);
  } else if (topic_String.equals("home/Light/03")) {
    if (payload_String.equals("1")) {
      light_03_state = 1;
    } else if(payload_String.equals("0")) {
      light_03_state = 0;
    }
    digitalWrite(light_03, light_03_state);
  } else if (topic_String.equals("home/Light/04")) {
    if (payload_String.equals("1")) {
      light_04_state = 1;
    } else if(payload_String.equals("0")) {
      light_04_state = 0;
    }
    digitalWrite(light_04, light_04_state);
  } else if (topic_String.equals("home/Light/05")) {
    if (payload_String.equals("1")) {
      light_05_state = 1;
    } else if(payload_String.equals("0")) {
      light_05_state = 0;
    }
    digitalWrite(light_05, light_05_state);
  } else if (topic_String.equals("home/Light/06")) {
    if (payload_String.equals("1")) {
      light_06_state = 1;
    } else if(payload_String.equals("0")) {
      light_06_state = 0;
    }
    digitalWrite(light_06, light_06_state);
  } else if (topic_String.equals("home/Light/07")) {
    if (payload_String.equals("1")) {
      light_07_state = 1;
    } else if(payload_String.equals("0")) {
      light_07_state = 0;
    }
    digitalWrite(light_07, light_07_state);
  } else if (topic_String.equals("home/Light/08")) {
    if (payload_String.equals("1")) {
      light_08_state = 1;
    } else if(payload_String.equals("0")) {
      light_08_state = 0;
    }
    digitalWrite(light_08, light_08_state);
  } else if (topic_String.equals("home/Light/09")) {
    if (payload_String.equals("1")) {
      light_09_state = 1;
    } else if(payload_String.equals("0")) {
      light_09_state = 0;
    }
    digitalWrite(light_09, light_09_state);
  } else if (topic_String.equals("home/Light/10")) {
    if (payload_String.equals("1")) {
      light_10_state = 1;
    } else if(payload_String.equals("0")) {
      light_10_state = 0;
    }
    digitalWrite(light_10, light_10_state);
  } else if (topic_String.equals("home/Light/11")) {
    if (payload_String.equals("1")) {
      light_11_state = 1;
    } else if(payload_String.equals("0")) {
      light_11_state = 0;
    }
    digitalWrite(light_11, light_11_state);
  } else if (topic_String.equals("home/Light/12")) {
    if (payload_String.equals("1")) {
      light_12_state = 1;
    } else if(payload_String.equals("0")) {
      light_12_state = 0;
    }
    digitalWrite(light_12, light_12_state);
  } else if (topic_String.equals("home/Light/13")) {
    if (payload_String.equals("1")) {
      light_13_state = 1;
    } else if(payload_String.equals("0")) {
      light_13_state = 0;
    }
    digitalWrite(light_13, light_13_state);
  } else if (topic_String.equals("home/Light/14")) {
    if (payload_String.equals("1")) {
      light_14_state = 1;
    } else if(payload_String.equals("0")) {
      light_14_state = 0;
    }
    digitalWrite(light_14, light_14_state);
  }
}

void setup() {
  
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  W5100.setRetransmissionTime(0x07D0);
  W5100.setRetransmissionCount(3);

  //client.setServer(mqtt_server, MQTT_PORT);
  client.setServer(Server_IP, MQTT_PORT);
  
  client.setCallback(callback);

  debug_log("My IP address: ");
  debug_log(DisplayAddress(Ethernet.localIP()));
  
  pinMode(debug_pin, INPUT);
  
  pinMode(light_01, OUTPUT);
  pinMode(light_02, OUTPUT);
  pinMode(light_03, OUTPUT);
  pinMode(light_04, OUTPUT);
  pinMode(light_05, OUTPUT);
  pinMode(light_06, OUTPUT);
  pinMode(light_07, OUTPUT);
  pinMode(light_08, OUTPUT);
  pinMode(light_09, OUTPUT);
  pinMode(light_10, OUTPUT);
  pinMode(light_11, OUTPUT);
  pinMode(light_12, OUTPUT);
  pinMode(light_13, OUTPUT);
  pinMode(light_14, OUTPUT);

  pinMode(btn_01, INPUT);
  pinMode(btn_02, INPUT);
  pinMode(btn_03, INPUT);
  pinMode(btn_04, INPUT);
  pinMode(btn_05, INPUT);
  pinMode(btn_06, INPUT);
  pinMode(btn_07, INPUT);
  pinMode(btn_08, INPUT);
  pinMode(btn_09, INPUT);
  pinMode(btn_10, INPUT);
  pinMode(btn_11, INPUT);
  pinMode(btn_12, INPUT);
  pinMode(btn_13, INPUT);
  pinMode(btn_14, INPUT);

  connect_mqtt();  
}



void loop() {
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



  
  if (millis() - last_millis_btn > 500) {
    // Проверяем сеть
    check_net();
    
    last_millis_btn = millis();

    // Считываем состояние выключаетелй
    int btn_01_state = digitalRead(btn_01);
    int btn_02_state = digitalRead(btn_02);
    int btn_03_state = digitalRead(btn_03);
    int btn_04_state = digitalRead(btn_04);
    int btn_05_state = digitalRead(btn_05);
    int btn_06_state = digitalRead(btn_06);
    int btn_07_state = digitalRead(btn_07);
    int btn_08_state = digitalRead(btn_08);
    int btn_09_state = digitalRead(btn_09);
    int btn_10_state = digitalRead(btn_10);
    int btn_11_state = digitalRead(btn_11);
    int btn_12_state = digitalRead(btn_12);
    int btn_13_state = digitalRead(btn_13);
    int btn_14_state = digitalRead(btn_14);

      if (btn_01_state != btn_01_state_prev) {
        light_01_state = !light_01_state;
        btn_01_state_prev = btn_01_state;
        digitalWrite(light_01, light_01_state);
        if (!debug_state) client.publish("home/Light/01", light_01_state ? "1":"0");
      }
      if (btn_02_state != btn_02_state_prev) {
        light_02_state = !light_02_state;
        btn_02_state_prev = btn_02_state;
        digitalWrite(light_02, light_02_state);
        if (!debug_state) client.publish("home/Light/02", light_02_state ? "1":"0");
      }
      if (btn_03_state != btn_03_state_prev) {
        light_03_state = !light_03_state;
        btn_03_state_prev = btn_03_state;
        digitalWrite(light_03, light_03_state);
        if (!debug_state) client.publish("home/Light/03", light_03_state ? "1":"0");
      }
      if (btn_04_state != btn_04_state_prev) {
        light_04_state = !light_04_state;
        btn_04_state_prev = btn_04_state;
        digitalWrite(light_04, light_04_state);
        if (!debug_state) client.publish("home/Light/04", light_04_state ? "1":"0");
      }
      if (btn_05_state != btn_05_state_prev) {
        light_05_state = !light_05_state;
        btn_05_state_prev = btn_05_state;
        digitalWrite(light_05, light_05_state);
        if (!debug_state) client.publish("home/Light/05", light_05_state ? "1":"0");
      }
      if (btn_06_state != btn_06_state_prev) {
        light_06_state = !light_06_state;
        btn_06_state_prev = btn_06_state;
        digitalWrite(light_06, light_06_state);
        if (!debug_state) client.publish("home/Light/06", light_06_state ? "1":"0");
      }
      if (btn_07_state != btn_07_state_prev) {
        light_07_state = !light_07_state;
        btn_07_state_prev = btn_07_state;
        digitalWrite(light_07, light_07_state);
        if (!debug_state) client.publish("home/Light/07", light_07_state ? "1":"0");
      }
      if (btn_08_state != btn_08_state_prev) {
        light_08_state = !light_08_state;
        btn_08_state_prev = btn_08_state;
        digitalWrite(light_08, light_08_state);
        if (!debug_state) client.publish("home/Light/08", light_08_state ? "1":"0");
      }
      if (btn_09_state != btn_09_state_prev) {
        light_09_state = !light_09_state;
        btn_09_state_prev = btn_09_state;
        digitalWrite(light_09, light_09_state);
        if (!debug_state) client.publish("home/Light/09", light_09_state ? "1":"0");
      }
      if (btn_10_state != btn_10_state_prev) {
        light_10_state = !light_10_state;
        btn_10_state_prev = btn_10_state;
        digitalWrite(light_10, light_10_state);
        if (!debug_state) client.publish("home/Light/10", light_10_state ? "1":"0");
      }
      if (btn_11_state != btn_11_state_prev) {
        light_11_state = !light_11_state;
        btn_11_state_prev = btn_11_state;
        digitalWrite(light_11, light_11_state);
        if (!debug_state) client.publish("home/Light/11", light_11_state ? "1":"0");
      }
      if (btn_12_state != btn_12_state_prev) {
        light_12_state = !light_12_state;
        btn_12_state_prev = btn_12_state;
        digitalWrite(light_12, light_12_state);
        if (!debug_state) client.publish("home/Light/12", light_12_state ? "1":"0");
      }
      if (btn_13_state != btn_13_state_prev) {
        light_13_state = !light_13_state;
        btn_13_state_prev = btn_13_state;
        digitalWrite(light_13, light_13_state);
        if (!debug_state) client.publish("home/Light/13", light_13_state ? "1":"0");
      }
      if (btn_14_state != btn_14_state_prev) {
        light_14_state = !light_14_state;
        btn_14_state_prev = btn_14_state;
        digitalWrite(light_14, light_14_state);
        if (!debug_state) client.publish("home/Light/14", light_14_state ? "1":"0");
      }

  }
  
  //Отправляем статусы лам на MQTT Server
  if (millis() - last_millis_send_status > 15000) {
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

}
