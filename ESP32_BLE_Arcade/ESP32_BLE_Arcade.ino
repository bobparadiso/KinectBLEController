// Prior work this is based on:
// https://gist.github.com/GameDragon2k/90709fbe6b15e66c1b274dde3e1b2439
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    Gamepad coding by Game Dragon
*/

//Used to allow control over serial, in the end used to allow a Kinect to act as a Bluetooth gamepad.
//Videos:
//	https://www.youtube.com/watch?v=G7VoD0XVeks
//	https://www.youtube.com/watch?v=vBMoQwGu6mU
//	https://www.youtube.com/watch?v=IZzk3EjRERw

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEHIDDevice.h>
#include <BLE2902.h>
#include "Terminal.h"
#include "Trace.h"
#include "gamepad_report.h"

static BLEHIDDevice *pHID;
BLEServer *pServer;
BLECharacteristic *input;

bool deviceConnected = false;
bool oldDeviceConnected = false;

bool operator!=(const gamepad_report_t& lhs, const gamepad_report_t& rhs)
{
  return memcmp(&lhs, &rhs, sizeof(gamepad_report_t)); 
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("device connected");
	  
		// workaround after reconnect (see comment below) 
		BLEDescriptor *desc = input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
		uint8_t val[] = {0x01, 0x00};
		desc->setValue(val, 2);	  

		deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("device disconnected");
      deviceConnected = false;
    }
};

gamepad_report_t oldValue, newValue;

// Set Report Map
const uint8_t reportMap[] = {
	0x05, 0x01,			//	USAGE_PAGE (Generic Desktop)
	0x09, 0x05,			//	USAGE (Game Pad)
	0xa1, 0x01,			//	COLLECTION (Application)
	0x85, 0x01,			//		REPORT_ID (1)

	//WARNING: DO NOT ONLY CHANGE COMMENTS

	0x15, 0x00,			//		LOGICAL_MINIMUM (0)
	0x25, 0x01,			//		LOGICAL_MAXIMUM (1)
	0x95, 0x08,			//		REPORT_COUNT (8)
	0x75, 0x01,			// 		REPORT_SIZE (1)
	0x05, 0x09,			//		USAGE_PAGE (Button)
	0x19, 0x01,			//		USAGE_MINIMUM (Button 1)
	0x29, 0x08,			//		USAGE_MAXIMUM (Button 8)
	0x81, 0x02,			//		INPUT (Data, Var, Abs)
	
	0x05, 0x01,			//		USAGE_PAGE (Generic Desktop)
	0xa1, 0x00,			//		COLLECTION (Physical)
	0x09, 0x30,			//          USAGE (X)
	0x09, 0x31,			//          USAGE (Y)
	0x15, 0x81,			//          LOGICAL_MINIMUM (-127)
	0x25, 0x7f,			//          LOGICAL_MAXIMUM (127)
	0x95, 0x02,			//          REPORT_COUNT (2)
	0x75, 0x08,			//          REPORT_SIZE (8)
	0x81, 0x02,			//          INPUT (Data, Var, Abs)
	0xc0,				//		END_COLLECTION

	0x09, 0x39,			//		Usage (Hat switch)
	0x15, 0x00,			//		Logical Minimum (0)
	0x25, 0x07,			//		Logical Maximum (7)
	0x75, 0x04,			//		Report Size (4)
	0x95, 0x01,			//		Report Count (1)
	0x81, 0x02,			//		INPUT (Data, Var, Abs)

	// PADDING for byte alignment
	0x75, 0x01,			//		REPORT_SIZE (1)
	0x95, 0x04,			//		REPORT_COUNT (4)
	0x81, 0x03,			//		INPUT (Constant, Var, Abs)

	0xc0				//	END_COLLECTION
};

//
void setInputValue()
{
	//uint8_t a[] = {newValue.buttons, (newValue.buttons >> 8), newValue.x, newValue.y, newValue.z, newValue.rx, newValue.ry, newValue.rz, newValue.hatSwitch};
	uint8_t a[] = {newValue.buttons, newValue.x, newValue.y, newValue.hatSwitch};
	input->setValue(a, sizeof(a));
}

//
void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Depth Sensor Gamepad"); // Give it a name

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Instantiate HID Device
  pHID = new BLEHIDDevice(pServer);
  input = pHID->inputReport(1);
  
  pHID->manufacturer()->setValue("ESP32");
  pHID->pnp(0x01,0x000,0x0000,0x0001);
  pHID->hidInfo(0x00,0x01);

  BLESecurity *pSecurity = new BLESecurity();
  //pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);
	
  pHID->reportMap((uint8_t*)reportMap, sizeof(reportMap));
  int numReport = sizeof(reportMap);
  Serial.println(numReport);

  // Start the service
  pHID->startServices();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);
  pAdvertising->addServiceUUID(pHID->hidService()->getUUID());
  pAdvertising->start();
  
  Serial.println("Waiting a client connection to notify...");
}

uint32_t lastTx;

//
void loop()
{
	processSerial();
		
	if (deviceConnected)
	{
		uint8_t valueUpdated = newValue != oldValue;
		if (valueUpdated)
		{
			Serial.println("---");
			//tracef("buttons:%04x x:%d y:%d z:%d rx:%d ry:%d rz:%d hat:%01x\r\n", newValue.buttons, newValue.x, newValue.y, newValue.z, newValue.rx, newValue.ry, newValue.rz, newValue.hatSwitch);      
			tracef("buttons:%02x x:%d y:%d hat:%01x\r\n", newValue.buttons, newValue.x, newValue.y, newValue.hatSwitch);      
			oldValue = newValue;
		}

		if (valueUpdated || (millis() - lastTx > 500))
		{
			setInputValue();
			input->notify();
			lastTx = millis();
		}		
	}

  // Connecting
  if (deviceConnected && !oldDeviceConnected) 
  {
	  Serial.println("connected...");
      oldDeviceConnected = deviceConnected;
  }

  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) 
  {
	  Serial.println("disconnected...");
      //delay(500); // give the bluetooth stack the chance to get things ready
      //pServer->startAdvertising();
      //Serial.println("restart advertising");
      oldDeviceConnected = deviceConnected;
  }
  
	static uint32_t lastAd = 0;
	if (!pServer->getConnectedCount() && (millis() - lastAd > 1000))
	{
		Serial.println("restart advertising");
		pServer->startAdvertising();
		lastAd = millis();
	}
}
