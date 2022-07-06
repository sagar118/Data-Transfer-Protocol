#include "../include/simulator.h"
#include "stdio.h"
#include "stdlib.h"

#define BUFFER_MSG 1000
#define A_marker 0
#define B_marker 1

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
struct pkt main_packet;
int sequence_number_A = 0;
int sequence_number_B = 0;

// we will declare the functions once
struct LL_messages *get_message();
void add_message(struct msg *temp_message);
int checksum(struct pkt *temp_packet);
// Lets create a buffer to store the messages 
// This will essentially be a linked list of all messages
struct LL_messages{
  struct msg message;
  struct LL_messages *next;
};

struct LL_messages *starting_message = NULL;
struct LL_messages *ending_message = NULL;

// Lets set some state variables:
int sender_state = 0;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  add_message(&message);
  struct LL_messages *message_tbs_LL;
  
  if (sender_state != 0){
    return;
  }

  // means that we have to wait for the ack from B
  sender_state = 1;

  message_tbs_LL = get_message();
  
  if (message_tbs_LL == NULL){
    return;
  }
  
  for (int i = 0; i< 20; i++){
    main_packet.payload[i] = message_tbs_LL->message.data[i];
  }
  free(message_tbs_LL);
  
  main_packet.seqnum = sequence_number_A;
  main_packet.acknum = sequence_number_A;
  main_packet.checksum = checksum(&main_packet);
  
  tolayer3(A_marker, main_packet);
  starttimer(A_marker, 20.0);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  if (packet.checksum != checksum(&packet)){
    return;
  }
  if (packet.acknum != sequence_number_A){
    return;
  }
  stoptimer(A_marker);
  sequence_number_A = (sequence_number_A + 1) % 2;
  sender_state = 0;
  printf("1. Sending Data to layer3");
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  // if (sender_state == 0){
  stoptimer(A_marker);
  tolayer3(A_marker, main_packet);
  starttimer(A_marker, 20.0);
  // }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  if (packet.checksum != checksum(&packet)){
    return;
  }
  if (packet.seqnum != sequence_number_B){
    return;
  }
  
  tolayer5(B_marker, packet.payload);
  struct pkt tbr_from_b;
  tbr_from_b.acknum = sequence_number_B;
  tbr_from_b.seqnum = sequence_number_B;
  tbr_from_b.checksum = checksum(&tbr_from_b);
  tolayer3(B_marker, tbr_from_b);
  sequence_number_B = (sequence_number_B + 1) % 2;

  // printf("1. Sending Data to layer5 from B");
  
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}

// Need to return a struct with the message
struct LL_messages *get_message()
{
  struct LL_messages *to_be_returned;
  if (starting_message == NULL)
  {
    return NULL;
  }
  
  to_be_returned = starting_message;
  
  starting_message = to_be_returned->next;
  
  if (starting_message == NULL)
  {
    ending_message = NULL;
  }

  return to_be_returned;
}

int checksum(struct pkt *temp_packet){
  int checksum_counter = 0;
  
  if (temp_packet == NULL){
    return 0;
  }
  
  for (int i=0; i<20;i++){
    checksum_counter += temp_packet->payload[i];
  }
  
  checksum_counter += temp_packet->seqnum;
  checksum_counter += temp_packet->acknum;
  return checksum_counter;
}

void add_message(struct msg *temp_message){
  struct LL_messages *temp_packet = malloc(sizeof(struct LL_messages));
  if (temp_message != NULL){
    temp_packet->next = NULL;
    for(int i=0; i< 20; i++){
      temp_packet->message.data[i] = temp_message->data[i];
    }

    if(ending_message == NULL){
      starting_message = temp_packet;
      ending_message = temp_packet;
    }
    else{
      ending_message->next = temp_packet;
      ending_message = ending_message->next;
    }
    // starting_message -> next = temp_packet;
    
  }
}