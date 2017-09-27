// Функция вывода в лог
void debug_log(String str) {
	if (DEBUG_LEVEL) {
		Serial.println(str);
	}
	
	Telnet_log(str);
	
	Logs[Log_index] = "[" + String(millis()/1000) + "] " + str;
	Log_index++;
	if (Log_index >=100) Log_index = 0;
}

void Telnet_setup() {
	TelnetServer.begin();
}

void Telnet_loop() {
	// проверяем наличие нового клиента по Telnet
	EthernetClient client1 = TelnetServer.available();
	if (client1) {
		boolean newClient = true;
		for (byte i = 0; i < 10; i++) {
			//check whether this client refers to the same socket as one of the existing instances:
			if (Telnet_clients[i] == client1) {
				newClient = false;
				break;
			}
		}
		if (newClient) {
			//check which of the existing Telnet_clients can be overridden:
			for (byte i = 0; i < 10; i++) {
				if (!Telnet_clients[i] && Telnet_clients[i] != client1) {
					Telnet_clients[i] = client1;
					// clear out the input buffer:
					client1.flush();
					debug_log("We have a new client");
					client1.print("Hello, client number: ");
					client1.print(i);
					client1.println();
					for (int i = Log_index; i < 100; ++i) {
						client1.println(String(i)+": " + Logs[i]);
					}
					for (int i = 0; i < Log_index; ++i) {
						client1.println(String(i)+": " + Logs[i]);
					}
					break;
				}
			}
		}
	}
	// стираем отключенных клиентов
	for (byte i = 0; i < 10; i++) {
		if (!(Telnet_clients[i].connected())) {
			Telnet_clients[i].stop();
		}
	}
}

void Telnet_log(String str) {
	for (byte i = 0; i < 10; i++) {
		if (Telnet_clients[i]) {
			Telnet_clients[i].println(str);
		}
	}
}

