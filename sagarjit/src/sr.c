#include "../include/simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

int const BUFFER_SIZE = 1000;
int const TIMEOUT = 20;
int MAX_WINDOW_SIZE;
int A_seq_num;
int B_seq_num;
int total_pkt_in_window = 0;
int window_start_ptr = 0;
int window_end_ptr = 0;
int buff_size = 0;
int timer_start = 0;
int win_size;
int B_win_ptr = 0;

struct buffer{
    struct msg message;
    struct buffer *next;
};

struct window_data{
	struct pkt payload_data;
	int timeout;
	int ack;
};

struct buffer *start = NULL;
struct buffer *end = NULL;
struct window_data *window;
struct window_data *B_window;

// Function to calculate the checksum
int calculate_checksum(struct pkt *packet){
	int checksum = 0;
	if (packet == NULL){
		return checksum;
	}

	for (int i=0; i<20; i++){
		checksum += packet->payload[i];
	}
	checksum += packet->seqnum;
	checksum += packet->acknum;

	// printf("Checksum: %d", checksum);
	return checksum;
}

// Function to add message from A to buffer
void add_message_to_buffer(struct msg message){
	struct buffer *packet = malloc(sizeof(struct buffer));
	packet->message = message;
	packet->next = NULL;

	// printf("Buffer size: %d", buff_size);
	if (buff_size < BUFFER_SIZE){
		if (start == NULL){
			start = malloc(sizeof(struct buffer));
			end = malloc(sizeof(struct buffer));
			start = packet;
			end = packet;
		}
		else{
			end->next = malloc(sizeof(struct buffer));
			end->next = packet;
			end = end->next;
		}
		buff_size ++;
	}
	return;
}

// Function to get message from buffer
struct buffer * get_packet_from_buffer(){
  struct buffer *packet = (struct buffer *) malloc(sizeof(struct buffer));
  if (buff_size != 0){
    packet = start;
	start = start->next;

    buff_size --;

    if (buff_size == 0){
      start = NULL;
      end = NULL;
    }
  }
  else{
	  return NULL;
  }
  return packet;
}

