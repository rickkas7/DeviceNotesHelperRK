#include "DeviceNotesHelperRK.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

DeviceNotesHelper deviceNotesHelper;

const unsigned long UPDATE_COUNTER_PERIOD = 60000; // Once a minute
unsigned long lastUpdateCounter = 0;

void incrementCounter();

void setup() {
	// Override settings here
	deviceNotesHelper.withBufferSize(640); // Allow a JSON object up to 640 bytes
	deviceNotesHelper.withGetAtBoot();

	deviceNotesHelper.withDataUpdatedCallback([](JsonParser &jp) {
		// This function gets called when the data is received from the cloud
		lastUpdateCounter = millis();
		incrementCounter();
	});

	// You must call this from setup!
	deviceNotesHelper.setup();
}

void loop() {
	if (millis() - lastUpdateCounter >= UPDATE_COUNTER_PERIOD && Particle.connected()) {
		lastUpdateCounter = millis();

		if (!deviceNotesHelper.hasData()) {
			// We didn't update from the cloud at boot successfully, so try to get data again
			deviceNotesHelper.getFromCloud();
		}
		else {
			// We have saved data, so just increment the value we got last time
			// rather than doing both a get and set
			incrementCounter();
		}
	}


	// You must call this from loop, preferably every loop but at least every few seconds.
	deviceNotesHelper.loop();
}

void incrementCounter() {
	JsonParser &jp = deviceNotesHelper.getJsonParser();

	int counter = 0;
	jp.getValueByKey(jp.getOuterObject(), "counter", counter);

	JsonModifier mod(jp);
	mod.insertOrUpdateKeyValue(jp.getOuterObject(), "counter", ++counter);

	Log.info("counter=%d", counter);

	deviceNotesHelper.putToCloud();
}

