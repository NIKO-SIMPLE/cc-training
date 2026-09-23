/* simple UDP receiver that acknowledges every datagram */

#include <cstdlib>
#include <iostream>

#include "socket.hh"
#include "contest_message.hh"

using namespace std;

int main( int argc, char *argv[] )
{
   /* check the command-line arguments */
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  if ( argc != 2 ) {
    cerr << "Usage: " << argv[ 0 ] << " PORT" << endl;
    return EXIT_FAILURE;
  }

  /* create UDP socket for incoming datagrams */
  UDPSocket socket;

  /* turn on timestamps on receipt */
  socket.set_timestamps();

  /* "bind" the socket to the user-specified local port number */
  socket.bind( Address( "::0", argv[ 1 ] ) );

  cerr << "Listening on " << socket.local_address().to_string() << endl;

  uint64_t sequence_number = 0;
  uint64_t packets_have_been_received = 0;
  const uint64_t delay_ack_buffer = 2;
  /* Loop and acknowledge every incoming datagram back to its source */
  while ( true ) {
    const UDPSocket::received_datagram recd = socket.recv();
    ContestMessage message = recd.payload;
    //cout<< "Before transform:" << static_cast<unsigned int>(message.header.type) << endl;
    
    //每接收到一个数据报文，计数器++
    ++packets_have_been_received;

    //如果等于delay_ack_buffer，则发送ack
    if(packets_have_been_received == delay_ack_buffer){
      //重置缓冲区大小
      packets_have_been_received = 0;
      //发送返回报文
      message.transform_into_ack( sequence_number++, recd.timestamp );
      message.set_send_timestamp(); 
      socket.sendto( recd.source_address, message.to_string() );
    }
  }

  return EXIT_SUCCESS;
}
