#include "contiki.h"

#include "random.h"

#include "net/netstack.h"
#include "sys/rtimer.h"
#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "./cc2420.h"

#include <stdio.h>
#include <string.h>

#define ONE_MS RTIMER_SECOND/1024
#define TS_LEN  6*ONE_MS

#define JAMMER_PACKET_LEN 120

typedef struct{
     char data[JAMMER_PACKET_LEN];
}jpacket_t;

//static struct rtimer jammer_timer;

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
  
  //set channel..
  cc2420_set_channel(26);

  //Load data into packet
  jpacket_t jpacket;
  memset(&jpacket, 0, sizeof(jpacket_t));
  strcpy(jpacket.data, "Antonio Gonga is taking down your network.");
    
  //Constant jammer - jams with fastest interval possible  
  while(1) {
    
     //CALL Radio Driver to transmit the Packet..
     cc2420_driver.send((void*)&jpacket, JAMMER_PACKET_LEN); 
    
  }
  PROCESS_END();
}

