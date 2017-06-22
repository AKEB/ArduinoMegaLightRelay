void Lights_setup() {
	for(int i=0; i<14; i++) {
		pinMode(light[i], OUTPUT);
	}
}

void Lights_loop() {
	//Отправляем статусы ламп на MQTT Server
	if (!debug_state && last_millis_send_status < 1) {
		last_millis_send_status = currentMillis;
		//sendStatusLights();
		Lights_RefreshStatus();
	} else if (debug_state) {
		last_millis_send_status = 0;
	}
}



//Отправляем статусы ламп на сервер MQTT
void sendStatusLights() {
	debug_log("sendStatusLights");
	// Проверяем сеть
	Network_check();
	
	for(int i=0; i<14; i++) {
		digitalWrite(light[i], light_state[i]);
	}
	
	// Проверяем режим Debug
	if (debug_state) {
		debug_log("net not available");
		return;
	}
	
	for(int i=0; i<14; i++) {
		Network_httpRequest(Light_switch_url[i] + "switch&" + (light_state[i] ? "turnOn":"turnOff"));
	}
	
}

void Lights_RefreshStatus() {
	for(int i=0; i<14; i++) {
		Network_httpRequest(Light_switch_url[i] + "Refresh");
	}
}

