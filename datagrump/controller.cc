#include <iostream>
#include <deque>
#include <chrono>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), bw_filter_(), rtt_filter_(), delivered_( 0 ), packets_(), sequence_number_(0)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  double bdp = 1.5 * get_rtt() * get_bw();
  // cerr << "At time " << timestamp_ms() << " window size is " << bdp << endl;
  return bdp;
  /*
  unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
  */
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */
  packets_[sequence_number].delivered = delivered_;

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
			       const uint64_t timestamp_ack_received,
             /* when the ack was received (by sender) */
             const uint64_t sequence_number )
{
  delivered_++;
  delivered_ += sequence_number * 0;

  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;
  update_rtt(rtt);
  double delivery_rate = double(delivered_ - packets_[sequence_number_acked].delivered) / rtt;
  update_bw(delivery_rate, sequence_number_acked);

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

double Controller::get_bw( void )
{
  return bw_filter_.empty() ? .15 : bw_filter_.front().bw;
}

void Controller::update_bw( const double new_bw, const uint64_t seqno )
{
  // Keep 10 packets.
  while (!bw_filter_.empty() && bw_filter_.front().seqno < seqno - 10) {
    bw_filter_.pop_front();
  }

  while (!bw_filter_.empty() && bw_filter_.back().bw < new_bw) {
    bw_filter_.pop_back();
  }

  bw_filter_.push_back( bw_sample_( seqno, new_bw ) );

  /*
  cerr << "new bw = " << new_bw << ", windowed = " << get_bw()
       << ", deque size = " << bw_filter_.size() << endl;
       */
}

uint64_t Controller::get_rtt( void )
{
  tp now = std::chrono::system_clock::now();
  while (!rtt_filter_.empty() && now - rtt_filter_.front().time > std::chrono::seconds(10)) {
    rtt_filter_.pop_front();
  }

  return rtt_filter_.empty() ? 100 : rtt_filter_.front().rtt;
}

void Controller::update_rtt( const uint64_t new_rtt )
{
  while (!rtt_filter_.empty() && rtt_filter_.back().rtt > new_rtt) {
    rtt_filter_.pop_back();
  }

  /*
  cerr << "new rtt = " << new_rtt << ", windowed = " << get_rtt()
       << ", deque size = " << rtt_filter_.size() << endl;
       */

  tp now = std::chrono::system_clock::now();
  rtt_filter_.push_back(rtt_sample_( now, new_rtt ));
}
