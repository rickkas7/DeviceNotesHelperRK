#include "DeviceNotesHelperRK.h"

DeviceNotesHelper::DeviceNotesHelper() {
}

DeviceNotesHelper::~DeviceNotesHelper() {

}

DeviceNotesHelper &DeviceNotesHelper::withGetPeriodic(unsigned long periodSecs) {
	getFrequency = GetFrequency::PERIODIC;
	getPeriodicPeriodMs = periodSecs * 1000;

	if (periodMs < 10000) {
		periodMs = 10000;
	}
	return *this;
}


void DeviceNotesHelper::setup() {

	Particle.subscribe(subscriptionName, &DeviceNotesHelper::hookResponseHandler, this, MY_DEVICES);
}


void DeviceNotesHelper::loop() {

	switch(getFrequency) {
	case GetFrequency::AT_BOOT:
		if (!getAtBootCompleted && Particle.connected()) {
			getAtBootCompleted = true;
			doGetFromCloud = true;
		}
		break;

	case GetFrequency::PERIODIC:
		if (Particle.connected()) {
			if (millis() - lastGetFromCloud >= getPeriodicPeriodMs) {
				lastGetFromCloud = millis();
				doGetFromCloud = true;
			}
		}
		break;
	}

	if (doGetFromCloud && Particle.connected()) {
		doGetFromCloud = false;
		Particle.publish(getEventName, "", PRIVATE);
		lastGetFromCloud = millis();
	}

	if (callDataUpdatedCallback) {
		callDataUpdatedCallback = false;
		if (dataUpdatedCallback) {
			dataUpdatedCallback(jp);
		}
	}
}

void DeviceNotesHelper::putToCloud() {
	jp.nullTerminate();

	Particle.publish(putEventName, jp.getBuffer(), PRIVATE);
}


void DeviceNotesHelper::hookResponseHandler(const char *event, const char *data) {

	int responseIndex = 0;

	const char *slashOffset = strrchr(event, '/');
	if (slashOffset) {
		responseIndex = atoi(slashOffset + 1);
	}

	if (responseIndex == 0) {
		jp.clear();
		if (data[0]) {
			jp.addString(data);
		}
		else {
			// If there is no data (the device notes were never set), create an empty object
			// so it will parse correctly
			jp.addString("{}");
		}
	}
	else {
		jp.addString(data);
	}

	if (jp.parse()) {
		// Looks valid (we received all parts)
		hasValidData = true;
		callDataUpdatedCallback = true;
		// Log.info("successfully received and parsed data");
	}
}


