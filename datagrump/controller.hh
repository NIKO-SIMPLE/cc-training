#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

/* 拥塞控制器接口 */

class Controller
{
private:
  bool debug_; /* 是否启用调试输出 */

  double congestion_window_;
  double slow_start_threshold_;

  bool has_rtt_sample_;
  double estimated_rtt_ms_;
  double deviation_rtt_ms_;
  unsigned int retransmission_timeout_ms_;

  bool has_ack_;
  uint64_t highest_sequence_acked_;

  void congestion_detected();

public:
  /* 拥塞控制器的公共接口 */
  /* 可根据需要修改这些接口，但同时需要修改
     sender.cc 中的调用位置 */

  /* 默认构造函数 */
  Controller( const bool debug );

  /* 获取当前窗口大小，单位为数据报 */
  unsigned int window_size();

  /* 数据报已发送 */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp,
			  const bool after_timeout );

  /* 收到一个 ACK */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* 未收到 ACK 时，发送下一个数据报前的等待时间（毫秒） */
  unsigned int timeout_ms();
};

#endif
