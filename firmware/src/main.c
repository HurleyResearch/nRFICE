/*
MIT License

Copyright (c) 2024 Hurley Research LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */
#include "uart_async_adapter.h"

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <dk_buttons_and_leds.h>

#include <zephyr/settings/settings.h>

#include <stdio.h>

#include <zephyr/logging/log.h>

#include "spimflash.h"
#include "utils.h"
#define testdelay k_sleep(K_MSEC(1))
#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

#if CONFIG_BT_NUS_UART_ASYNC_ADAPTER
UART_ASYNC_ADAPTER_INST_DEFINE(async_adapter);
#else
// static const struct device *const async_adapter;
#endif


#define NCOMMANDS 10

#define COMMAND_READ_FLASH_BT 110
#define COMMAND_READ_FLASH_STATUS 111
#define COMMAND_WRITE_FLASH 112
#define COMMAND_ERASE_FLASH 113
#define COMMAND_TEST_FLASH 114
#define COMMAND_HARDRESETICE 115
#define COMMAND_SOFTRESETEICE 116
#define COMMAND_CRESETBLOW 117
#define COMMAND_CRESETBHIGH 118
#define COMMAND_TESTREADFLASH 119
#define COMMAND_TESTWRITEFLASH 127
#define COMMAND_ERASEZERO 131

#define COMMAND_TESTBTSEND 132

#define COMMAND_READ_FLASH_SERIAL 120
#define COMMAND_RAMTEST 121
#define COMMAND_RAMTESTLONG 122

#define COMMAND_SETRGB 210
#define COMMANDLENGTH 128
uint8_t commandBytes[COMMANDLENGTH];

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	if (err)
	{
		lg("connection failed\n");
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	current_conn = bt_conn_ref(conn);

    struct bt_conn_info info;
	bt_conn_get_info(current_conn,&info);
	lgv("set conn val %i\n",info.id);

}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	lg("disconnected\n");
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected: %s (reason %u)", addr, reason);
	if (auth_conn)
	{
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}
	if (current_conn)
	{
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
							 enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	if (!err)
	{
		LOG_INF("Security changed: %s level %u", addr, level);
	}
	else
	{
		LOG_WRN("Security failed: %s level %u err %d", addr,
				level, err);
	}
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	.security_changed = security_changed,
#endif
};

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Passkey for %s: %06u", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];
	auth_conn = bt_conn_ref(conn);
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Passkey for %s: %06u", addr, passkey);
	LOG_INF("Press Button 1 to confirm, Button 2 to reject.");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Pairing cancelled: %s", addr);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Pairing completed: %s, bonded: %d", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Pairing failed conn: %s, reason %d", addr, reason);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
static struct bt_conn_auth_info_cb conn_auth_info_callbacks;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////








uint8_t flashTestBuf[128];
uint8_t readBufBT[20];
uint8_t sendBufBT[20];

uint32_t waitCount = 0;

uint32_t flashReadStartAddress = 0;
uint32_t flashReadLength = 0;
uint32_t flashReadCurrentAddress = 0;
uint32_t flashReadSessionID = 0;
void flashGPIOICE()
{
	gpioConfigInputNoPull(ICE_SCK); 
	gpioConfigInputNoPull(ICE_SSB); 
									
	gpioConfigInputNoPull(ICE_SDI);
	gpioConfigInputNoPull(ICE_SDO);
}

void flashGPIONRF()
{
	gpioConfigOutputDrive(ICE_SCK, DRIVEH0H1); 
	pinClear(ICE_SSB);
	gpioConfigOutputDrive(ICE_SSB, DRIVEH0H1);
	pinSet(ICE_SSB);

	gpioConfigOutputDrive(ICE_SDI, DRIVEH0H1);
	pinClear(ICE_SDI);

	gpioConfigInputNoPull(ICE_SDO);
}

