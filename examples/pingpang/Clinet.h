#ifndef CLIENT_H
#define CLIENT_H


#include <string>
#include "uv/uv11.h"
#include "ModeDefine.h"

class Client : public uv::TcpClient
{
public:
    Client(uv::EventLoop* loop)
        :TcpClient(loop),
        sockAddr(nullptr)
    {
        setConnectStatusCallback(std::bind(&Client::onConnect,this,std::placeholders::_1));
        setMessageCallback(std::bind(&Client::newMessage,this,std::placeholders::_1,std::placeholders::_2));
    }

    void connectToServer(uv::SocketAddr& addr)
    {
        sockAddr = std::make_shared<uv::SocketAddr>(addr);
        connect(addr);
    }

    void reConnect()
    {
        uv::Timer<void*>* timer;
        timer = new uv::Timer<void*>(Loop(), 1000, 0, [this](uv::Timer<void*>* timer, void*)
        {
            connect(*(sockAddr.get()));
            timer->close([](uv::Timer<void*>* handle)
            {
                delete handle;
            });
        }, nullptr);
        timer->start();
    }

    void onConnect(TcpClient::ConnectStatus status)
    {
        if(status != TcpClient::ConnectStatus::OnConnectSuccess)
        {
            reConnect();
        }
        else
        {
#if     USED_NO_PACKET
            write(data,1024);
#else
            uv::Packet packet;
            packet.fill(data,1024);

            write(packet.Buffer(), packet.BufferSize(), nullptr);
#endif
        }
    }

    void newMessage(const char* buf,unsigned int size)
    {
#if     USED_NO_PACKET
        write(data,1024);
#else
        uv::Packet packet;
        appendToBuffer(buf, size);
        while (0 == readFromBuffer(packet))
        {
            write(packet.Buffer(), packet.BufferSize(), nullptr);
        }
#endif
    }

private:
    std::shared_ptr<uv::SocketAddr> sockAddr;
    char data[10000];
};
#endif
