#include <enet/enet.h>
#include <iostream>

using namespace std;

ENetAddress address;
ENetHost* server = nullptr;
ENetHost* client = nullptr;

bool CreateServer()
{
    address.host = ENET_HOST_ANY;

    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server != nullptr;
}

bool CreateClient()
{
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    return client != nullptr;
}

int main(int argc, char** argv)
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);
    
    cout << "1. Create Server " << endl;
    cout << "2. Create Client " << endl;
    int userInput;
    cin >> userInput;
    switch (userInput)
    {
    case 1: 
        if (!CreateServer())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet server host.\n");
            exit(EXIT_FAILURE);
        }

        while (true)
        {
            ENetEvent event;
            /* Wait up to 1000 milliseconds for an event. */
            while (enet_host_service(server, &event, 1000) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A new client connected from %x:%u.\n",
                        event.peer->address.host,
                        event.peer->address.port);
                    /* Store any relevant client information here. */
                    event.peer->data = (void*)"Client information";

                    {
                        /* Create a reliable packet of size 7 containing "packet\0" */
                        ENetPacket* packet = enet_packet_create("Hello World!",
                            strlen("packet") + 1,
                            ENET_PACKET_FLAG_RELIABLE);

                        /* enet_host_broadcast (host, 0, packet);         */
                        enet_peer_send(event.peer, 0, packet);
                        /* One could just use enet_host_service() instead. */
                        enet_host_flush(server);
                    }


                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    cout << "A packet of length " << event.packet->dataLength << " was received." << endl;
                    cout << "Packet contains: " << (char*)event.packet->data << endl;
                    
                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy(event.packet);

                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("%s disconnected.\n", (char*)event.peer->data);
                    /* Reset the peer's client information. */
                    event.peer->data = NULL;
                }
            }
        }
        

        break;
    case 2: 
        if (!CreateClient())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet client host.\n");
            exit(EXIT_FAILURE);
        }

        ENetAddress address;
        ENetEvent event;
        ENetPeer* peer;
        /* Connect to some.server.net:1234. */
        enet_address_set_host(&address, "127.0.0.1");
        address.port = 1234;
        /* Initiate the connection, allocating the two channels 0 and 1. */
        peer = enet_host_connect(client, &address, 2, 0);
        if (peer == NULL)
        {
            fprintf(stderr,
                "No available peers for initiating an ENet connection.\n");
            exit(EXIT_FAILURE);
        }
        /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (enet_host_service(client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            cout << "Connection to 127.0.0.1:1234 succeeded." << endl;
        }
        else
        {
            /* Either the 5 seconds are up or a disconnect event was */
            /* received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset(peer);
            cout << "Connection to 127.0.0.1:1234 failed." << endl;
        }

        while (true)
        {
            ENetEvent event;
            // Wait up to 1000 milliseconds for an event
            while (enet_host_service(client, &event, 1000) > 0)
            { 
                switch (event.type)
                {
                case ENET_EVENT_TYPE_RECEIVE:
                    cout << "A packet of length " << event.packet->dataLength
                        << " containing " << (char*)event.packet->data << endl;

                    // Clean up since we are done using it.
                    enet_packet_destroy(event.packet);

                    {
                        /* Create a reliable packet of size 7 containing "packet\0" */
                        ENetPacket* packet = enet_packet_create("Sup Dawg",
                            strlen("hi") + 1,
                            ENET_PACKET_FLAG_RELIABLE);

                        /* enet_host_broadcast (client, 0, packet);         */
                        enet_peer_send(event.peer, 0, packet);
                        /* One could just use enet_host_service() instead. */
                        enet_host_flush(client);
                    }

                    break;
                }
            }
        }

        break;
    default:
        cout << "Bad input" << endl;
        break;
    }

    if (server != nullptr)
    {
        enet_host_destroy(server);
    }

    if (client != nullptr)
    {
        enet_host_destroy(client);
    }    

    return EXIT_SUCCESS;
}