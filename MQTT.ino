void MQTT_setup() {
	//MQTT_client.setServer(mqtt_server, MQTT_PORT);
	MQTT_client.setServer(Server_IP, MQTT_PORT);
	
	MQTT_client.setCallback(callback);
}

void MQTT_loop() {
	// Проверяем коннект к серверу MQTT раз в 5 секунд
	if (currentMillis - last_millis_reconnect > 5*1000UL) {
		last_millis_reconnect = currentMillis;
		
		if (!debug_state && !MQTT_client.connected()) {
			MQTT_connect();
		}
	}
	
	// Поддерживаем коннект к MQTT серверу
	if (!debug_state) MQTT_client.loop();
}


// Функция подключения к MQTT серверу
void MQTT_connect() {
	debug_log("connect_mqtt function");
	// Проверяем сеть
	Network_check();
	
	// Проверяем режим Debug
	if (debug_state) {
		debug_log("Debug_state=1");
		return;
	}
	
	debug_log("MQTT connecting...");
	
	// Пытаемся подключиться
	if (!MQTT_client.connected()) {
		mqtt_net.stop();
		if (!MQTT_client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
			debug_log("MQTT can not connect");
			return;
		}
	}
	
	debug_log("connected!");
	
	// Подписываемся на топики
	MQTT_client.subscribe("home/Light/#");
}


//  Функция вызывается, когда есть изменения по топику на который мы подписаны!
void callback(char* topic, byte* payload, unsigned int length) {
	String topic_String = String(topic);
	String payload_String = String((char *)payload);

	payload_String = payload_String.substring(0,length);
	
	debug_log("MQTT incoming: "+topic_String+" - "+payload_String);
	
	topic_String.remove(0, 11);
	int num = topic_String.toInt();
	
	if (num >=1 && num <=14) {
		--num;
		
		light_state_switch[num] = 0;
		
		if (payload_String.equals("1")) {
			light_state[num] = 1;
		} else if(payload_String.equals("0")) {
			light_state[num] = 0;
		}
		// Изменяем состояние лампочки (Пина для реле)
		digitalWrite(light[num], light_state[num]);
	}
}
