
/* IMPLEMENTATION OF SERVER - responsible for sending messages*/

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <os/dev/radio.h>  
#include <string.h>
#include <stdio.h> /* For printf() */


/*-----------------------------CONFIGURATION-----------------------------*/
#define sequence = {1,5,7,9.13}:
#define SEND_INTERVAL (8 * CLOCK_SECOND)

//This adress {{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
static linkaddr_t dest_addr = {{ 0xd6, 0x73, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};  //addresses

//MAC??
#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr = {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

//state message variables
bool ackReceived = false;
int failMsgCount = 0;
int startChannel = 26;
int receivedMsgCount = 0;

/*---------------------------------------------------------------------------*/
PROCESS(server_comm, "Server_comm");
AUTOSTART_PROCESSES(&server_comm);

//NETSTACK_RADIO.on();
//NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL,11);


/*-------------------------------INPUT TO SERVER CALLBACK---------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest)
{ 
  //only receive message of valid type 
  if(len == sizeof(unsigned)) {
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));                  //set data 
    printf("The acknowledgement message resived \n");
    
    //update state 
    receivedMsgCount++;
    ackReceived = true;

  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(server_comm, ev, data)
{
  static struct etimer periodic_timer;  //initialize timer
  static int msgCount = 0;              //initialize message count 

  PROCESS_BEGIN();

  //aner ik hvad der sker her 
  #if MAC_CONF_WITH_TSCH
    if (linkaddr_cmp(&coordinator_addr,&linkaddr_node_addr)) {
      printf("Server set as coordinator");
      tsch_set_coordinator(true);
    }
    else {
      printf("Server not set as coordinator");
    }
    //tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr)); //make coordinator if equal
  #endif /* MAC_CONF_WITH_TSCH */


  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&msgCount;
  nullnet_len = sizeof(msgCount);
  nullnet_set_input_callback(input_callback);

  //If addresses not equal
  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) { 
    printf("LLAddress and DestAddr not equal. Proceeding..");
    etimer_set(&periodic_timer, SEND_INTERVAL);
    
    
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      
      //check if message is received 
      if (ackReceived == true)
      {
        failMsgCount = 0;
      } else {
        failMsgCount ++;
        printf("No acknowledgement message received before timeout \n");
      }

      //if failmsgcount is above boundary, start channel hopping on given sequence
      if (failMsgCount >= 5) { 
        printf("Attempting to change channel");
        startChannel++;                             //change to next channel 

        //try to change channel 
        if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startChannel) != RADIO_RESULT_OK){
          printf("Issue changing channel");
        }
        else {
          printf("\n Succesfully changed channel to %u",startChannel);
          failMsgCount = 0; //reset msg count
        }
      }

      //transmitting message 
      printf("Sending %u to client \n", msgCount);
      NETSTACK_NETWORK.output(&dest_addr);
      msgCount++;
      ackReceived = false;

      //reset timer 
      etimer_reset(&periodic_timer);
    }
  }
  
  printf("Destination and current address are equal");
  PROCESS_END();
}
