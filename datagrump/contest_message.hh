#ifndef CONTEST_MESSAGE_HH
#define CONTEST_MESSAGE_HH

#include <string>
#include <cstdint>
enum class ContestMessageType : uint64_t{
  DATA = 0,
  ACK = 1
};
struct ContestMessage
{
  struct Header {
    ContestMessageType type; //消息类型
    uint64_t sequence_number; //序列号  
    uint64_t send_timestamp;  //发送时间

    uint64_t ack_sequence_number; //确认序列号
    uint64_t ack_send_timestamp;  //确认发送时间
    uint64_t ack_recv_timestamp;  //确认接收时间
    uint64_t ack_payload_length;  //确认载荷长度

    /* Header for new message */
    Header( const uint64_t s_sequence_number ); //发送端，从序列号构造消息头

    /* Parse header from wire */
    Header( const std::string & str ); //接收端，从字节流中解析消息头

    /* Make wire representation of header */
    std::string to_string() const; 
  } header;

  std::string payload; //消息体

  /* New message */
  ContestMessage( const uint64_t s_sequence_number,
		  const std::string & s_payload );

  /* Parse incoming datagram from wire */
  ContestMessage( const std::string & str );

  /* Fill in the send_timestamp for an outgoing datagram */
  void set_send_timestamp();

  /* Make wire representation of datagram */
  std::string to_string() const;

  /* Transform into an ack of the ContestMessage */
  void transform_into_ack( const uint64_t sequence_number,
			   const uint64_t recv_timestamp );

  /* Is this message an ack? */
  bool is_ack() const;
};

#endif /* CONTEST_MESSAGE_HH */
