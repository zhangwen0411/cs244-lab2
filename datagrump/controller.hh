#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <deque>
#include <unordered_map>
#include <chrono>

/* Congestion controller interface */

class Controller
{
  typedef std::chrono::time_point<std::chrono::system_clock> tp;

private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  struct bw_sample_ {
    uint64_t seqno;
    double bw;

    bw_sample_( uint64_t seqno_, double bw_ ) : seqno( seqno_ ), bw( bw_ ) { }
  };
  std::deque<bw_sample_> bw_filter_;

  struct rtt_sample_ {
    tp time;
    uint64_t rtt;

    rtt_sample_( tp time_, uint64_t rtt_ ) : time( time_ ), rtt( rtt_ ) { }
  };
  std::deque<rtt_sample_> rtt_filter_;

  uint64_t delivered_;  // # packets.

  struct packet_ {
    uint64_t delivered;
  };
  std::unordered_map<uint64_t, packet_> packets_;  // seqno =>.

  double get_bw( void );
  void update_bw( const double new_bw, const uint64_t seqno );

  uint64_t get_rtt( void );
  void update_rtt( const uint64_t new_rtt );

  uint64_t sequence_number_;

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received,
         const uint64_t sequence_number );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );

  void increment_sequence_number( void ) { sequence_number_++; }
};

#endif
