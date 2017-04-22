#include <iostream>
#include <queue>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), packets_sent_(), the_window_size_(10)
{ }

/* Get current window size, in datagrams */
double Controller::window_size( void )
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size_ << endl;
  }

  return the_window_size_;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp
            /* in milliseconds */)
{
  packets_sent_.push(sent_packet_info_(sequence_number, send_timestamp));
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
  bool timeout = true;
  while (!packets_sent_.empty() &&
         packets_sent_.front().seqno <= sequence_number_acked) {
    timeout = false;
    packets_sent_.pop();
  }

  if (!timeout) {
    the_window_size_ += 1.0 / the_window_size_;
    if ( debug_ ) cerr << "Ack; window size = " << the_window_size_ << endl;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }

  /*
  cerr << "RTT = " << timestamp_ack_received - send_timestamp_acked
       << ", window size = " << the_window_size_ << endl;
       */
}

void Controller::adjust_window( void )
{
  bool timeout = false;
  uint64_t now = timestamp_ms();
  while (!packets_sent_.empty() &&
         packets_sent_.front().sent_time < now - timeout_ms()) {
    timeout = true;
    packets_sent_.pop();
  }
  /*
  cerr << "Timeout at time " << timestamp_ms()
       << " window size is " << the_window_size_ << endl;
       */
  if (timeout) {
    the_window_size_ = max(1.0, the_window_size_ / 2.0);
    if ( debug_ ) cerr << "Timeout; window size = " << the_window_size_ << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 100;
}
