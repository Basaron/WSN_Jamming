
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */



//static linkaddr_t dest_addr ={{ 0x98, 0xa0, 0x93, 0x1c, 0x00, 0x74, 0x12, 0x00 }};
/*---------------------------------------------------------------------------*/
PROCESS(client_comm, "client_comm");
AUTOSTART_PROCESSES(&client_comm);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    unsigned msg;
    memcpy(&msg, data, sizeof(msg));
    
    printf("The message resived is: %u \n", msg);

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
  
  nullnet_set_input_callback(input_callback);

  printf("Ready to recive\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
