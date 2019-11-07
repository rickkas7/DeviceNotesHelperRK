#include "DeviceNotesHelperRK.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

DeviceNotesHelper deviceNotesHelper;


void setup() {
	// Override settings here
	deviceNotesHelper
		.withBufferSize(1024) 	// Allow a JSON object up to 1024 bytes
		.withGetAtBoot();	 	// Fetch the devices notes once at boot after connecting to the cloud

	deviceNotesHelper.withDataUpdatedCallback([](JsonParser &jp) {
		// This function gets called when the data is updated at boot time. We just print the
		// data here, but you could do something more useful with it.

		int setting = 0;
		jp.getValueByKey(jp.getOuterObject(), "setting", setting);

		Log.info("received setting=%d", setting);
	});

	// You must call this from setup!
	deviceNotesHelper.setup();
}

void loop() {

	// You must call this from loop, preferably every loop but at least every few seconds.
	deviceNotesHelper.loop();
}