void commandReadFlashBT(uint8_t *data)
{
	pinClear(ICE_CRESETB);
	flashReadStartAddress = unpack32(data, 4);
	flashReadLength = unpack32(data, 8);
	flashReadSessionID = unpack32(data, 12);
	flashReadCurrentAddress = flashReadStartAddress;
	lgv("readFlashBT %d\n", flashReadSessionID);
}
char hexChars[] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
};
void bytesToHexString(uint8_t *bytes, char *str, int len)
{
	for (int i = 0; i < len; i++)
	{
		uint8_t b = bytes[i];
		str[i * 2] = hexChars[(b >> 4) & 0x0f];
		str[i * 2 + 1] = hexChars[b & 0x0f];
	}
}
void commandReadFlashStatus()
{
	pinClear(ICE_CRESETB);

	uint8_t status = flashReadStatus();
	lgv("yayflash status %d\n", status);

}
uint32_t maxWaitCount = 0;
uint8_t readCheckBuf[16];
void commandWriteFlash(uint8_t *data, uint8_t len)
{
	pinClear(ICE_CRESETB);

	uint8_t status = flashReadStatus();

	uint32_t startAddress = unpack24(data, 1);

	if (startAddress % 0x1000 == 0)
	{
	}

	flashWriteEnable();
	flashProgram(data, startAddress, len);

	waitCount = 0;
	status = flashReadStatus();
	while (status > 0)
	{
		status = flashReadStatus();
		waitCount++;
	}
	if (waitCount > maxWaitCount)
	{
		maxWaitCount = waitCount;
	}

	flashWriteDisable();
	flashRead(readCheckBuf, startAddress, len);
	uint32_t badX = 0;
	uint8_t allGood = 1;
	for (uint8_t x = 0; x < len; x++)
	{
		if (readCheckBuf[x] != data[x + 4])
		{
			allGood = 0;
			badX = x;
			x = len;
		}
	}
	if (allGood == 0)
	{

		lgv("allgood zero: \n", badX);

		flashWriteEnable();
		flashProgram(data, startAddress, len);

		flashWriteDisable();
		flashRead(readCheckBuf, startAddress, len);

		uint8_t allGoodAgain = 1;
		for (uint8_t x = 0; x < len; x++)
		{
			if (readCheckBuf[x] != data[x + 4])
			{
				allGoodAgain = 0;
				badX = x;
				x = len;
			}
		}
		if (allGoodAgain == 0)
		{

			lgv("allgood zero again: %d\n", badX);
		}
	}

	status = flashReadStatus();
	if (startAddress % 0x1000 == 0)
	{
	}
}

void commandEraseFlash(uint8_t *data)
{
	pinClear(ICE_CRESETB);

	uint32_t addr = unpack24(data, 1);

	uint8_t status = flashReadStatus();
	lgv("erase %d", status);
	waitCount = 0;
	flashWriteEnable();
	flashEraseSector(addr);

	status = flashReadStatus();

}

void commandEraseZero()
{
	pinClear(ICE_CRESETB);

	flashWriteEnable();
	flashEraseSector(0);
}

uint32_t insideFlashReadSession = 0;
void flashReadSession()
{

	insideFlashReadSession = 1;
	if (flashReadCurrentAddress < flashReadLength + flashReadStartAddress)
	{
		lg("readFlashSession");
		uint8_t status = flashReadStatus();
		if (status != 0)
		{

			lgv("abort %d\n", status);
			flashReadCurrentAddress = flashReadLength + flashReadStartAddress;
			insideFlashReadSession = 0;
			return;
		}
		flashRead(readBufBT, flashReadCurrentAddress, 16);
		for (uint8_t x = 0; x < 16; x++)
		{
			sendBufBT[x + 4] = readBufBT[x];
		}
		sendBufBT[0] = COMMAND_READ_FLASH_BT;
		pack24(flashReadCurrentAddress, sendBufBT, 1);

		int wth = bt_nus_send(current_conn, sendBufBT, 20);
		if (wth != 0)
		{
			lgv("prblm session %i\n", wth);
			flashReadCurrentAddress = flashReadLength + flashReadStartAddress;
		}
		else
		{
			flashReadCurrentAddress += 16;
		}
	}
	insideFlashReadSession = 0;
}

void testFlashRead(void)
{

	lg("testFlashRead");
	uint8_t status = flashReadStatus();
	lgv("test status read: %i\n", status);

	flashRead(readBufBT, 0, 16);
	for (uint8_t x = 0; x < 16; x++)
	{
		testdelay;
		lgv("readbuf %i\n", readBufBT[x]);
	}
}

