#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <variant>
#include <vector>
#include <utility>
#include <poll.h>
#include <set>
#include <unistd.h>
#include <sys/timerfd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <cstring>
#include <sstream>
#include <endian.h>
#include <map>

/* Some crc table */
constexpr uint32_t crc32Table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};
/* Crc function */
uint32_t crc32(const char *buffer, ssize_t size)
{
    const char *p = buffer;
    uint32_t crc;
    crc = ~0U;
    while (size--)
        crc = crc32Table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return crc ^ ~0U;
}
/* Struct storing new game data */
struct NewGame
{
    uint32_t maxx;
    uint32_t maxy;
    std::string players;
};
/* Struct storing pixel data */
struct Pixel
{
    uint32_t x;
    uint32_t y;
    uint8_t player;
};
/* Struct storing player eliminated data */
struct PlayerEliminated
{
    uint8_t player;
};
/* Struct storing no data :) */
struct GameOver
{
};

/* Struct for event with std::variant :) */
struct Event
{
    uint32_t len;
    uint32_t eventNo;
    uint8_t eventType;
    std::variant<NewGame, Pixel, PlayerEliminated, GameOver> eventData;
    uint32_t crc;
};

/* Map for all event <3 */
std::map<uint32_t, Event> eventMap;

/* Class representing socket */
class Socket
{
private:
    /* Socket value */
    int sock = 0;

public:
    /* Returning socket value */
    int getSocket() const
    {
        return sock;
    }
    /* Connecting socket, Flag = 0 -> TCP, Flag = 1 -> UDP */
    void connectSocket(const std::string &address, const std::string &port, bool flag)
    {
        struct addrinfo addr_hints;
        struct addrinfo *addr_result;

        memset(&addr_hints, 0, sizeof(struct addrinfo));

        if (flag == 0)
        {
            addr_hints.ai_family = AF_INET6;
            addr_hints.ai_socktype = SOCK_STREAM;
            addr_hints.ai_protocol = IPPROTO_TCP;
        }
        else
        {

            addr_hints.ai_family = AF_UNSPEC;
            addr_hints.ai_socktype = SOCK_DGRAM;
            addr_hints.ai_protocol = IPPROTO_UDP;
            addr_hints.ai_flags = 0;
            addr_hints.ai_next = NULL;
            addr_hints.ai_addrlen = 0;
            addr_hints.ai_canonname = NULL;
            addr_hints.ai_addr = NULL;
        }

        /* Getting address */
        if (getaddrinfo(address.c_str(), port.c_str(), &addr_hints, &addr_result) != 0)
        {
            std::cerr << "Cannot get address" << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Getting socket */
        sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
        if (sock < 0)
        {
            std::cerr << "Cannot get address" << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Connecting */
        if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
        {
            std::cerr << "Cannot connect to socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        freeaddrinfo(addr_result);
    }
};

/* Client class, most important object here */
class Client
{
private:
    /* Key statuses */
    bool leftKey;
    bool rightKey;
    bool lastKey; //0 -> left, 1 -> right
    /* Info about the game and ports etc. */
    bool gameStarted = false;
    std::string gameServer;
    std::string playerName;
    std::string gamePort;
    std::string guiServer;
    std::string guiPort;
    Socket gameSocket;
    Socket guiSocket;
    /* Some cool poll, Ein programm, Ein thread, Ein prozess */
    struct pollfd connections[3];
    uint32_t expectingEvent = 0;
    uint32_t maxx = 0;
    uint32_t maxy = 0;
    uint32_t activeGame = 0;
    std::vector<std::string> playersGame;
    
    /* Sending new game event */
    void sendNewGame(int sock, uint32_t maxx, uint32_t maxy, const std::string &players)
    {
        std::string newGame("NEW_GAME");
        std::string blank(" ");
        std::string newLine("\n");
        std::string message = newGame + blank + std::to_string(maxx) + blank + std::to_string(maxy) + blank + players + newLine;
        sendMessage(sock, message);
    }

    /* Sending pixel event */
    void sendPixel(int sock, uint32_t x, uint32_t y, const std::string &player_name)
    {
        std::string newPixel("PIXEL");
        std::string blank(" ");
        std::string newLine("\n");
        std::string message = newPixel + blank + std::to_string(x) + blank + std::to_string(y) + blank + player_name + newLine;
        sendMessage(sock, message);
    }

    /* Sending player eliminated event */
    void sendPlayerEliminated(int sock, const std::string &player)
    {
        std::string message = "PLAYER_ELIMINATED " + player + "\n";
        sendMessage(sock, message);
    }

    /* Sending some message */
    void sendMessage(int sock, const std::string Message)
    {
        if (write(sock, Message.c_str(), Message.size()) < 0)
        {
            std::cerr << "Message error" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    /* Getting direction of player */
    uint8_t getDirection()
    {
        if (lastKey == 0)
        {
            if (leftKey == 1)
            {
                return 2;
            }
        }
        else
        {
            if (rightKey == 1)
            {
                return 1;
            }
        }
        return 0;
    }

    /* Chars to int transform, needs to have size 4 */
    uint32_t charsToInt(const std::string &s)
    {
        return ((uint32_t)(((uint8_t)s[0]) << 0) | (uint32_t)(((uint8_t)s[1]) << 8) | (uint32_t)(((uint8_t)s[2]) << 16) | (uint32_t)((uint8_t)s[3]) << 24);
    }

public:
    /* Constructor and a bunch of setters */
    Client(std::string gameServer_ = "localhost", std::string guiServer_ = "localhost", std::string gamePort_ = "2021",
           std::string playerName_ = "", std::string guiPort_ = "20210") : leftKey(0), rightKey(0), lastKey(0), gameServer(gameServer_), playerName(playerName_), gamePort(gamePort_), guiServer(guiServer_),
                                                                           guiPort(guiPort_){};

    void setGameServer(const std::string &newServer)
    {
        gameServer = newServer;
    }
    void setGamePort(const std::string &newPort)
    {
        gamePort = newPort;
    }
    void setGuiServer(const std::string &newServer)
    {
        gameServer = newServer;
    }
    void setGuiPort(const std::string &newPort)
    {
        guiPort = newPort;
    }
    void setPlayerName(const std::string &newPlayer)
    {
        playerName = newPlayer;
    }
    /* Running client, this is where the fun begins */
    void runClient()
    {
        struct timeval currentTime;
        if (gettimeofday(&currentTime, NULL) < 0)
        {
            std::cerr << "Cannot get sessionID";
            exit(EXIT_FAILURE);
        }
        /* SessionId, who needs it? */
        uint64_t mySessionId = currentTime.tv_usec + currentTime.tv_sec * 1000000;

        /* Gui Server */
        guiSocket.connectSocket(guiServer, guiPort, 0);
        connections[0].fd = guiSocket.getSocket();
        connections[0].events = POLLIN;
        connections[0].revents = 0;

        /* Game Server */
        gameSocket.connectSocket(gameServer, gamePort, 1);
        connections[1].fd = gameSocket.getSocket();
        connections[1].events = POLLIN;
        connections[1].revents = 0;

        /* Shut down this styupid algorithm */
        int flag = 1;
        if (setsockopt(connections[0].fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flag, sizeof(int)) < 0)
        {
            std::cerr << "Cannot make sockete TCP_NODELAY" << std::endl;
        }

        /* Timer creations to send messages in equal timestamps <3 */
        struct sockaddr_in6 serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        int timerfd;
        struct itimerspec timerValue;

        /* Set timerfd */
        timerfd = timerfd_create(CLOCK_REALTIME, 0);
        if (timerfd < 0)
        {
            std::cerr << "Cannot create timer" << std::endl;
            exit(EXIT_FAILURE);
        }

        bzero(&timerValue, sizeof(timerValue));
        timerValue.it_value.tv_sec = 0;
        timerValue.it_value.tv_nsec = 30000000;
        timerValue.it_interval.tv_sec = 0;
        timerValue.it_interval.tv_nsec = 30000000;

        /* Timer */
        connections[2].fd = timerfd;
        connections[2].events = POLLIN;
        connections[2].revents = 0;

        /* Start timer */
        if (timerfd_settime(timerfd, 0, &timerValue, NULL) < 0)
        {
            std::cerr << "Cannot set timer" << std::endl;
            exit(EXIT_FAILURE);
        }


        char buffer[2137];
        std::size_t playerLength = playerName.size();
        int rcv_len, ret;
        uint8_t *tempData;
        long int timersElapsed;
        while (true)
        {
            /* poll, yay */
            ret = poll(connections, 3, -1);
            if (ret < 0)
            {
                std::cerr << "Poll error" << std::endl;
                exit(EXIT_FAILURE);
            }
            else if (ret > 0)
            {
                if (connections[2].revents & (POLLIN | POLLERR))
                {
                    // Receiving timer and sending message to server, requesting first missing message
                    rcv_len = read(connections[2].fd, &timersElapsed, 8);
                    std::string sendMessage;
                    uint64_t sendSessionId = htobe64(mySessionId);
                    uint32_t sendExpectedEvent = htonl(expectingEvent);
                    /* Parsing all those bytes... */
                    tempData = (uint8_t *)(&sendSessionId);

                    for (int j = 0; j < 8; ++j)
                    {
                        sendMessage.push_back(tempData[j]);
                    }

                    sendMessage.push_back(getDirection());

                    tempData = (uint8_t *)(&sendExpectedEvent);

                    for (int j = 0; j < 4; ++j)
                    {
                        sendMessage.push_back(tempData[j]);
                    }

                    for (std::size_t len = 0; len < playerLength; ++len)
                    {
                        sendMessage.push_back(playerName[len]);
                    }
                    /* Gimme that message */
                    rcv_len = write(connections[1].fd, sendMessage.c_str(), sendMessage.size());
                }
                else if (connections[1].revents & (POLLIN | POLLERR))
                {

                    memset(buffer, 0, sizeof(buffer));
                    socklen_t len = sizeof(serveraddr);
                    /* "Hello there, more things to parse */
                    rcv_len = recvfrom(connections[1].fd, &buffer, 1024, 0, (struct sockaddr *)&serveraddr, &len);
                    if (rcv_len < 0)
                    {
                        std::cerr << "Bad read" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    else if (rcv_len > 550)
                    {
                        /* Datagram too big i suppose */
                        std::cerr << "Bad read" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        /* More parsing, exhausting to read and write about */
                        std::string receivedData;
                        std::string receivedEventData;
                        int index = 4, startIndex = 4;

                        for (int i = 0; i < rcv_len; ++i)
                        {
                            receivedData.push_back(buffer[i]);
                        }
                        uint32_t receivedId = ntohl(charsToInt(receivedData.substr(0, 4)));
                        while (index + 4 <= rcv_len)
                        {
                            startIndex = index;
                            uint32_t dataLength = ntohl(charsToInt(receivedData.substr(index, 4)));
                            index += 4;
                            if (dataLength + 4 + index > (uint32_t)rcv_len)
                            {
                                std::cerr << "Bad length" << std::endl;
                                exit(EXIT_FAILURE);
                            }
                            uint32_t eventNumber = ntohl(charsToInt(receivedData.substr(index, 4)));
                            index += 4;
                            uint8_t eventType = (uint8_t)receivedData.substr(index, 1)[0];
                            if (eventType > 3)
                            {
                                std::cerr << "Bad event type" << std::endl;
                                exit(EXIT_FAILURE);
                            }
                            index += 1;
                            /* Now we have to check event typese sheeesh */
                            if (eventType == 0)
                            {
                                /* Parsing new game */
                                maxx = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (maxx > 3920)
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }
                                index += 4;
                                maxy = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (maxx > 2220)
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }
                                index += 4;
                                std::string players;
                                std::string player;
                                eventMap.clear();
                                playersGame.clear();
                                expectingEvent = 0;
                                for (uint32_t i = 0; i < dataLength - 13; ++i)
                                {
                                    if (receivedData[index + i] == 0)
                                    {
                                        if (i + 1 != dataLength - 13)
                                        {
                                            players.push_back(' ');
                                        }
                                        playersGame.push_back(player);
                                        player.clear();
                                    }
                                    else
                                    {
                                        if (receivedData[index + i] < 33 || receivedData[index + i] > 126)
                                        {
                                            std::cerr << "Bad player name" << std::endl;
                                            exit(EXIT_FAILURE);
                                        }
                                        players.push_back(receivedData[index + i]);
                                        player.push_back(receivedData[index + i]);
                                    }
                                }
                                index += dataLength - 13;
                                uint32_t crc = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (crc != crc32(receivedData.substr(startIndex, index - startIndex).c_str(), index - startIndex))
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }

                                index += 4;
                                if (eventMap.find(eventNumber) == eventMap.end())
                                {
                                    eventMap.insert({eventNumber, (Event){dataLength, eventNumber, eventType, (NewGame){maxx, maxy, players}, crc}});
                                }
                            }
                            else if (eventType == 1)
                            {
                                /* Parsing pixel */
                                uint8_t player = (uint8_t)receivedData.substr(index, 1)[0];
                                index += 1;
                                uint32_t x = ntohl(charsToInt(receivedData.substr(index, 4)));
                                index += 4;
                                uint32_t y = ntohl(charsToInt(receivedData.substr(index, 4)));
                                index += 4;
                                uint32_t crc = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (crc != crc32(receivedData.substr(startIndex, index - startIndex).c_str(), index - startIndex))
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }

                                if (player >= playersGame.size())
                                {
                                    std::cerr << "Bad player";
                                    exit(EXIT_FAILURE);
                                }
                                if (x > maxx - 1)
                                {
                                    std::cerr << "Bad pixel";
                                    exit(EXIT_FAILURE);
                                }
                                if (y > maxy - 1)
                                {
                                    std::cerr << "Bad pixel";
                                    exit(EXIT_FAILURE);
                                }

                                index += 4;
                                if (eventMap.find(eventNumber) == eventMap.end())
                                {
                                    eventMap.insert({eventNumber, (Event){dataLength, eventNumber, eventType, (Pixel){x, y, player}, crc}});
                                }
                            }
                            else if (eventType == 2)
                            {
                                /* Parsing player eliminated */
                                uint8_t eliminated = (uint8_t)receivedData.substr(index, 1)[0];
                                index += 1;
                                uint32_t crc = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (crc != crc32(receivedData.substr(startIndex, index - startIndex).c_str(), index - startIndex))
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }
                                if (eliminated >= playersGame.size())
                                {
                                    std::cerr << "Bad player";
                                    exit(EXIT_FAILURE);
                                }
                                index += 4;
                                if (eventMap.find(eventNumber) == eventMap.end())
                                {
                                    eventMap.insert({eventNumber, (Event){dataLength, eventNumber, eventType, (PlayerEliminated){eliminated}, crc}});
                                }
                            }
                            else if (eventType == 3)
                            {
                                /* Parsing game over */
                                uint32_t crc = ntohl(charsToInt(receivedData.substr(index, 4)));
                                if (crc != crc32(receivedData.substr(startIndex, index - startIndex).c_str(), index - startIndex))
                                {
                                    std::cerr << "Bad crc" << std::endl;
                                    break;
                                }
                                index += 4;
                                if (eventMap.find(eventNumber) == eventMap.end())
                                {
                                    eventMap.insert({eventNumber, (Event){dataLength, eventNumber, eventType, (GameOver){}, crc}});
                                }
                            }
                        }
                        //Handling received events
                        int diff = 0;
                        auto start = std::next(eventMap.begin(), expectingEvent);
                        while (expectingEvent + diff < eventMap.size())
                        {
                            if (start->second.eventNo == expectingEvent + diff)
                            {
                                //Choosing event and sending its message
                                auto myEvent = start->second;
                                if (auto val = std::get_if<NewGame>(&(myEvent.eventData)))
                                {
                                    sendNewGame(guiSocket.getSocket(), val->maxx, val->maxy, val->players);
                                    gameStarted = true;
                                    activeGame = receivedId;
                                }
                                else if (auto val = std::get_if<Pixel>(&(myEvent.eventData)))
                                {
                                    if (gameStarted == true && activeGame == receivedId)
                                    {
                                        sendPixel(guiSocket.getSocket(), val->x, val->y, playersGame[val->player]);
                                    }
                                }
                                else if (auto val = std::get_if<PlayerEliminated>(&(myEvent.eventData)))
                                {
                                    if (gameStarted == true && activeGame == receivedId)
                                    {
                                        sendPlayerEliminated(guiSocket.getSocket(), playersGame[val->player]);
                                    }
                                }
                                else if (activeGame == receivedId)
                                {
                                    diff++;
                                    start++;
                                    gameStarted = false;
                                    break;
                                }

                                diff++;
                                start++;
                            }
                            else
                            {
                                break;
                            }
                        }

                        //Actualizing expected event
                        if (eventMap.size() && gameStarted == true)
                        {
                            expectingEvent += diff;
                        }
                    }
                }
                else if (connections[0].revents & (POLLIN | POLLERR))
                {
                    /* Some communication with TCP GUI */
                    memset(buffer, 0, sizeof(buffer));
                    rcv_len = read(guiSocket.getSocket(), buffer, sizeof(buffer));
                    if (rcv_len < 0)
                    {
                        std::cerr << "Bad read" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    else if (rcv_len == 0)
                    {
                        std::cerr << "GUI disconnected" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        std::string input(buffer);
                        if (input == "LEFT_KEY_DOWN\n")
                        {
                            lastKey = 0;
                            leftKey = 1;
                        }
                        else if (input == "LEFT_KEY_UP\n")
                        {
                            leftKey = 0;
                        }
                        else if (input == "RIGHT_KEY_DOWN\n")
                        {
                            lastKey = 1;
                            rightKey = 1;
                        }
                        else if (input == "RIGHT_KEY_UP\n")
                        {
                            rightKey = 0;
                        }
                    }
                }
            }
        }
    }
};

int main(int argc, char *argv[])
{
    /* Chechking whether input is correct and setting up things base on options */
    if (argc < 2)
    {
        std::cerr << "Invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        Client myClient(argv[1]);
        int opt;
        while ((opt = getopt((argc - 1), (argv + 1), "n:p:i:r:")) != -1)
        {
            switch (opt)
            {
            case 'n':
            {
                std::string newPlayer(optarg);
                if (newPlayer.size() > 20)
                {
                    std::cerr << "Invalid player name length" << std::endl;
                    return EXIT_FAILURE;
                }
                for (std::size_t i = 0; i < newPlayer.size(); ++i)
                {
                    if (newPlayer[i] < 33 || newPlayer[i] > 126)
                    {
                        std::cerr << "Invalid player name content" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                myClient.setPlayerName(newPlayer);
            }
            break;
            case 'p':
            {
                std::string newPort(optarg);
                for (std::size_t i = 0; i < newPort.size(); ++i)
                {
                    if (newPort[i] < '0' || newPort[i] > '9')
                    {
                        std::cerr << "Bad server port" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                int32_t portValue = std::atoi(newPort.c_str());
                if (newPort.size() > 5 || portValue < 1000 || portValue > 65535)
                {
                    std::cerr << "Bad server port" << std::endl;
                    return EXIT_FAILURE;
                }
                myClient.setGamePort(newPort);
            }
            break;
            case 'i':
            {
                std::string newServer(optarg);
                myClient.setGuiServer(newServer);
            }
            break;
            case 'r':
            {
                std::string newPort(optarg);
                for (std::size_t i = 0; i < newPort.size(); ++i)
                {
                    if (newPort[i] < '0' || newPort[i] > '9')
                    {
                        std::cerr << "Bad gui port" << std::endl;
                        return EXIT_FAILURE;
                    }
                }
                int32_t portValue = std::atoi(newPort.c_str());
                if (newPort.size() > 5 || portValue < 1000 || portValue > 65535)
                {
                    std::cerr << "Bad gui port" << std::endl;
                    return EXIT_FAILURE;
                }

                myClient.setGuiPort(newPort);
            }
            break;
            default:
                std::cerr << "Unknown option" << std::endl;
                return EXIT_FAILURE;
            }
        }

        if (argc - 1 - optind != 0)
        {
            std::cerr << "Garbage as parameter" << std::endl;
            return EXIT_FAILURE;
        }
        myClient.runClient();
    }
}
