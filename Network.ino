void Network_setup() {
	Ethernet.begin(mac, ip);
	W5100.setRetransmissionTime(0x07D0);
	W5100.setRetransmissionCount(3);

	debug_log("My IP address: ");
	debug_log(String(DisplayAddress(Ethernet.localIP())));
}

// Преобразование IP Адреса
String DisplayAddress(IPAddress address) {
	return String(address[0]) + "." + 
		String(address[1]) + "." + 
		String(address[2]) + "." + 
		String(address[3]);
}

// Проверка наличия сети, попытка подключиться к серверу MajorDomo по 80 порту
void Network_check() {
	int debug = digitalRead(debug_pin);
	if (debug) {
		debug_state = 1;
		return;
	}
	
	unsigned long currentMillis = millis();
	if (currentMillis < last_millis_check) last_millis_check = 0;
	if (currentMillis - last_millis_check > 1000) {
		last_millis_check = currentMillis;
		
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


// Отсылка данных по HTTP
void Network_httpRequest(String url) {
	//debug_log("httpRequest "+ url);
	// Проверяем сеть
	Network_check();
	
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
