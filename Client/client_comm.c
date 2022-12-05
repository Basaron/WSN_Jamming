
/* IMPLEMENTATION OF CLIENT - responsible for receiving messages */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */

#define EXPECTED_REC_INTERVAL (8 * CLOCK_SECOND)


//state message variables
bool received = false;
int msgCount = 0;
int failCount = 0;
int startChannel = 26;
//static linkaddr_t dest_addr ={{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }}; 


/*---------------------------------------------------------------------------*/
PROCESS(client_comm, "client_comm");
AUTOSTART_PROCESSES(&client_comm);


/*--------------------------CALLBACK TO RECEIVE MESSAGES-------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) 
{
  //only receive messages that are unsigned i.e non-negative 
  if(len == sizeof(unsigned)) {
    received = true;
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));                //set received data 
    
    //incrementing message count 
    printf("The message received is: %u \n", msg);  //print message 
    msgCount ++;

    //acknoledgement messages 
    static int ackmsg = 1;
    nullnet_buf = (uint8_t *)&ackmsg;
    nullnet_len = sizeof(ackmsg);

    //send to stack 
    NETSTACK_NETWORK.output(src);
  }

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(client_comm, ev, data)
{
  PROCESS_BEGIN();  
  static struct etimer periodic_timer;                //initialize timer
  nullnet_set_input_callback(input_callback);         //set callback
  etimer_set(&periodic_timer, EXPECTED_REC_INTERVAL); //set timer

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    //check for received message 
    if (received) {
      failCount = 0;
    }
    else {
      printf("Did not receive expected message. Incrementing count\n");
      failCount++;
    }

    //change channel if too many failed messages (result of succesfull jamming) 
    if (failCount >= 5){
      printf("Too many non-received messages. Swapping channel");
      startChannel++;                                                 //try the next channel 
      
      //try to change channel 
      if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startChannel) != RADIO_RESULT_OK) {
        printf("Issue changing channel");
      }
      else {
        printf("\n Succesfully changed channel to %d", startChannel);
        failCount = 0;                                                  //reset
        nullnet_set_input_callback(input_callback);                     //trying to reinstantiate

      }
    }

    //reset timer 
    etimer_reset(&periodic_timer);
  }

  printf("Ready to receive\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
