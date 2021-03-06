#include "DeviceNotesHelperRK.h"

SYSTEM_THREAD(ENABLED);

DeviceNotesHelper deviceNotesHelper;

void printJson(JsonParser &jp);

void setup() {
	Serial.begin();

	// Override settings here
	deviceNotesHelper
		.withBufferSize(2048) 	// Allow a JSON object up to 2048 bytes
		.withGetAtBoot()	 	// Fetch the devices notes once at boot after connecting to the cloud
		.withDataUpdatedCallback(printJson);	// When data arrives, call the printJson function

	// You must call this from setup!
	deviceNotesHelper.setup();
}

void loop() {

	// You must call this from loop, preferably every loop but at least every few seconds.
	deviceNotesHelper.loop();
}


void printIndent(size_t indent) {
	for(size_t ii = 0; ii < 2 * indent; ii++) {
		Serial.printf(" ");
	}
}

void printString(const char *str) {
	Serial.printf("\"");

	for(size_t ii = 0; str[ii]; ii++) {
		if (str[ii] == '"') {
			Serial.printf("\\\"");
		}
		else
			if (str[ii] == '\\') {
				Serial.printf("\\\\");
			}
			else
				if (str[ii] >= 32 && str[ii] < 127) {
					Serial.printf("%c", str[ii]);
				}
				else {
					Serial.printf("\\x%02x", str[ii]);
				}
	}
	Serial.printf("\"");
}

void printJsonInner(JsonParser &jp, const JsonParserGeneratorRK::jsmntok_t *container, size_t indent) {

	switch(container->type) {
	case JsonParserGeneratorRK::JSMN_OBJECT: {
		printIndent(indent);
		Serial.printf("{\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *keyToken;
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getKeyValueTokenByIndex(container, keyToken, valueToken, ii)) {
				break;
			}
			if (ii > 0) {
				Serial.printf(",\n");
			}

			String keyName;
			jp.getTokenValue(keyToken, keyName);

			printIndent(indent + 1);
			printString(keyName);
			Serial.printf(":");
			printJsonInner(jp, valueToken, indent + 1);
		}
		Serial.printf("\n");
		printIndent(indent);
		Serial.printf("}\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_ARRAY: {
		printIndent(indent);
		Serial.printf("[\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getValueTokenByIndex(container, ii, valueToken)) {
				break;
			}
			if (ii > 0) {
				Serial.printf(",\n");
			}
			printIndent(indent + 1);
			printJsonInner(jp, valueToken, indent + 1);
		}
		Serial.printf("\n");
		printIndent(indent);
		Serial.printf("]\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_STRING: {
		Serial.printf("\"");
		for(int ii = container->start; ii < container->end; ii++) {
			Serial.printf("%c", jp.getBuffer()[ii]);
		}
		Serial.printf("\"");
		break;
	}
	case JsonParserGeneratorRK::JSMN_PRIMITIVE: {
		for(int ii = container->start; ii < container->end; ii++) {
			Serial.printf("%c", jp.getBuffer()[ii]);
		}
		break;
	}
	case JsonParserGeneratorRK::JSMN_UNDEFINED:
	default: {
		break;
	}
	}

}

void printJson(JsonParser &jp) {
	printJsonInner(jp, jp.getOuterToken(), 0);
}