void testBTSend(void)
{


	struct bt_conn_info info;
	bt_conn_get_info(current_conn,&info);
	lgv("sendID %i\n", info.id);

	for (uint8_t x = 0; x < 20; x++)
	{
		sendBufBT[x] = x;
	}
	sendBufBT[0] = COMMAND_READ_FLASH_BT;

	int wth = bt_nus_send(current_conn, sendBufBT, 20);

	if (wth != 0)
	{
		lgv("fprobnus %i\n", wth);
	}
	else
	{
		lgv("good nus %i\n", wth);
	}
}

void testFlashWrite(void)
{

	lg("testFlashWrite");
	uint8_t status = flashReadStatus();
	lgv("testFlashWrite: %i\n", status);
	for (int x = 0; x < 64; x++)
	{
		flashTestBuf[x] = x;
		flashWriteEnable();
	}
	flashProgram(flashTestBuf, 0, 64);
	lg("after flash program");
}

#define testdelay k_sleep(K_MSEC(1))
void writeEBRByte(uint8_t byte) {
    for (uint8_t b = 0; b < 8; b++) {
        if (((byte >> (7 - b)) & 0x01) == 0x01) {
            pinSet(ICEAPP_SPIMOSI);
        } else {
            pinClear(ICEAPP_SPIMOSI);
        }
        testdelay;
        pinSet(ICEAPP_SPICLK);
        testdelay;
        pinClear(ICEAPP_SPICLK);
    }
}
void writeEBR(uint8_t addr, uint16_t val)
{
	testdelay;
	pinClear(ICEAPP_SPICLK);
	pinClear(ICEAPP_SPIMOSI);
	pinClear(ICEAPP_SPICSN);
	testdelay;
	writeEBRByte(0x10); 
	testdelay;
	writeEBRByte(addr);
	testdelay;
	writeEBRByte((val >> 8) & 0xff);
	testdelay;
	writeEBRByte(val & 0xff);
	testdelay;

	pinSet(ICEAPP_SPICSN);
	pinClear(ICEAPP_SPICLK);
	pinClear(ICEAPP_SPIMOSI);
}


void writeFPGARGB(uint8_t addr, uint8_t val)
{
	testdelay;
	pinClear(ICEAPP_SPICLK);
	pinClear(ICEAPP_SPIMOSI);
	pinClear(ICEAPP_SPICSN);
	testdelay;
	writeEBRByte(0x12 + addr); 
	testdelay;
	writeEBRByte(addr);
	testdelay;
	writeEBRByte(0);
	testdelay;
	writeEBRByte(val & 0xff);
	testdelay;

	pinSet(ICEAPP_SPICSN);
	pinClear(ICEAPP_SPICLK);
	pinClear(ICEAPP_SPIMOSI);
	testdelay;
}



void setRGB(uint8_t *data) {
    uint8_t r = data[1];
    uint8_t g = data[2];
    uint8_t b = data[3];

    writeFPGARGB(0,r);
    writeFPGARGB(1,g);
    writeFPGARGB(2,b);


}


