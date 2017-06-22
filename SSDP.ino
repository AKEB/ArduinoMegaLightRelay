void SSDP_setup() {
	Udp.begin(localPort);
}


void SSDP_loop() {
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if(packetSize) {
		Serial.print("Received packet of size ");
		Serial.println(packetSize);
		Serial.print("From ");
		IPAddress remote = Udp.remoteIP();
		for (int i =0; i < 4; i++) {
			Serial.print(remote[i], DEC);
			if (i < 3) {
				Serial.print(".");
			}
		}
		Serial.print(", port ");
		Serial.println(Udp.remotePort());
		
		// read the packet into packetBufffer
		Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
		Serial.println("Contents:");
		Serial.println(packetBuffer);
		
    	// send a reply, to the IP address and port that sent us the packet we received
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write("HTTP/1.1 200 OK\r\nSERVER:Cream/3.1,UPnP/1.0,UPnP/1.0\r\nCACHE-CONTROL:max-age=360\r\nLOCATION:http://");
		Udp.write(Ethernet.localIP());    // this really should be Ethernet.LocalIP()
		//Udp.write("/devdesc.xml\r\nST:upnp:rootdevice\r\nUSN:uuid:b629a14a-d342-3c5c-bc8c-f7417arduino::upnp:rootdevice\r\nEXT:\r\n\r\n");
		Udp.write("/devdesc.xml\r\nNT:uuid:b629a14a-d342-3c5c-bc8c-f7417arduino\r\nNTS:ssdp:alive\r\nUSN:uuid:b629a14a-d342-3c5c-bc8c-f7417arduino\r\n\r\n");
		Udp.endPacket();
	}
}



