
#include <iostream>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>
#include <iostream>

int main()
{
    ix::initNetSystem();

    ix::WebSocket webSocket;


    std::string url("localhost:9001");
    webSocket.setUrl(url);

    std::cout << "Connecting to " << url << "..." << std::endl;

    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
        {
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                std::cout << "received message: " << msg->str << std::endl;
                std::cout << "> " << std::flush;
            }
            else if (msg->type == ix::WebSocketMessageType::Open)
            {
                std::cout << "Connection established" << std::endl;
                std::cout << "> " << std::flush;
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                // Maybe SSL is not configured properly
                std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
                std::cout << "> " << std::flush;
            }
        }
    );

    webSocket.start();

    webSocket.send("hello world");

    std::cout << "> " << std::flush;

    std::string text;
    while (std::getline(std::cin, text))
    {
        webSocket.send(text);
        std::cout << "> " << std::flush;
    }

    return 0;
}

