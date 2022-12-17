
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <os/dev/radio.h>

#include <string.h>
#include <stdio.h> /* For printf() */

// state message variables
bool ackresived = false;
int failMsgCount = 0;
int startchannel = 11;
int resivedMsgCount = 0;
int testLenge = 50;
#define sequence = {1, 5, 7, 9.13}:

/*-----------------------------CONFIGURATION-----------------------------*/
#define SEND_INTERVAL (4 * CLOCK_SECOND)

// static linkaddr_t dest_addr = {{0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00}};
//  static linkaddr_t dest_addr = {{ 0xd6, 0x73, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
// static linkaddr_t dest_addr = {{0x36, 0x29, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00}};
static linkaddr_t dest_addr = {{0xd4, 0xb1, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00}};

// MAC??
#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr = {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
PROCESS(server_comm, "Server_comm");
AUTOSTART_PROCESSES(&server_comm);

/*-------------------------------INPUT TO SERVER CALLBACK---------------------------------------*/
void input_callback(const void *data, uint16_t len,
                    const linkaddr_t *src, const linkaddr_t *dest)
{
  // only receive message of valid type
  if (len == sizeof(unsigned))
  {
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));
    if (msg == 1)
    {
      printf("The acknowledgement message resived \n");

      // update state
      resivedMsgCount++;
      ackresived = true;
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(server_comm, ev, data)
{

  static struct etimer periodic_timer; // initialize timer
  static int msgCount = 0;             // initialize message count
  static int i = 0;

  PROCESS_BEGIN();

  NETSTACK_RADIO.on();
  // NETSTACK_RADIO.set_value(RADIO_CONST_TXPOWER_MAX, 10);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, startchannel);

#if MAC_CONF_WITH_TSCH
  if (linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr))
  {
    printf("Server set as coordinator");
    tsch_set_coordinator(true);
  }
  else
  {
    printf("Server not set as coordinator");
  }
  // tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr)); //make coordinator if equal
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&msgCount;
  nullnet_len = sizeof(msgCount);
  nullnet_set_input_callback(input_callback);

  // If addresses not equal
  if (!linkaddr_cmp(&dest_addr, &linkaddr_node_addr))
  { // If not equal
    printf("LLAddress and DestAddr not equal. Proceeding..\n");
    etimer_set(&periodic_timer, SEND_INTERVAL);

    while (i <= testLenge)
    {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      // check if message is received
      if (ackresived)
      {
        failMsgCount = 0;
      }
      else
      {
        //failMsgCount++;
        printf("No acknowledgement message resived before timeout \n");
      }

      // if failmsgcount is above boundary, start channel hopping on given sequence
      if (failMsgCount >= 5)
      { // change to failmsg
        printf("Attempting to change channel \n");
        startchannel++; // change to next channel

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
          failMsgCount = 0;                           // reset msg count
          nullnet_set_input_callback(input_callback); // trying to reinstantiate
        }
      }
      // if failmsgcount is above boundary, start channel hopping on given sequence

      // transmitting message
      printf("sending %u to client \n", msgCount);
      NETSTACK_NETWORK.output(&dest_addr);
      msgCount++;
      ackresived = false;
      i++;

      // reset timer
      etimer_reset(&periodic_timer);
    }
  }

  printf("Number of message Send: %d \n", msgCount);
  printf("Number of message resived: %d \n", resivedMsgCount);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
