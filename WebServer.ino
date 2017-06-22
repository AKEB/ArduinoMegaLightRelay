
void WebServer_setup() {
	server_http.begin();
}

void WebServer_loop() {
	EthernetClient client_http = server_http.available();
	if (client_http) {
		// выводим сообщение о новом клиенте
		debug_log("new client_http");
		// HTTP-запрос заканчивается пустой линией
		boolean currentLineIsBlank = true;
		readString = "";
		while (client_http.connected()) {
			if (client_http.available()) {
				char c = client_http.read();
				readString += c;
				//Serial.write(c);
				// если добрались до конца строки (т.е. получили символ новой строки),
				// и эта строка – пустая, это значит, что это конец HTTP-запроса.
				// то есть, можно приступать к отправке ответа:
				if (c == '\n' && currentLineIsBlank) {
					Serial.println(readString);
					
					if (readString.lastIndexOf("favicon.ico")>-1) {
						client_http.println("HTTP/1.1 404 Not Found");
						client_http.println("Content-Type: text/html");
						client_http.println("Connection: close");
						client_http.println();
						client_http.println("<html><head><title>Arduino Web Server - Error 404</title></head><body><h1>Error 404: Sorry, that page cannot be found!</h1></body></html>");
						client_http.stop();
						break;
					} else if (readString.lastIndexOf("devdesc.xml")>-1) {
						client_http.println("HTTP/1.1 200 OK");
						Serial.println("send device description");
						client_http.println("Content-Type: text/xml");
						client_http.println();
						client_http.print("<?xml version=\"1.0\"?>\r\n");
						client_http.print("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n");
						client_http.print("  <specVersion>\r\n");
						client_http.print("    <major>1</major>\r\n");
						client_http.print("    <minor>0</minor>\r\n");
						client_http.print("  </specVersion>\r\n");
						client_http.print("  <URLBase>http://");
						client_http.print(Ethernet.localIP());
						client_http.print("</URLBase>\r\n");
						client_http.print("  <device>\r\n");
						client_http.print("    <deviceType>urn:AcmePlus-com:device:Arduino:1</deviceType>\r\n");
						client_http.print("    <friendlyName>Light Relays</friendlyName>\r\n");
						client_http.print("    <manufacturer>Vadim Babajanyan</manufacturer>\r\n");
						client_http.print("    <manufacturerURL>http://akeb.ru</manufacturerURL>\r\n");
						client_http.print("    <modelDescription></modelDescription>\r\n");
						client_http.print("    <modelName>Light Relays</modelName>\r\n");
						client_http.print("    <modelURL>https://bitbucket.org/smarthometeam_AKEB/arduinomegalightrelay</modelURL>\r\n");
						client_http.print("    <modelNumber>000000000001</modelNumber>\r\n");
						client_http.print("    <serialNumber>000000000001</serialNumber>\r\n");
						client_http.print("    <UDN>uuid:b629a14a-d342-3c5c-bc8c-f7417arduino</UDN>\r\n");
						client_http.print("    <presentationURL>http://");
						client_http.print(Ethernet.localIP());
						client_http.print("</presentationURL>\r\n");   //BaseURL + /info is device homepage.
						client_http.print("  </device>\r\n");
						client_http.print("</root>\r\n");
					} else {
						// отсылаем стандартный заголовок для HTTP-ответа:
						client_http.println("HTTP/1.1 200 OK");
						client_http.println("Content-Type: text/html");
						// после выполнения ответа соединение будет разорвано
						client_http.println("Connection: close");
						// автоматически обновляем страницу каждую 1 секунду
						client_http.println("Refresh: 10");
						client_http.println();
						client_http.println("<!DOCTYPE HTML>");
						client_http.println("<html>");
						client_http.println("<head>");
						//client_http.println("<meta http-equiv=\"refresh\" content=\"1\">");
						client_http.println("<meta charset=\"UTF-8\">");
						client_http.println("<title></title>");
						client_http.println("<style>.b {font-weight: bold;} .r {font-weight: bold; color:red;} .g {font-weight: bold; color:green;} table td {padding:4px 12px; text-align:right;}</style>");
						client_http.println("</head>");
						
						client_http.println("<body>");
						client_http.print(Time_uptime());
						client_http.println("<br />");
						client_http.println("Debug State: " + String(debug_state ? "<span class=\"r\">True</span>" : "<span class=\"g\">False</span>")+"<br />");
						client_http.println("<br />");
						client_http.println("<table cellpadding=0 cellspacing=0 border=1>");
						for (int i = 0; i < 14; ++i) {
							client_http.print("<tr><td>Button <span class=\"b\">" + String(i+1) + "</span> pin["+String(btn[i])+"] State: " + String(btn_state_prev[i] ? "<span class=\"g\">On</span>" : "<span class=\"r\">Off</span>") + "</td> ");
							client_http.println("<td>Light <span class=\"b\">" + String(i+1) + "</span> pin["+String(light[i])+"] State: " + String(light_state[i] ? "<span class=\"g\">On</span>" : "<span class=\"r\">Off</span>") + "</td></tr>");
						}
						client_http.println("</table>");

						client_http.println("</body></html>");
						client_http.stop();
						break;
					}
				}
				if (c == '\n') {
					// начинаем новую строку
					currentLineIsBlank = true;
				} else if (c != '\r') {
					// в текущей строке есть символ:
					currentLineIsBlank = false;
				}
			}
		}
	}
}

