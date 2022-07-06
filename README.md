# Data Transfer Protocol

## Overview
In this programming assignment, we have written the sending and receiving transport-layer code for implementing a simple reliable data transfer protocol. There are 3 versions of this assignment,
- Alternating-Bit Protocol version
- Go-Back-N version
- Selective-Repeat version.

Since we don't have standalone machines (with an OS that you can modify), your code will have to execute in a simulated hardware/software environment. However, the programming interface provided to your routines, i.e., the code that would call your entities from above and from below is very close to what is done in an actual UNIX environment. Stopping/starting of timers is also simulated, and timer interrupts will cause your timer handling routine to be activated.

## Overall flow of the project
The procedures sends message from entity (A) and entity (B) receives the data. Only unidirectional transfer of data (from A to B) is required. Of course, the B side will have to send packets to A to acknowledge receipt of data. Our routines are to be implemented in the form of the procedures described below. These procedures will be called by (and will call) procedures that simulate a network environment. The overall structure of the environment is shown below:

<p align="center">
<img width="676" alt="image" src="https://user-images.githubusercontent.com/24275587/177456197-1297cc7b-2a7b-4cee-8493-a7b435d6f43a.png">
</p>

The unit of data passed between the upper layers and your protocols is a message. Routines that were implemented:

1. **A_output(message)**<br>
Where the message is a structure of type msg, containing data to be sent to the B-side. This routine will be called whenever the upper layer at the sending side (A) has a message to send. It is the job of your protocol to ensure that the data in such a message is delivered in order, and correctly, to the receiving side upper layer.

2. **A_input(packet)**<br>
Where the packet is a structure of type pkt. This routine will be called whenever a packet sent from the B-side (as a result of a tolayer3() (see section 3.5) being called by a B-side procedure) arrives at the A-side. packet is the (possibly corrupted) packet sent from the B-side.

3. **A_timerinterrupt()**<br>
This routine will be called when A's timer expires (thus generating a timer interrupt). You'll probably want to use this routine to control the retransmission of packets. See starttimer() and stoptimer() below for how the timer is started and stopped.

4. **A_init()**<br>
This routine will be called once, before any of your other A-side routines are called. It can be used to do any required initialization.

5. **B_input(packet)**<br>
Where the packet is a structure of type pkt. This routine will be called whenever a packet sent from the A-side (as a result of a tolayer3() (see section 3.5) being called by a A-side procedure) arrives at the B-side. packet is the (possibly corrupted) packet sent from the A-side.

6. **B_init()**<br>
This routine will be called once, before any of your other B-side routines are called. It can be used to do any required initialization.

## Collaborator:
- Pushkaraj Joshi
- Sagar Thacker
