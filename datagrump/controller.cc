#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), rate_( 1e-2 ), prev_rtt_( 0.0 ), rtt_diff_( 0.0 ),
    counter_( 0 ), neg_gradient_counter_( 0 )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  counter_++;
  uint64_t new_rtt = timestamp_ack_received - send_timestamp_acked;
  if (counter_ == 1) {
    prev_rtt_ = new_rtt;
    return;
  }
  
  uint64_t new_rtt_diff = new_rtt - prev_rtt_;
  prev_rtt_ = new_rtt;
  if (counter_ == 2) {
    rtt_diff_ = new_rtt_diff;
    return;
  }

  rtt_diff_ = (1 - alpha) * rtt_diff_ + alpha * new_rtt_diff;
  if (new_rtt < t_low) {
    rate_ += delta;
    return;
  }
  if (new_rtt > t_high) {
    rate *= (1 - beta * (1 - t_high / new_rtt));
    return;
  }

  if (rtt_diff_ < 0) { neg_gradient_counter_++; }
  else { neg_gradient_counter_ = 0; }

  if (rtt_diff_ <= 0) {
    int N = (neg_gradient_counter_ >= 5) ? 5 : 1;
    rate += N * delta;
  } else {
    rate *= (1 - beta * rtt_diff_ / 50.0);
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
