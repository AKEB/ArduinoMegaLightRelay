void Buttons_setup() {
	for(int i=0; i<14; i++) {
		pinMode(btn[i], INPUT);
	}
}

void Buttons_loop() {
	if (currentMillis - last_millis_btn > 200UL) {
		// Проверяем сеть
		Network_check();
		
		last_millis_btn = currentMillis;
		
		int btn_state[13];
		
		// Считываем состояние выключаетелй
		for(int i=0; i<14; ++i) {
			btn_state[i] = digitalRead(btn[i]);
			
			if (btn_state[i] != btn_state_prev[i]) {
				light_state[i] = !light_state[i];
				
				debug_log("Button["+String(i)+"] State New="+String(btn_state[i])+" Prev="+String(btn_state_prev[i])+" debug_state="+String(debug_state)+" light_state="+String(light_state[i]));
				
				btn_state_prev[i] = btn_state[i];
				if (!debug_state) Network_httpRequest(Light_switch_url[i] + "switch&"));
				else digitalWrite(light[i], light_state[i]);
			}
			
		}
		
	}
}

