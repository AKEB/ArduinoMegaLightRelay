void Time_setup() {
	currentMillis = millis();
}

void Time_loop() {
	currentMillis = millis();
	
	if(currentMillis>=3000000000){ 
		HighMillis=1;
	}
	//** Making note of actual rollover **//
	if(currentMillis<=100000 && HighMillis==1){
		Rollover++;
		HighMillis=0;
	}
}


// Function made to millis() be an optional parameter
char *Time_uptime() {
	return (char *)Time_uptime(millis()); // call original uptime function with unsigned long millis() value
}

char *Time_uptime(unsigned long milli) {
	static char _return[32];
	unsigned long secs=milli/1000, mins=secs/60;
	unsigned int hours=mins/60, days=hours/24;
	milli-=secs*1000;
	secs-=mins*60;
	mins-=hours*60;
	hours-=days*24;
	days = days+ (Rollover*50);
	sprintf(_return,"Uptime %d days %02d:%02d:%02d.%03d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
	return _return;
}