void processCommand(uint8_t payloadSize)
{

	uint8_t command = commandBytes[0];
      lgv("cmd %i\n", command);
	if (command == COMMAND_READ_FLASH_BT)
	{
		commandReadFlashBT(commandBytes);
	}
	else if (command == COMMAND_READ_FLASH_STATUS)
	{

		commandReadFlashStatus();
	}
	else if (command == COMMAND_READ_FLASH_SERIAL)
	{
		// commandReadFlashSerial(commandBytes);
	}
	else if (command == COMMAND_ERASEZERO)
	{
		commandEraseZero();
	}
	else if (command == COMMAND_RAMTEST)
	{
		// uint8_t ramAddr = commandBytes[1];
		// uint16_t ramData = unpack16(commandBytes, 2);
		// commandRamTest(ramAddr, ramData);
	}
	else if (command == COMMAND_RAMTESTLONG)
	{
		// commandRamTestLong();
	}
	else if (command == COMMAND_WRITE_FLASH)
	{
		commandWriteFlash(commandBytes, payloadSize);
	}
	else if (command == COMMAND_ERASE_FLASH)
	{
		commandEraseFlash(commandBytes);
	}
	else if (command == COMMAND_TEST_FLASH)
	{
		pinClear(ICE_CRESETB);
		uint8_t status = flashReadStatus();
		flashWriteEnable();
		status = flashReadStatus();
		flashWriteDisable();
		status = flashReadStatus();
	}
	else if (command == COMMAND_HARDRESETICE)
	{
		pinClear(NRF_LEDR);
		flashGPIOICE();
		k_sleep(K_MSEC(100));
		pinSet(ICE_CRESETB);
		k_sleep(K_MSEC(100));
		pinClear(ICE_CRESETB);
		k_sleep(K_MSEC(100));
		pinSet(ICE_CRESETB);
		pinSet(NRF_LEDR);
	
	}
	else if (command == COMMAND_SOFTRESETEICE)
	{

	


		pinClear(NRF_LEDG);
		pinClear(ICESOFTRESET);
		k_sleep(K_MSEC(100));
		pinSet(ICESOFTRESET);

		k_sleep(K_MSEC(100));
		pinClear(ICESOFTRESET);
		pinSet(NRF_LEDG);
	}
	else if (command == COMMAND_CRESETBLOW)
	{
		pinClear(ICE_CRESETB);
	}
	else if (command == COMMAND_CRESETBHIGH)
	{
		pinSet(ICE_CRESETB);
	}
	else if (command == COMMAND_TESTBTSEND)
	{
		testBTSend();
	}
	else if (command == COMMAND_TESTREADFLASH)
	{

		testFlashRead();

	}
	else if (command == COMMAND_TESTWRITEFLASH)
	{

		testFlashWrite();

	}
	else if (command == COMMAND_SETRGB)
	{
		setRGB(commandBytes);
	}
}
////////////////////////////////////////////////////////////

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{

	char addr[BT_ADDR_LE_STR_LEN] = {0};



	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
	for (uint8_t x = 0; x < 20; x++)
	{
		commandBytes[x] = data[x];
	}
	processCommand(16);

}

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

void error(void)
{
	while (true)
	{
		k_sleep(K_MSEC(1000));
	}
}

uint32_t hbi = 0;
void heartBeat(void)
{
	if (hbi % 2 == 0)
	{
		pinClear(NRF_LEDR);
	}
	else
	{
		pinSet(NRF_LEDR);
	}
	hbi++;

	if (insideFlashReadSession == 0)
	{
		flashReadSession();
	}
}

int main(void)
{
	
	int err = 0;

	gpioConfigOutputSet(ICEAPP_SPICSN);
	gpioConfigOutputClear(ICEAPP_SPICLK);
	gpioConfigOutputClear(ICEAPP_SPIMOSI);
	gpioConfigInputNoPull(ICEAPP_SPIMISO);
	gpioConfigOutputClear(ICESOFTRESET);
	gpioConfigOutputClear(ICE_CRESETB);

	gpioConfigOutputSet(NRF_LEDR);
	gpioConfigOutputSet(NRF_LEDG);
	gpioConfigOutputSet(NRF_LEDB);
	setupSeggerUART();
	lg("top of main");
	if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED))
	{
		err = bt_conn_auth_cb_register(&conn_auth_callbacks);
		if (err)
		{
			printk("Failed to register authorization callbacks.\n");
			return 0;
		}

		err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
		if (err)
		{
			printk("Failed to register authorization info callbacks.\n");
			return 0;
		}
	}
	err = bt_enable(NULL);
	if (err)
	{
		error();
	}
	LOG_INF("Bluetooth initialized");
	k_sem_give(&ble_init_ok);
	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}
	err = bt_nus_init(&nus_cb);
	if (err)
	{
		LOG_ERR("Failed to initialize UART service (err: %d)", err);
		return 0;
	}
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)", err);
		return 0;
	}
	spimFlashInit();

	for (;;)
	{

		k_sleep(K_MSEC(100));
		heartBeat();
	}
}
