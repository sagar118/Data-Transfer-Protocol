#include "../include/simulator.h"
#include <stdlib.h>
#include <stdio.h>

#define DEF_ACK 111
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

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
struct pkt *main_packets;

struct LL_messages{
  struct msg message;
  struct LL_messages *next;
};

struct LL_messages *starting_message = NULL;
struct LL_messages *ending_message = NULL;

struct LL_messages *get_message();
void add_message(struct msg *temp_message);
int checksum(struct pkt *temp_packet);

int send_to_B();
int buffer_size = 0;
int number_of_packets_in_window = 0;

int current_position_in_window = 0;
int window_start = 0;
int max_buffer_size = 1000;

int sequence_number_A = 0;
int B_window_sequence_number = 0;
int winsize;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
struct msg message;{
    struct LL_messages *to_be_returned = malloc(sizeof(struct LL_messages)); 
    add_message(&message);
    // printf("win size %i\n", getwinsize());
    if (!(buffer_size == 0 || number_of_packets_in_window == getwinsize())){
        to_be_returned = get_message();
        for(int i=0; i < 20; i++){
            main_packets[current_position_in_window].payload[i] = to_be_returned->message.data[i];
        }

        main_packets[current_position_in_window].acknum = sequence_number_A;
        main_packets[current_position_in_window].seqnum = sequence_number_A;
        main_packets[current_position_in_window].checksum = checksum(&main_packets[current_position_in_window\
        ]);
        // printf("sending to layer 3 from send_to_B");
        number_of_packets_in_window ++;
        tolayer3(A_marker,main_packets[current_position_in_window]);
        current_position_in_window = (current_position_in_window + 1) % getwinsize();
        // printf("sequence_number_A %i\n", sequence_number_A);
        sequence_number_A ++;
        // printf("number_of_packets_in_window %i\n",number_of_packets_in_window);
        if (window_start == current_position_in_window-1 ){
          // printf("Started Time\n");
            starttimer(A_marker, 30.0);
        }
    }
    // if the window is already full, dont send anything.
    
    
    // return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    int res = 0;
    // struct LL_messages *to_be_returned = malloc(sizeof(struct LL_messages));
    if (!(main_packets[window_start].seqnum != packet.acknum || checksum(&packet) != packet.checksum)){
        stoptimer(A_marker);
        number_of_packets_in_window--;
        main_packets[window_start].seqnum = -1;
        if (number_of_packets_in_window==0){
            window_start=0;
            current_position_in_window = 0;
            struct LL_messages *to_be_returned = malloc(sizeof(struct LL_messages));
        
            // Without this line multiple attempts will be made to start the timer that has already been started.
            // Also works as a sanity check.
            // if the window is already full, dont send anything.
            if (!(buffer_size == 0 || number_of_packets_in_window == getwinsize())){
                to_be_returned = get_message();
                for(int i=0; i < 20; i++){
                    main_packets[current_position_in_window].payload[i] = to_be_returned->message.data[i];
                }

                main_packets[current_position_in_window].acknum = sequence_number_A;
                main_packets[current_position_in_window].seqnum = sequence_number_A;
                main_packets[current_position_in_window].checksum = checksum(&main_packets[current_position_in_window\
                ]);
                // printf("sending to layer 3 from send_to_B");
                number_of_packets_in_window ++;
                tolayer3(A_marker,main_packets[current_position_in_window]);
                current_position_in_window = (current_position_in_window + 1) % getwinsize();
                // printf("sequence_number_A %i\n", sequence_number_A);
                sequence_number_A ++;
                    // if (res == 1){
                starttimer(A_marker,30.0);
                    // }  
            }   
        }
        else{
            window_start = (window_start+1)%getwinsize();
            struct LL_messages *to_be_returned = malloc(sizeof(struct LL_messages));
        
            // Without this line multiple attempts will be made to start the timer that has already been started.
            // Also works as a sanity check.
            if (!(buffer_size == 0 || number_of_packets_in_window == getwinsize())){
                to_be_returned = get_message();
                for(int i=0; i < 20; i++){
                    main_packets[current_position_in_window].payload[i] = to_be_returned->message.data[i];
                }

                main_packets[current_position_in_window].acknum = sequence_number_A;
                main_packets[current_position_in_window].seqnum = sequence_number_A;
                main_packets[current_position_in_window].checksum = checksum(&main_packets[current_position_in_window\
                ]);
                // printf("sending to layer 3 from send_to_B");
                number_of_packets_in_window ++;
                tolayer3(A_marker,main_packets[current_position_in_window]);
                current_position_in_window = (current_position_in_window + 1) % getwinsize();
                // printf("sequence_number_A %i\n", sequence_number_A);
                sequence_number_A ++;
            }
        }
        if ((window_start != current_position_in_window)||(number_of_packets_in_window==1)){
            starttimer(A_marker, 30.0);
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(){
    int initial_position = window_start;
    int i = 0;
    // printf("number_of_packets_in_window %i/n", number_of_packets_in_window);
    while (i < number_of_packets_in_window){
        // printf("Looping");
        if (main_packets[initial_position].seqnum != -1){
          tolayer3(0, main_packets[initial_position]);
          break;
        }
        initial_position = (initial_position + 1) % getwinsize();
        i++;
    }
    if (window_start != current_position_in_window -1 || number_of_packets_in_window==1){
      starttimer(A_marker, 30.0);
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
    main_packets = malloc(sizeof(struct pkt) * getwinsize());
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;{
    
    if (packet.checksum != checksum(&packet)){
      return;
    }
    if (packet.seqnum == B_window_sequence_number){
        B_window_sequence_number ++;
        // printf("Sending to Layer 5 from B");
        tolayer5(B_marker, packet.payload);
    }
    else if (packet.seqnum < B_window_sequence_number){
      struct pkt tbr;
      tbr.acknum = packet.seqnum;
      tbr.seqnum = packet.seqnum;
      tbr.checksum = checksum(&tbr);
      // printf("Sending to Layer 3 from B");
      tolayer3(B_marker, tbr);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}

// Need to return a struct with the message
struct LL_messages *get_message(){
    struct LL_messages *to_be_returned = malloc(sizeof(struct LL_messages));
    
    if (buffer_size != 0){
        to_be_returned = starting_message;
        starting_message = starting_message->next;
        buffer_size --;
        if (buffer_size == 0){
            starting_message = NULL;
            ending_message = NULL;
        }
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
  
  if ((temp_message != NULL) && (buffer_size < max_buffer_size)){
    // temp_packet->next = NULL;
    for(int i=0; i< 20; i++){
      temp_packet->message.data[i] = temp_message->data[i];
    }

    if(starting_message == NULL){
      starting_message = temp_packet;
      ending_message = temp_packet;
    }
    else{
      ending_message->next = temp_packet;
      ending_message = ending_message->next;
    }
    buffer_size ++;
  }
}
