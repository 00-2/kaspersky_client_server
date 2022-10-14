//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;



enum {
    NOT_LOGIN=0,
    LOGIN,
    TEXTING,
    SIGN_OUT
} statuses;


class session
{
public:
    session(boost::asio::io_service& io_service)
            : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            std::string copy = std::string(data_, bytes_transferred - 1);
            switch (this->status) {
                case NOT_LOGIN:
                    if (copy.compare("START")==0){//match
                        BOOST_LOG_TRIVIAL(info) << this->socket().remote_endpoint().address().to_string() << "- Login";
                    }
                    else {
                        BOOST_LOG_TRIVIAL(warning) << this->socket().remote_endpoint().address().to_string() << "- Try to login, wrong message received:" << copy;
                    }
                    this->status = LOGIN;
                    break;
                case LOGIN:
                    if (copy.compare("STOP")==0) {//match
                        BOOST_LOG_TRIVIAL(info) << this->socket().remote_endpoint().address().to_string() << "- SIGNOUT";
                        this->status = SIGN_OUT;
                        break;
                    }
                    BOOST_LOG_TRIVIAL(info) << this->socket().remote_endpoint().address().to_string() << "- message received:"<<copy;
            }

            //response
            boost::asio::async_write(socket_,
         boost::asio::buffer(data_, bytes_transferred),
         boost::bind(&session::handle_write, this,
                     boost::asio::placeholders::error));

        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    int status = NOT_LOGIN;
};

class server
{
public:
    server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service),
              acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                                   boost::bind(&server::handle_accept, this, new_session,
                                               boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (std::getenv("SERVER_PORT")==NULL){
            std::cerr << "Error: SERVER_PORT not set";
            exit(1);
        }
        std::string SERVER_PORT = std::getenv("SERVER_PORT");
        std::cout << "Try to start server, port:" << SERVER_PORT << std::endl ;
        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        server s(io_service, atoi(SERVER_PORT.c_str()));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        std::cout << "To change port: ./setup.sh" << std::endl ;
    }

    return 0;
}