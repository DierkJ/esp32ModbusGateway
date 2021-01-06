/**
**********************************************************************************************************************************************************************************************************************************
* @file:	lorawan.cpp
*
* @brief:	lorawan functions for Modbus Gateway
*
*
* @author:	Dierk Arp
* @date:	20201230 16:01:24 
* @version:	1.0
*
* @copyright:	(c) 2021 Team HAHIS
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
**********************************************************************************************************************************************************************************************************************************
**/
static const char TAG[] = __FILE__;


#include "globals.h"

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <arduino_lmic_hal_boards.h>


#include "lorawan.h"
#include "loraconf.h"

// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 60;
static osjob_t sendjob;

lora_status_t g_LoraData;


// static TX Buffer
static uint8_t bTxBuffer[32];

// Function to do a byte swap in a byte array
static void RevBytes(unsigned char *b, size_t c) 
{
  u1_t i;
  for (i = 0; i < c / 2; i++) 
  {
    unsigned char t = b[i];
    b[i] = b[c - 1 - i];
    b[c - 1 - i] = t;
  }
}

void os_getArtEui (u1_t* buf) 
{ 
  memcpy_P(buf, APPEUI, 8);
  RevBytes(buf, 8); // TTN requires it in LSB First order, so we swap bytes
}


/* DevEUI generator using devices's MAC address */
static void gen_lora_deveui(uint8_t *pdeveui) 
{
  uint8_t *p = pdeveui, dmac[6];
  ESP_ERROR_CHECK(esp_efuse_mac_get_default(dmac));
  // deveui is LSB, we reverse it so TTN DEVEUI display
  // will remain the same as MAC address
  // MAC is 6 bytes, devEUI 8, set middle 2 ones
  // to an arbitrary value
  *p++ = dmac[5];
  *p++ = dmac[4];
  *p++ = dmac[3];
  *p++ = 0xfe;
  *p++ = 0xff;
  *p++ = dmac[2];
  *p++ = dmac[1];
  *p++ = dmac[0];
}


// This should also be in little endian format, see above.
void os_getDevEui (u1_t* buf) 
{ 
  int i = 0, k = 0;
  memcpy(buf, DEVEUI, 8); // get fixed DEVEUI from loraconf.h
  for (i = 0; i < 8; i++) {
    k += buf[i];
  }

  if (k) 
  {
    RevBytes(buf, 8); // use fixed DEVEUI and swap bytes to LSB format
  } 
  else 
  {
    gen_lora_deveui(buf); // generate DEVEUI from device's MAC
  }
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
void os_getDevKey (u1_t* buf) 
{  
  memcpy_P(buf, APPKEY, 16);
}

// Display a key
static void printKey(const char *name, const uint8_t *key, uint8_t len, bool lsb) 
{
  const uint8_t *p;
  char keystring[len + 1] = "", keybyte[3];
  for (uint8_t i = 0; i < len; i++) {
    p = lsb ? key + len - i - 1 : key + i;
    snprintf(keybyte, 3, "%02X", *p);
    strncat(keystring, keybyte, 2);
  }
  debugD( "%s: %s", name, keystring);
}

void showLoraKeys(void) 
{
  // LMIC may not have used callback to fill
  // all EUI buffer so we do it here to a temp
  // buffer to be able to display them
  uint8_t buf[32];
  os_getDevEui((u1_t *)buf);
  printKey("DevEUI", buf, 8, true);
  os_getArtEui((u1_t *)buf);
  printKey("AppEUI", buf, 8, true);
  os_getDevKey((u1_t *)buf);
  printKey("AppKey", buf, 16, false);
}

void do_send(osjob_t* j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) 
    {
        debugD( "OP_TXRXPEND, not sending");
    } 
    else 
    {
        //
        // simple data for port #1 first...
        // later: TX queue
        *(float *) &bTxBuffer[0] = g_modBusMeterData.fEnergyIn;
        *(float *) &bTxBuffer[4] = g_modBusMeterData.fEnergyOut;
        *(float *) &bTxBuffer[8] = g_modBusMeterData.fPower;

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, bTxBuffer, 3*sizeof(float), 0);
        g_LoraData.nTX++;
        debugD( "Packet queued");
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

/**
 * @brief Lora LMIC call back. 
 * 
 * @param ev type of event:  one of EV_xxx
 */
void onEvent (ev_t ev) 
{
    switch(ev) 
    {
        case EV_SCAN_TIMEOUT:
            debugD( "EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            debugD( "EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            debugD( "EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            debugD( "EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            debugD( "EV_JOINING");
            break;
        case EV_JOINED:
            debugD( "EV_JOINED");
            {
              g_LoraData.fJoined = true;
              LMIC_getSessionKeys(&g_LoraData.netid, &g_LoraData.devaddr, g_LoraData.nwkKey, g_LoraData.artKey);

              debugD( " netid: %d", g_LoraData.netid);
              debugD( " devaddr: 0x%8.8x", g_LoraData.devaddr);
              printKey("AppSKey", g_LoraData.artKey, 16, false);
              printKey("NwkSKey", g_LoraData.nwkKey, 16, true);
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            
            break;
        case EV_JOIN_FAILED:
            debugD( "EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            debugD( "EV_REJOIN_FAILED");
            break;
            break;
        case EV_TXCOMPLETE:
            debugD( "EV_TXCOMPLETE (includes waiting for RX windows)");
            if (LMIC.txrxFlags & TXRX_ACK)
            {
              g_LoraData.nAck++;
              debugD( "Received ack");
            }
            if (LMIC.dataLen) 
            {
              // TODO:  RX dispatcher
              debugD( "Received %dbytes of payload", LMIC.dataLen);
              g_LoraData.nRX++;
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            debugD( "EV_LOST_TSYNC");
            break;
        case EV_RESET:
            debugD( "EV_RESET");
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            debugD( "EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            debugD( "EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            debugD( "EV_LINK_ALIVE");
            break;
        case EV_TXSTART:
            debugD( "EV_TXSTART");
            break;
        case EV_TXCANCELED:
            debugD( "EV_TXCANCELED");
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            debugD( "EV_JOIN_TXCOMPLETE: no JoinAccept");
            break;

        default:
            debugD( "Unknown event: %d", (unsigned) ev);
            break;
    }
}


/**
 * @brief call lora initialization on startup
 * 
 */
void loraInit() 
{
    debugD( "Starting LORA");

    memset(& g_LoraData, 0, sizeof(g_LoraData));
    //
    // LMIC init using always ttgo_lora32
    //
    const lmic_pinmap *pPinMap = Arduino_LMIC::GetPinmap_ttgo_lora32_v1();

    if (pPinMap == nullptr) 
    {
      debugD( "board not known to library; add pinmap or update getconfig_thisboard.cpp");
      return;
    }

    int iRet = os_init_ex(pPinMap);
    debugD( "Lora Init returns: %d", iRet);
    
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);

}

void loraHandle()
{
   os_runloop_once();
}