// Function to add packet to window
void add_packet_to_window(struct buffer *packet, int input){
	// print("Total packets in window: %d", total_pkt_in_window);
	// print("A sequence number: %d", A_seq_num);
	// print("Window start: %d and end %d", window_start_ptr, window_end_ptr);
	if (total_pkt_in_window != 0 && input != 1){
		window_end_ptr = (window_end_ptr + 1) % MAX_WINDOW_SIZE;
	}

	strcpy(window[window_end_ptr].payload_data.payload, packet->message.data);

	window[window_end_ptr].payload_data.seqnum = A_seq_num;
	window[window_end_ptr].payload_data.acknum = A_seq_num;
	window[window_end_ptr].payload_data.checksum = calculate_checksum(&window[window_end_ptr].payload_data);

	window[window_end_ptr].ack = 0;
	window[window_end_ptr].timeout = get_sim_time() + TIMEOUT;

	A_seq_num ++;
	total_pkt_in_window ++;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;{
	  add_message_to_buffer(message);

	  if (total_pkt_in_window == MAX_WINDOW_SIZE){
		  return;
	  }

	  if (buff_size == 0){
		  return;
	  }
	  
	  struct buffer *packet = malloc(sizeof(struct buffer));
	  packet = get_packet_from_buffer();

	  add_packet_to_window(packet, 0);

	  tolayer3(0, window[window_end_ptr].payload_data);
	  if (timer_start == 0){
		  timer_start = 1;
		  starttimer(0, TIMEOUT);
	  }
	  return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;{
	  if (&packet == NULL || packet.checksum != calculate_checksum(&packet)){
		  return;
	  }

	  // Received ack for the first packet in the window
	  if (packet.acknum == window[window_start_ptr].payload_data.seqnum){
		  window[window_start_ptr].ack = 1;
		  total_pkt_in_window --;

		// printf("Total packet in window: %d", total_pkt_in_window);
		// printf("Buffer size: %d", buff_size);
		  if (total_pkt_in_window == 0){
			  // Window is empty
			  window_start_ptr = (window_start_ptr + 1) % MAX_WINDOW_SIZE;
			  window_end_ptr = (window_end_ptr + 1) % MAX_WINDOW_SIZE;
			  
			  if (buff_size == 0){
				  timer_start = 0;
				  stoptimer(0);
			  }
			  else{
				  struct buffer *packet = malloc(sizeof(struct buffer));
			  	  packet = get_packet_from_buffer();

			  	  add_packet_to_window(packet, 1);
			  	  tolayer3(0, window[window_end_ptr].payload_data);
			  }
		  }
		  else{
			  // IF window is not empty then find for which packet 
			  // we received the ACK and make the ack as 1
			  int temp_start = window_start_ptr;

			  // printf("Has packet in window");
			  // printf("Total packet in window: %d", total_pkt_in_window);
			  while(true){ 
				  // For packets in between where we already got the ack
				  // Reduce the total packet in window count
				  if (temp_start == window_end_ptr || window[(temp_start + 1) % MAX_WINDOW_SIZE].ack == 0){
					  break;
				  }
				  total_pkt_in_window --;
				  temp_start = (temp_start + 1) % MAX_WINDOW_SIZE;
			  }

			  window_start_ptr = (temp_start + 1) % MAX_WINDOW_SIZE;
			
			  if (total_pkt_in_window == 0){ // After removal is window empty
				  window_end_ptr = window_start_ptr;
			  }

			  // If buffer not empty then get packet from buffer and send to layer 3
			  if (buff_size != 0){
				  struct buffer *packet = malloc(sizeof(struct buffer));
			  	  packet = get_packet_from_buffer();

			  	  add_packet_to_window(packet, 1);
			  	  tolayer3(0, window[window_end_ptr].payload_data);
			  }
		  }
	  }
	  else if (packet.acknum > window[window_start_ptr].payload_data.seqnum){
		  // ACK received for packet in between the window
		  int temp_start = window_start_ptr;
		  while (true){
			  if (packet.acknum == window[temp_start].payload_data.seqnum){
				  window[temp_start].ack = 1;
				  break;
			  }
			  if (temp_start == window_end_ptr){
				  break;
			  }
			  temp_start = (temp_start + 1) % MAX_WINDOW_SIZE;
		  }
	  }
}

/* called when A's timer goes off */
void A_timerinterrupt(){	
	int index = window_start_ptr;
	if (total_pkt_in_window == 0){
		starttimer(0, TIMEOUT);
		return;
	}

	while(true){
		// printf("Inside while");
		if (index == window_end_ptr){
			break;
		}
		// For packets we did not receive ack within timeout, resend them
		if (window[index].ack == 0 && window[index].timeout < get_sim_time()){
			window[index].timeout = get_sim_time() + TIMEOUT;
			tolayer3(0, window[index].payload_data);
		}
		// tolayer3(0, window[index].payload_data);
		index = (index + 1) % MAX_WINDOW_SIZE;
	}
	if (window[index].ack == 0 && window[index].timeout < get_sim_time()){
		window[index].timeout = get_sim_time() + TIMEOUT;
		tolayer3(0, window[index].payload_data);
	}
	starttimer(0, TIMEOUT);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
	A_seq_num = 0;
	MAX_WINDOW_SIZE = getwinsize();
	window = malloc(sizeof(struct window_data) * MAX_WINDOW_SIZE);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

// Struct to define the response from Host B
struct pkt get_response(struct pkt res){
	// printf("Insider Response");
	struct pkt response;
	response.seqnum = res.seqnum;
	response.acknum = res.seqnum;
	response.checksum = calculate_checksum(&response);
	return response;
}

// Function to send packets to layer5 from Host B
// for which ack's were already received
void send_remaining_packets(){
	B_window[B_win_ptr].ack=(B_seq_num)+win_size-1;
	B_win_ptr = (B_win_ptr + 1) % win_size;

	while(B_window[B_win_ptr].payload_data.seqnum == B_seq_num){
		// printf("Data send to layer5");
		tolayer5(1, B_window[B_win_ptr].payload_data.payload);
		B_seq_num ++;
		B_window[B_win_ptr].ack=(B_seq_num)+win_size-1;
		B_win_ptr = (B_win_ptr + 1) % win_size;
	}
}

// Function to save out of order packets received at Host B.
void save_future_packet(struct pkt packet){
	if (packet.seqnum <= B_seq_num+win_size){
		for (int j=0; j<win_size; j++){
			if (B_window[j].ack == packet.seqnum){
				B_window[j].payload_data = packet;

				struct pkt response = get_response(packet);

				// printf("Data send to layer3, future packet ack");
				tolayer3(1, response);
				break;
			}	
		}
	}
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;{
	  if (&packet == NULL || packet.checksum != calculate_checksum(&packet)){
		  return;
	  }

	  // Received packet in order
	  if (packet.seqnum == B_seq_num){
		  tolayer5(1, packet.payload);
		  B_seq_num ++;

		  struct pkt response = get_response(packet);
		  // printf("Data send to layer3 in order");
		  tolayer3(1, response);

		  send_remaining_packets();
	  }
	  else{
		  // Received future packet
		  if (packet.seqnum > B_seq_num){
			  save_future_packet(packet);
		  }
		  else{
			  // Only send ACK for old packet
			  struct pkt response = get_response(packet);
			  // printf("Data send to layer3 old packets");
			  tolayer3(1, response);
		  }
	  }  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	B_seq_num = 0;
	win_size = getwinsize();
	B_window  = malloc(sizeof(struct window_data) * win_size);
}
