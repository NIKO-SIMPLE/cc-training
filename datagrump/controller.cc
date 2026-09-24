#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

unsigned int WindowSize =50;
uint64_t last_ack = 0;  //上一个ack的序列号
bool has_last_ack = false; //上一次ack是否存在，防止0号报文被误认为是重复ack
/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = WindowSize;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << WindowSize << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }

  //判断是否超时
  if (after_timeout) {
    cout << "time out" << endl;
    WindowSize /= 2; // 超时后窗口大小减半
    if (WindowSize < 1) {
      WindowSize = 1; // 窗口大小不能小于1
    }
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
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
  //cout<<"num_acked:"<<sequence_number_acked<<endl;
  if (has_last_ack && sequence_number_acked == last_ack) {
    cout<<"got same ack"<<sequence_number_acked<<endl;
  }
  last_ack = sequence_number_acked;
  has_last_ack = true;
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 1000; /* timeout of one second */
}
