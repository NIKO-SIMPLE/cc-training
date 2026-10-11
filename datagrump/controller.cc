#include <algorithm>
#include <cmath>
#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* 默认构造函数 */
Controller::Controller( const bool debug )
  : debug_( debug ),
    congestion_window_( 1.0 ),
    slow_start_threshold_( 16.0 ),
    has_rtt_sample_( false ),
    estimated_rtt_ms_( 0.0 ),
    deviation_rtt_ms_( 0.0 ),
    retransmission_timeout_ms_( 1000 ),
    has_ack_( false ),
    highest_sequence_acked_( 0 )
{}

void Controller::congestion_detected()
{
  slow_start_threshold_ = max( 2.0, congestion_window_ / 2.0 );
  congestion_window_ = 1.0;
}


/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* 拥塞避免阶段在内部保留窗口的小数部分。 */
  const unsigned int the_window_size
    = max( 1U, static_cast<unsigned int>( congestion_window_ ) );

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << the_window_size << endl;
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
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }

  if ( after_timeout ) {
    congestion_detected();
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
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
  /* 使用发送方时钟测量 RTT。忽略异常时间戳，
     避免无符号整数减法发生回绕。 */
  if ( timestamp_ack_received >= send_timestamp_acked ) {
    const double sample_rtt = timestamp_ack_received - send_timestamp_acked;

    if ( not has_rtt_sample_ ) {
      estimated_rtt_ms_ = sample_rtt;
      deviation_rtt_ms_ = sample_rtt / 2.0;
      has_rtt_sample_ = true;
    } else {
      estimated_rtt_ms_ = 0.875 * estimated_rtt_ms_ + 0.125 * sample_rtt;
      deviation_rtt_ms_ = 0.75 * deviation_rtt_ms_
                        + 0.25 * abs( sample_rtt - estimated_rtt_ms_ );
    }

    const double rto = estimated_rtt_ms_ + 4.0 * deviation_rtt_ms_;
    /* 上下界可避免零毫秒忙循环和无限期停顿。 */
    retransmission_timeout_ms_
      = static_cast<unsigned int>( ceil( min( 60000.0, max( 10.0, rto ) ) ) );
  }

  /* 跳过尚未确认序号的 ACK 是该接口可提供的丢包信号。
     只有更大的新 ACK 才能报告新缺口，因此延迟或乱序的旧 ACK
     不会导致拥塞窗口再次减半。 */
  const bool packet_loss = ( not has_ack_ && sequence_number_acked > 0 )
                        || ( has_ack_
                             && sequence_number_acked > highest_sequence_acked_ + 1 );

  if ( not has_ack_ || sequence_number_acked > highest_sequence_acked_ ) {
    highest_sequence_acked_ = sequence_number_acked;
    has_ack_ = true;
  }

  if ( packet_loss ) {
    congestion_detected();
  } else if ( congestion_window_ < slow_start_threshold_ ) {
    /* 慢启动：每收到一个 ACK 增加一个数据报（每个 RTT 约翻倍）。 */
    congestion_window_ += 1.0;
  } else {
    /* 拥塞避免：每个 ACK 增加 1/cwnd（每个 RTT 约增加 1）。 */
    congestion_window_ += 1.0 / congestion_window_;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return retransmission_timeout_ms_;
}
