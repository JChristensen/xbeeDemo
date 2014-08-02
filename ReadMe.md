### XBee Mesh Networking demo for Flint Area Coding Meetup ###

This work by Jack Christensen is licensed under CC BY-SA 4.0, //http://creativecommons.org/licenses/by-sa/4.0/

This demo consists of two microcontrollers that communicate via XBee ZB modules. Two button switches and a potentiometer on the sending unit control two LEDs and a servo on the receiving unit.

Note that the same code is used for the sender and receiver. If Arduino digital pin 7 is grounded, the microcontroller will act as the receiver. If it is left unconnected, the microcontroller will act as the transmitter.