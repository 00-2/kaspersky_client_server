// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/thread/thread.hpp>
using boost::asio::ip::tcp;
boost::asio::io_service service;

boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string("127.0.0.1"), 33333);
size_t read_complete(char * buf, const boost::system::error_code & err, size_t bytes)
{
    if ( err) return 0;
    bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}
void sync_echo(size_t count)
{
//    std::cout << std::this_thread::get_id();
    std::string msg = "";
    boost::asio::ip::tcp::socket sock(service);
    sock.connect(ep);
    msg = "START\n";
    sock.write_some(boost::asio::buffer(msg));
    std::cout << "send data" << std::endl;
    char buf[1024];
    int bytes;
    std::string copy;
    bytes=read(sock, boost::asio::buffer(buf), boost::bind(read_complete,buf,_1,_2));
    copy = std::string(buf, bytes - 1);
    std::cout << "server echoed " << copy << std::endl;
    sleep(1);
    msg = "SOMESEXTEXT";
    for(;count>0; --count){
        sock.write_some(boost::asio::buffer(msg+std::to_string(count)+"\n"));
        bytes=read(sock, boost::asio::buffer(buf), boost::bind(read_complete,buf,_1,_2));
        copy = std::string(buf, bytes - 1);
        std::cout << "server echoed " << copy << std::endl;
    }
    sock.write_some(boost::asio::buffer("STOP"));
}
int main(int argc, char* argv[])
{
    size_t messages[] = { 1, 6, 7, 2, 1 , 0};
    boost::thread_group threads;
    for ( size_t * message = messages; *message; ++message)
    {
        threads.create_thread( boost::bind(sync_echo, *message));
        boost::this_thread::sleep( boost::posix_time::millisec(100));
    }
    threads.join_all();
}