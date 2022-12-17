
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <os/dev/radio.h>

#include <string.h>
#include <stdio.h> /* For printf() */

// state message variables
bool recieved = false;
int msgCount = 0;
int failcount = 0;
int startchannel = 11;

#define EXPECTED_REC_INTERVAL (4 * CLOCK_SECOND)

// static linkaddr_t dest_addr ={{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
/*---------------------------------------------------------------------------*/
PROCESS(client_comm, "client_comm");
AUTOSTART_PROCESSES(&client_comm);

/*--------------------------CALLBACK TO RECEIVE MESSAGES-------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
                    const linkaddr_t *src, const linkaddr_t *dest)
{
  // only receive messages that are unsigned i.e non-negative
  if (len == sizeof(unsigned))
  {
    recieved = true;
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));

    // incrementing message count
    printf("The message resived is: %u \n", msg);
    msgCount++;

    // acknoledgement messages
    static int ackmsg = 1;
    nullnet_buf = (uint8_t *)&ackmsg;
    nullnet_len = sizeof(ackmsg);

    // send to stack
    NETSTACK_NETWORK.output(src);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(client_comm, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;                // initialize timer
  nullnet_set_input_callback(input_callback);         // set callback
  etimer_set(&periodic_timer, EXPECTED_REC_INTERVAL); // set timer

  NETSTACK_RADIO.on();
  // NETSTACK_RADIO.set_value(RADIO_CONST_TXPOWER_MAX, 10);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startchannel);

  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    // check for received message
    if (recieved)
    {
      failcount = 0;
    }
    else
    {
      printf("Didnt recieve expected message. Incrementing count \n");
      //failcount++;
    }
    //change channel if too many failed messages (result of succesfull jamming) 

    // change channel if too many failed messages (result of succesfull jamming)
    if (failcount >= 5)
    {
      printf("Too many non-recieved messages. Swapping channel \n");
      startchannel++;

      //try to change channel 
      if (startchannel > 26)
      {
        startchannel = 11;
      }

      // try to change channel
      if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startchannel) != RADIO_RESULT_OK)
      {
        printf("issue changing channel \n");
      }
      else
      {
        printf("\n Succesfully changed channel to %d \n", startchannel);
        failcount = 0;                              // reset
        nullnet_set_input_callback(input_callback); // trying to reinstantiate
      }
    }
    recieved = false;

    // reset timer
    etimer_reset(&periodic_timer);
  }

  printf("Ready to recive \n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
