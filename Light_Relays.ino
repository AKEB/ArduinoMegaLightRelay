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

// Порт для подключения по Telnet к Arduino, для просмотра логов
// #define DEBUG_SERVER_PORT 23

// ПИН на который нужно подать GND при необходимости отключиться от сети!
#define DEBUG_PIN 14

// ПИН для Счетчика электроэнергии
// #define POWER_PIN A0
// ПИН для Счетчика электроэнергии
// #define POWER_LED_PIN 13

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
#define MQTT_USER       "akeb"
#define MQTT_PASSWORD   "Akeb123Akeb"

// IP адрес сервера MajorDomo куда надо посылать команду на переключение света, при назатии на выключатель
char Server_IP[14] = "192.168.1.200";
const int Server_PORT = 80;

// const String Power_url = "/objects/?script=ElectroCounter";

// Скрипты для переключения светильников, список взят из MajorDomo
const String Light_switch_url[] = {"/objects/?object=Light_01&op=m&m=", "/objects/?object=Light_02&op=m&m=", "/objects/?object=Light_03&op=m&m=", "/objects/?object=Light_04&op=m&m=", "/objects/?object=Light_05&op=m&m=", "/objects/?object=Light_06&op=m&m=", "/objects/?object=Light_07&op=m&m=", 
                                   "/objects/?object=Light_08&op=m&m=", "/objects/?object=Light_09&op=m&m=", "/objects/?object=Light_10&op=m&m=", "/objects/?object=Light_11&op=m&m=", "/objects/?object=Light_12&op=m&m=", "/objects/?object=Light_13&op=m&m=", "/objects/?object=Light_14&op=m&m="};

// Пины для Вывода на РЕЛЕ
const int light[] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49};

// Пины для Выключателей, должны быть подтянуты к земле через резистор 10K
const int btn[] =  {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48};

// Состояние лампочек
boolean light_state[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// Состояние лампочек
boolean light_state_switch[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// Текущее состояние выключателей
boolean btn_state_prev[] =  {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// Настройка импульсных выключателей
boolean btn_state_pulse[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// переменные для эмитации команды delay
unsigned long last_millis_btn = 0;
unsigned long last_millis_reconnect = 0;
unsigned long last_millis_check = 0;
unsigned long last_millis_send_status = 0;

unsigned long last_millis_resend_status = 0;

// Текущее кол-во попыток связаться с сервером
byte check_cnt = 0;
// Максимальное кол-во попыток связаться с сервером
const int check_cnt_max = SERVER_CONNECT_MAX_COUNT; 

const int debug_pin =  DEBUG_PIN;
int debug_state = 0;

EthernetClient net;
EthernetClient mqtt_net;

// EthernetServer TelnetServer(DEBUG_SERVER_PORT);
EthernetServer server_http(80);

// Кол-во клиентов способных подключиться по telnet
// EthernetClient Telnet_clients[10];

PubSubClient MQTT_client(mqtt_net);

String readString;

int HighMillis=0;
int Rollover=0;

// int lastPowerState = 0;
// struct port_param_t {String name;int value;};
// #define MAX_PARAMS 10
// port_param_t params[MAX_PARAMS];

unsigned long currentMillis = millis();

void setup() {
	Serial.begin(9600);

	// Power_setup();
	
	Time_setup();
	
	debug_log("Network_setup");
	Network_setup();
	
	// debug_log("Telnet_setup");
	// Telnet_setup();

	debug_log("WebServer_setup");
	WebServer_setup();
	
	debug_log("MQTT_setup");
	MQTT_setup();
	
	pinMode(debug_pin, INPUT_PULLUP);
	
	debug_log("Buttons_setup");
	Buttons_setup();
	
	debug_log("Lights_setup");
	Lights_setup();
	
	debug_log("Sleep 2 sec");
	delay(2000);
	
	MQTT_connect();
}

void loop() {
	Time_loop();

	Buttons_loop();
	
	Lights_loop();

	MQTT_loop();
	
	WebServer_loop();
	
	// Telnet_loop();

	// Power_loop();
}
