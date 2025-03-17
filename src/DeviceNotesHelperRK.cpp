#include "DeviceNotesHelperRK.h"

DeviceNotesHelper::DeviceNotesHelper() {
}

DeviceNotesHelper::~DeviceNotesHelper() {

}

DeviceNotesHelper &DeviceNotesHelper::withGetPeriodic(unsigned long periodSecs) {
	getFrequency = GetFrequency::PERIODIC;
	getPeriodicPeriodMs = periodSecs * 1000;

	if (getPeriodicPeriodMs < 10000) {
		getPeriodicPeriodMs = 10000;
	}
	return *this;
}


void DeviceNotesHelper::setup() {
	String name;

	if (prefixWithDeviceId) {
		name = String::format("%s/%s", System.deviceID().c_str(), subscriptionName);
	}
	else {
		name = subscriptionName;
	}

	Particle.subscribe(name, &DeviceNotesHelper::hookResponseHandler, this, MY_DEVICES);
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

	if (!data[0]) {
		// If there is no data (the device notes were never set), create an empty object
		// so it will parse correctly
		jp.clear();
		jp.addString("{}");
	}
	else {
		jp.addChunkedData(event, data);
	}

	if (jp.parse()) {
		// Looks valid (we received all parts)
		hasValidData = true;
		callDataUpdatedCallback = true;
		// Log.info("successfully received and parsed data");
	}
}


