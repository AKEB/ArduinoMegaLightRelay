void Power_setup() {
	pinMode(POWER_LED_PIN,OUTPUT);
	pinMode(POWER_PIN,INPUT_PULLUP);
}

void Power_loop() {
	int sensorVal = digitalRead(POWER_PIN);
	
	if (sensorVal == HIGH) {
		digitalWrite(POWER_LED_PIN, LOW);
		
	} else {
		if (lastPowerState != sensorVal) {
			digitalWrite(POWER_LED_PIN, HIGH);
			Network_httpRequest(Power_url);
		}
	}
	
	lastPowerState = sensorVal;
	
}

