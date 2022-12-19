# Jamming and Anti-jamming in Wireless Sensor Networks 

- Copy code into contiki-ng
- Insert three motes 
- Cd into the code and run these makes:

  make TARGET=sky MOTES=/dev/ttyUSB0 server_comm.upload login
  make TARGET=sky MOTES=/dev/ttyUSB1 client_comm.upload login
Here xxx can be change to witch type of jammer is desired (constant/random/reactive)
  make TARGET=sky MOTES=/dev/ttyUSB3 xxx_jammer.upload login


