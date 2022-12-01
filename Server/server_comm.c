
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */

bool ackresived = false;
int failMsgCount = 0;

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

//This adress {{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
static linkaddr_t dest_addr = {{ 0xd6, 0x73, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr = {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
PROCESS(server_comm, "Server_comm");
AUTOSTART_PROCESSES(&server_comm);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));
    
    printf("The acknowledgement message resived \n");

    ackresived = true;

  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(server_comm, ev, data)
{
  
  static struct etimer periodic_timer;
  static int msgCount = 0;
  PROCESS_BEGIN();

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

  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) { //If not equal
    printf("LLAddress and DestAddr not equal. Proceeding..");
    etimer_set(&periodic_timer, SEND_INTERVAL);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      if (ackresived == true)
      {
        failMsgCount = 0;

      }else
      {
        failMsgCount ++;
        printf("No acknowledgement message resived before timeout \n");
      }
      

      printf("sending %u to client \n", msgCount);
      NETSTACK_NETWORK.output(&dest_addr);
      msgCount++;

      ackresived = false;

      etimer_reset(&periodic_timer);
    }
  }
  printf("Dest and Current addr is equal");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
