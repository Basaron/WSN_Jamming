
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */
bool recieved = false;
int msgCount = 0;
int failcount = 0;
int startchannel = 26;
#define EXPECTED_REC_INTERVAL (8 * CLOCK_SECOND)

//static linkaddr_t dest_addr ={{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
/*---------------------------------------------------------------------------*/
PROCESS(client_comm, "client_comm");
AUTOSTART_PROCESSES(&client_comm);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    recieved = true;
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));
    
    printf("The message resived is: %u \n", msg);
    msgCount ++;

    static int ackmsg = 1;
    nullnet_buf = (uint8_t *)&ackmsg;
    nullnet_len = sizeof(ackmsg);
    NETSTACK_NETWORK.output(src);
  }

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(client_comm, ev, data)
{
  PROCESS_BEGIN();  
  static struct etimer periodic_timer;

  nullnet_set_input_callback(input_callback);
  etimer_set(&periodic_timer, EXPECTED_REC_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    if (recieved) {
      failcount = 0;
    }
    else {
      printf("Didnt recieve expected message. Incrementing count\n");
      failcount++;
    }
    if (failcount >= 5){
      printf("Too many non-recieved messages. Swapping channel");
      startchannel++;
      if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startchannel) != RADIO_RESULT_OK){
        printf("issue changing channel");
      }
      else {
        printf("\n Succesfully changed channel to %d",startchannel);
        failcount = 0; //reset
        nullnet_set_input_callback(input_callback); //trying to reinstantiate

      }
    }
    etimer_reset(&periodic_timer);
  }


  printf("Ready to recive\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
