void Buttons_setup() {
	for(int i=0; i<14; i++) {
		pinMode(btn[i], INPUT);
	}
}

void Buttons_loop() {
	if (currentMillis - last_millis_btn > 500UL) {
		// Проверяем сеть
		Network_check();
		
		last_millis_btn = currentMillis;
		
		int btn_state[13];
		
		// Считываем состояние выключаетелй
		for(int i=0; i<14; ++i) {
			btn_state[i] = digitalRead(btn[i]);
			
			if (btn_state[i] != btn_state_prev[i]) {
				light_state[i] = !light_state[i];
				btn_state_prev[i] = btn_state[i];
				digitalWrite(light[i], light_state[i]);
				if (!debug_state) Network_httpRequest(Light_switch_url[i] + (light_state[i] ? "turnOn":"turnOff"));
			}
			
		}
		
	}
}

