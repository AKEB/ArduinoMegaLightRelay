void Power_setup() {
	pinMode(POWER_LED_PIN,OUTPUT);
	pinMode(POWER_PIN,INPUT);
}

void Power_loop() {
	//int sensorVal = analogRead(POWER_PIN);
	//Serial.println(sensorVal);
	/*
	if (sensorVal == HIGH) {
		debug_log("Power High");
		if (lastPowerState != sensorVal) {
			digitalWrite(POWER_LED_PIN, HIGH);
			Network_httpRequest(Power_url);
		}
		
	} else {
		debug_log("Power Low");
		digitalWrite(POWER_LED_PIN, LOW);
	}
	
	lastPowerState = sensorVal;
	*/
}

