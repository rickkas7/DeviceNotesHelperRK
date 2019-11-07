#ifndef __DEVICENOTESHELPERRK_H
#define __DEVICENOTESHELPERRK_H

#include "Particle.h"

#include "JsonParserGeneratorRK.h"

class DeviceNotesHelper {
public:
	/**
	 * @brief Construct the helper object
	 *
	 * Typically you create only one, and as global variable.
	 */
	DeviceNotesHelper();

	/**
	 * @brief Destructor.
	 *
	 * Not normally needed as you create one from a global variable.
	 */
	virtual ~DeviceNotesHelper();

	/**
	 * @brief Call this method from setup(). Required!
	 */
	void setup();

	/**
	 * @brief Call this method from loop(). Required!
	 */
	void loop();

	/**
	 * @brief Sets the get data at boot flag. Default is manually.
	 *
	 * You should call this before setup().
	 */
	DeviceNotesHelper &withGetAtBoot() { getFrequency = GetFrequency::AT_BOOT; return *this; };

	/**
	 * @brief Gets data periodically. Default is manually.
	 *
	 * @param periodSecs The number of seconds between fetches. Minimum is 10 seconds.
	 */
	DeviceNotesHelper &withGetPeriodic(unsigned long periodSecs);

	/**
	 * @brief Sets the buffer size.
	 *
	 * It is most efficient if you set this to be larger than the largest data you will receive.
	 *
	 * If you only get data (don't put it to the cloud) this can be a multiple part response
	 * that is quite large.
	 *
	 * If you put data to the cloud, then you're limited to 622 bytes, the size of a single publish.
	 */
	DeviceNotesHelper &withBufferSize(size_t bufferSize) { jp.allocate(bufferSize); return *this; };

	/**
	 * @brief Register a callback to be called when the device notes have been successfully retrieved
	 *
	 * @param callback A function or lambda function to be called when the data has been received and parsed.
	 *
	 * The function prototype for the callback is:
	 *
	 *   void callback(JsonParser &jp);
	 *
	 * The callback is called from within the loop() method (not from the subscription callback) so
	 * there are fewer concurrency issues to worry about in your callback. In particular, it's safe to
	 * publish from the data updated callback if you want to.
	 *
	 * You can only register one callback. If call it again, the existing callback will be replaced.
	 * Setting it to 0 (NULL) removes the callback.
	 */
	DeviceNotesHelper &withDataUpdatedCallback(std::function<void(JsonParser &)> callback) { dataUpdatedCallback = callback; return *this; };


	/**
	 * @brief Change the event name used for the subscription
	 *
	 * @param name A c-string containing the name. This string will not be modified. It also will not
	 * be copied so it must remain valid after this call returns. Since you'll typically pass in a string
	 * constant that will be fine. You must generate a name in a stack allocated buffer.
	 *
	 * Default is "DeviceNotesResponse". This must match the hook-response field in the DeviceNotesGet webhook.
	 */
	DeviceNotesHelper &withSubscriptionName(const char *name) { subscriptionName = name; return *this; };


	/**
	 * @brief Change the event name used for the get event
	 *
	 * @param name A c-string containing the name. This string will not be modified. It also will not
	 * be copied so it must remain valid after this call returns. Since you'll typically pass in a string
	 * constant that will be fine. You must generate a name in a stack allocated buffer.
	 *
	 * Default is "DeviceNotesGet". This must match the event field in the DeviceNotesGet webhook.
	 */
	DeviceNotesHelper &withGetEventName(const char *name) { getEventName = name; return *this; };

	/**
	 * @brief Change the event name used for the put event
	 *
	 * @param name A c-string containing the name. This string will not be modified. It also will not
	 * be copied so it must remain valid after this call returns. Since you'll typically pass in a string
	 * constant that will be fine. You must generate a name in a stack allocated buffer.
	 *
	 * Default is "DeviceNotesPut". This must match the event field in the DeviceNotesPut webhook.
	 */
	DeviceNotesHelper &withPutEventName(const char *name) { putEventName = name; return *this; };

	/**
	 * @brief Get data from the cloud
	 *
	 * This is typically needed if using GetFrequency::MANUAL. It can also be used in AT_BOOT or PERIODIC modes
	 * to request data manually. If using PERIODIC mode, this will adjust the period to start from now.
	 */
	void getFromCloud() { doGetFromCloud = true; }

	/**
	 * @brief Put the data to the cloud
	 *
	 * Since you can't send multi-part data to the cloud, this is limited to 622 bytes.
	 */
	void putToCloud();

	/**
	 * @brief Returns true if the data has been successfully parsed
	 *
	 * If there are multiple parts to the get requests, this is true after the last part.
	 * If ther data in the device notes is not valid JSON, this will never be true.
	 */
	bool getHasValidData() const { return hasValidData; };

	/**
	 * @brief Access the JsonParser object.
	 *
	 * This is used to get data that was sent and also to modify the data. The data is updated when the getFromCloud()
	 * method is called, or at the appropriate get frequency (at boot or periodic).
	 */
	JsonParser &getJsonParser() { return jp; };


	/**
	 * @brief The frequency for how often to get
	 *
	 * This is set by withGetAtBook() and withGetPeriodic() so you typically don't need to use these
	 * constants directly. Default is MANUAL.
	 */
	enum class GetFrequency {
		MANUAL,
		AT_BOOT,
		PERIODIC
	};

protected:
	/**
	 * @brief Used internally to subscribe to the subscriptionName event ("DeviceNotesResponse")
	 */
	void hookResponseHandler(const char *event, const char *data);


	GetFrequency getFrequency = GetFrequency::MANUAL;			//!< How often to get the value from the cloud.
	bool getAtBootCompleted = false;							//!< If using GetFrequency::AT_BOOT, true if it's been done
	unsigned long getPeriodicPeriodMs = 60000;					//!< Period in milliseconds for GetFrequency::PERIODIC
	unsigned long lastGetFromCloud = 0;							//!< Last millis() value we got data from the cloud
	JsonParser jp;												//!< JsonParser object. Parses data from cloud and is updated by JsonModifier.
	std::function<void(JsonParser &)> dataUpdatedCallback = 0;	//!< Function that gets called when data is gotten from the cloud.
	bool callDataUpdatedCallback = false;						//!< Flag to indicate that dataUpdateCallback should be called from loop()
	bool doGetFromCloud = false;								//!< Flag to indicate that we should get data from cloud (when Particle.connected)
	bool hasValidData = false;									//!< True after data has been downloaded from the cloud the first time.
	const char *subscriptionName = "DeviceNotesResponse";		//!< Event name used for subscription. Set before setup()
	const char *getEventName = "DeviceNotesGet";				//!< Event name used to get data from the cloud
	const char *putEventName = "DeviceNotesPut";				//!< Event name to put data to the cloud
};


#endif /* __DEVICENOTESHELPERRK_H */
