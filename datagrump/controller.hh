#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <queue>

/* Congestion controller interface */

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */

  struct sent_packet_info_ {
    uint64_t seqno;
    uint64_t sent_time;

    sent_packet_info_(uint64_t seqno_, uint64_t sent_time_)
      : seqno(seqno_), sent_time(sent_time_) { }
  };

  std::queue<sent_packet_info_> packets_sent_;
  double the_window_size_;

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  double window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* A timeout occurred. */
  void adjust_window( void );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );
};

#endif
