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
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <chrono>
#include <cmath>

/* Again crc table */
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

/* Class that generates random numbers, cool stuff */
class RandomNumberGenerator
{
private:
    uint32_t value;
    static constexpr uint64_t MODULO_CONST = 4294967291;
    static constexpr uint64_t MULTIPLICATION_CONST = 279410273;

public:
    RandomNumberGenerator(uint32_t startValue) : value(startValue) {}
    /* Generating */
    uint32_t generateNewNumber()
    {
        uint32_t returnValue = value;
        value = ((uint64_t)value * MULTIPLICATION_CONST) % MODULO_CONST;
        return returnValue;
    }
    /* Getting seed */
    uint32_t getSeed()
    {
        return value;
    }
    /* And setting it, awesome */
    void setNewSeed(uint32_t startValue)
    {
        value = startValue;
    }
};

/* Struct for storing new game event data */
struct NewGame
{
    uint32_t maxx;
    uint32_t maxy;
    std::vector<std::string> players;
};

/* Struct for storing pixel data */
struct Pixel
{
    uint32_t x;
    uint32_t y;
    uint8_t player;
};

/* Struct for storing player eliminated data */
struct PlayerEliminated
{
    uint8_t player;
};

/* Empty struct for being over */
struct GameOver
{
};

/* Location struct of each player */
struct Location
{
    std::pair<double, double> position;
    uint16_t destination;
    std::string playerName;
};

/* Ah  yes, event with std variant of previous data structs */
struct Event
{
    uint32_t len;
    uint32_t eventNo;
    uint8_t eventType;
    std::variant<NewGame, Pixel, PlayerEliminated, GameOver> eventData;
    uint32_t crc;
};

/* Struct holding game state, very important */
struct Game
{
    uint32_t gameId;
    std::vector<Location> playerLocations;
    std::vector<Event> events;
    std::set<std::pair<uint32_t, int32_t>> eatenPixels;
};
/* Custom comparator to make map working */
struct customComparator
{
    bool operator()(const std::pair<struct in6_addr, uint16_t> &a, const std::pair<struct in6_addr, uint16_t> &b) const
    {
        return a.second < b.second;
    }
};

/* Sombe client data */
struct Client
{
    uint64_t sessionId;
    std::string playerName;
    struct sockaddr_in6 sockaddr;
    uint8_t direction;
    bool eliminated;
    bool isReady;
    bool connected;
    uint64_t lastConnection;
};

/* Converting chars to decimal values, very handy */
uint32_t charsToInt(const std::string &s)
{
    return ((uint32_t)(((uint8_t)s[0]) << 0) | (uint32_t)(((uint8_t)s[1]) << 8) | (uint32_t)(((uint8_t)s[2]) << 16) | (uint32_t)((uint8_t)s[3]) << 24);
}

uint64_t charsToIntExt(const std::string &s)
{
    return ((uint64_t)(((uint8_t)s[0]) << 0) | (uint64_t)((uint8_t)s[1]) << 8 | (uint64_t)((uint8_t)s[2]) << 16 | (uint64_t)((uint8_t)s[3]) << 24 | (uint64_t)((uint8_t)s[4]) << 32 | (uint64_t)((uint8_t)s[5]) << 40 | (uint64_t)((uint8_t)s[6]) << 48 | (uint64_t)((uint8_t)s[7]) << 56);
}

/* Maps for a lot of stuff */
std::map<std::pair<struct in6_addr, uint16_t>, Client, customComparator> Players;
std::map<std::string, std::pair<struct in6_addr, uint16_t>> PlayerNames;
std::map<std::string, std::pair<struct in6_addr, uint16_t>> ClientNames;
std::map<std::pair<struct in6_addr, uint16_t>, Client, customComparator> Observers;
std::map<std::pair<struct in6_addr, uint16_t>, Client, customComparator> Clients;

/* Server class, most important one */
class Server
{
private:
    /* Sombie boring data about server */
    static constexpr uint8_t MAX_CAPABILITY = 27;
    uint32_t portNum;
    uint32_t seed;
    uint32_t turningSpeed;
    uint32_t roundsPerSec;
    uint32_t width;
    uint32_t height;
    uint8_t availablePlayers;
    uint8_t readyPlayers = 0;
    struct pollfd client[MAX_CAPABILITY];
    uint32_t lastEventNo = 0;
    RandomNumberGenerator gen;
    Game myGame;
    bool gameStarted;
    uint8_t playersAlive = 0;
    std::string preparedResult;

    /* Start a game and have some fun */
    void startGame()
    {
        /* Set up */
        NewGame newGameEventData;
        lastEventNo = 0;
        Event newGameEvent;
        myGame.gameId = gen.generateNewNumber();
        myGame.eatenPixels.clear();
        myGame.playerLocations.clear();
        PlayerNames.clear();
        myGame.events.clear();
        lastEventNo = 0;
        Players.clear();

        uint32_t playerSizes = 0;

        for (auto it = Clients.begin(); it != Clients.end(); ++it)
        {
            it->second.eliminated = false;
            if (it->second.playerName != "" && it->second.isReady == true)
            {
                Players.insert(*it);
                playerSizes += it->second.playerName.size();
                PlayerNames.insert({it->second.playerName, it->first});
                newGameEventData.players.push_back(it->second.playerName);
            }
        }
        playersAlive = Players.size();
        newGameEventData.maxx = width;
        newGameEventData.maxy = height;
        std::sort(newGameEventData.players.begin(), newGameEventData.players.end());
        /* Genereting newe game event */
        newGameEvent.eventData = newGameEventData;
        newGameEvent.eventType = 0;
        newGameEvent.eventNo = myGame.events.size();
        newGameEvent.len = 13 + playerSizes + Players.size();
        myGame.events.push_back(newGameEvent);

        for (std::size_t i = 0; i < Players.size(); ++i)
        {
            /* Generating new location */
            Location newLocation;
            double xPosition = (gen.generateNewNumber() % newGameEventData.maxx) + 0.5;
            double yPosition = (gen.generateNewNumber() % newGameEventData.maxy) + 0.5;
            newLocation.position = std::make_pair(xPosition, yPosition);
            newLocation.destination = (gen.generateNewNumber() % 360);
            newLocation.playerName = newGameEventData.players[i];
            myGame.playerLocations.push_back(newLocation);

            if (myGame.eatenPixels.find(std::make_pair((uint32_t)xPosition, (uint32_t)yPosition)) == myGame.eatenPixels.end())
            {
                /* Generating new pixel */
                Event newPixelEvent;
                newPixelEvent.eventNo = myGame.events.size();
                newPixelEvent.eventType = 1;
                newPixelEvent.len = 14;
                Pixel pixelData;
                pixelData.player = (uint8_t)i;
                pixelData.x = (uint32_t)xPosition;
                pixelData.y = (uint32_t)yPosition;
                newPixelEvent.eventData = pixelData;
                myGame.events.push_back(newPixelEvent);
                myGame.eatenPixels.insert(newLocation.position);
            }
            else
            {
                /* Eliminating player */
                Event newPlayerEliminatedEvent;
                newPlayerEliminatedEvent.eventNo = myGame.events.size();
                newPlayerEliminatedEvent.eventType = 2;
                newPlayerEliminatedEvent.len = 6;
                PlayerEliminated playerEliminatedData;
                playerEliminatedData.player = (uint8_t)i;
                newPlayerEliminatedEvent.eventData = playerEliminatedData;
                myGame.events.push_back(newPlayerEliminatedEvent);
                Clients.find(PlayerNames.find(newGameEventData.players[i])->second)->second.eliminated = true;
                Clients.find(PlayerNames.find(myGame.playerLocations[i].playerName)->second)->second.isReady = false;
                readyPlayers--;

                playersAlive--;
                /* Ending game */
                if (playersAlive == 1)
                {
                    Event gameOverEvent;
                    gameOverEvent.eventNo = myGame.events.size();
                    gameOverEvent.eventType = 3;
                    gameOverEvent.len = 5;
                    gameOverEvent.eventData = {};
                    myGame.events.push_back(gameOverEvent);
                    gameStarted = false;
                    for (auto it = Clients.begin(); it != Clients.end(); ++it)
                    {
                        if (it->second.eliminated == false && it->second.isReady == true)
                        {
                            it->second.isReady = false;
                            readyPlayers--;
                        }
                    }
                }
            }
        }
    }
    /* Preparing result */
    std::string prepareResult(uint32_t &from)
    {
        auto tdat = htonl(myGame.gameId);
        uint8_t *tempData = (uint8_t *)(&tdat);
        std::string finalResult;
        std::size_t milestone;
        for (int j = 0; j < 4; ++j)
        {
            finalResult.push_back(tempData[j]);
        }
        /* A lot of parsing */
        for (std::size_t i = from; i < myGame.events.size(); ++i)
        {
            std::string result;
            milestone = result.size();
            uint32_t data = htonl(myGame.events[i].len);
            tempData = (uint8_t *)(&data);
            for (int j = 0; j < 4; ++j)
            {
                result.push_back(tempData[j]);
            }
            data = htonl(myGame.events[i].eventNo);
            tempData = (uint8_t *)(&data);
            for (int j = 0; j < 4; ++j)
            {
                result.push_back(tempData[j]);
            }
            tempData = (uint8_t *)(&myGame.events[i].eventType);
            auto eventType = *tempData;
            for (int j = 0; j < 1; ++j)
            {
                result.push_back(tempData[j]);
            }
            /* New game */
            if (eventType == 0)
            {
                if (auto eventData = std::get_if<NewGame>(&myGame.events[i].eventData))
                {
                    data = htonl(eventData->maxx);
                    tempData = (uint8_t *)(&data);
                    for (int j = 0; j < 4; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                    data = htonl(eventData->maxy);
                    tempData = (uint8_t *)(&data);
                    for (int j = 0; j < 4; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                    for (std::size_t j = 0; j < eventData->players.size(); ++j)
                    {
                        for (std::size_t k = 0; k < eventData->players[j].size(); ++k)
                        {
                            result.push_back(eventData->players[j][k]);
                        }
                        result.push_back((uint8_t)(0));
                    }
                }
            }
            /* Pixel */
            else if (auto eventData = std::get_if<Pixel>(&myGame.events[i].eventData))
            {
                if (eventType == 1)
                {
                    tempData = (uint8_t *)(&eventData->player);
                    for (int j = 0; j < 1; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                    data = htonl(eventData->x);
                    tempData = (uint8_t *)(&data);
                    for (int j = 0; j < 4; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                    data = htonl(eventData->y);
                    tempData = (uint8_t *)(&data);
                    for (int j = 0; j < 4; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                }
            }
            /* Player eliminated */
            else if (auto eventData = std::get_if<PlayerEliminated>(&myGame.events[i].eventData))
            {
                if (eventType == 2)
                {
                    tempData = (uint8_t *)(&eventData->player);
                    for (int j = 0; j < 1; ++j)
                    {
                        result.push_back(tempData[j]);
                    }
                }
            }

            myGame.events[i].crc = crc32(result.substr(milestone, result.size() - milestone).c_str(), result.size() - milestone);
            /* Chcking whether pool of events exceeds 550 */
            data = htonl(myGame.events[i].crc);
            tempData = (uint8_t *)(&data);
            for (int j = 0; j < 4; ++j)
            {
                result.push_back(tempData[j]);
            }
            if (finalResult.size() + result.size() <= 500)
            {
                from++;
                finalResult.append(result);
            }
            else
            {
                break;
            }
        }
        return finalResult;
    }

public:
    /* Constructors, setters and getters */
    Server(uint32_t portNum_ = 2021, uint32_t seed_ = time(NULL),
           uint32_t turningSpeed_ = 6, uint32_t roundsPerSec_ = 50,
           uint32_t width_ = 640, uint32_t height_ = 480) : portNum(portNum_), seed(seed_), turningSpeed(turningSpeed_),
                                                            roundsPerSec(roundsPerSec_), width(width_), height(height_), gen(time(NULL))
    {
    }

    void setPortNum(int32_t value)
    {
        portNum = value;
    }
    void setSeed(int32_t value)
    {
        seed = value;
        gen.setNewSeed(seed);
    }
    void setTurningSpeed(int32_t value)
    {
        turningSpeed = value;
    }
    void setRoundsPerSec(int32_t value)
    {
        roundsPerSec = value;
    }
    void setWidth(int32_t value)
    {
        width = value;
    }
    void setHeight(int32_t value)
    {
        height = value;
    }
    /* Running our beautifull server */
    void runServer()
    {
        availablePlayers = 0;
        readyPlayers = 0;
        gameStarted = 0;
        gameStarted = false;
        for (uint8_t i = 0; i < MAX_CAPABILITY; ++i)
        {
            client[i].fd = -1;
            client[i].events = POLLIN;
            client[i].revents = 0;
        }
        client[0].fd = socket(AF_INET6, SOCK_DGRAM, 0); // creating IPv6 UDP socket
        if (client[0].fd == -1)
        {
            std::cerr << "Cannot open IPv6 socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in6 server;
        server.sin6_family = AF_INET6;
        server.sin6_flowinfo = 0;
        server.sin6_addr = in6addr_any;
        server.sin6_port = htons(portNum);
        if (bind(client[0].fd, (struct sockaddr *)&server,
                 (socklen_t)sizeof(server)) == -1)
        {
            std::cerr << "Cannot bind IPv6 socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        long int timersElapsed;

        int timerfd;
        struct itimerspec timerValue;

        /* Set timerfd */
        timerfd = timerfd_create(CLOCK_REALTIME, 0);
        if (timerfd < 0)
        {
            std::cerr << "Cannot create timer" << std::endl;
        }

        bzero(&timerValue, sizeof(timerValue));
        if (roundsPerSec != 1)
        {
            uint32_t myTimer = 1000000000 / roundsPerSec;
            timerValue.it_value.tv_sec = 0;
            timerValue.it_value.tv_nsec = myTimer;
            timerValue.it_interval.tv_sec = 0;
            timerValue.it_interval.tv_nsec = myTimer;
        }
        else
        {
            timerValue.it_value.tv_sec = 1;
            timerValue.it_value.tv_nsec = 0;
            timerValue.it_interval.tv_sec = 1;
            timerValue.it_interval.tv_nsec = 0;
        }

        client[1].fd = timerfd;
        client[1].events = POLLIN;
        client[1].revents = 0;

        /* Start timer */
        if (timerfd_settime(timerfd, 0, &timerValue, NULL) < 0)
        {
            std::cerr << "Cannot set timer" << std::endl;
        }

        char buffer[1024];
        while (true)
        {
            uint32_t ret = poll(client, MAX_CAPABILITY, -1);
            if (ret > 0)
            {
                if (client[1].revents & (POLLIN | POLLERR))
                {
                    ret = read(client[1].fd, &timersElapsed, 8);
                    uint64_t currTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    preparedResult = prepareResult(lastEventNo);
                    for (auto it = Clients.begin(); it != Clients.end();)
                    {
                        if (it->second.lastConnection + 2000 < currTime)
                        {
                            it->second.connected = false;
                            Clients.erase(it++);
                        }
                        else
                        {
                            if (preparedResult.size() > 4)
                            {
                                currTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                                sendto(client[0].fd, preparedResult.c_str(), preparedResult.size(), 0, (struct sockaddr *)&(it->second.sockaddr), sizeof(it->second.sockaddr));
                            }
                            ++it;
                        }
                    }

                    if (gameStarted == true)
                    {
                        // Wykonujemy runde
                        for (std::size_t i = 0; i < myGame.playerLocations.size(); ++i)
                        {
                            auto it = Clients.find(PlayerNames.find(myGame.playerLocations[i].playerName)->second)->second;
                            if (it.eliminated == true)
                            {
                                continue;
                            }
                            uint8_t direction = it.direction;

                            if (direction == 1)
                            {
                                myGame.playerLocations[i].destination = (myGame.playerLocations[i].destination + turningSpeed) % 360;
                            }
                            else if (direction == 2)
                            {
                                myGame.playerLocations[i].destination = (360 + myGame.playerLocations[i].destination - turningSpeed) % 360;
                            }
                            double xPosition = myGame.playerLocations[i].position.first;
                            double yPosition = myGame.playerLocations[i].position.second;

                            auto previous = std::make_pair((uint32_t)xPosition, (uint32_t)yPosition);
                            xPosition += cos(M_PI * myGame.playerLocations[i].destination / 180.0);
                            yPosition += sin(M_PI * myGame.playerLocations[i].destination / 180.0);
                            myGame.playerLocations[i].position.first = xPosition;
                            myGame.playerLocations[i].position.second = yPosition;
                            if (std::make_pair((uint32_t)xPosition, (uint32_t)yPosition) == previous)
                            {
                                continue;
                            }
                            else if (myGame.eatenPixels.find(std::make_pair((uint32_t)xPosition, (uint32_t)yPosition)) == myGame.eatenPixels.end() && xPosition >= 0 && yPosition >= 0 && xPosition <= width - 1 && yPosition <= height - 1)
                            {
                                Event newPixelEvent;
                                newPixelEvent.eventNo = myGame.events.size();
                                newPixelEvent.eventType = 1;
                                newPixelEvent.len = 14;
                                Pixel pixelData;
                                pixelData.player = (uint8_t)i;
                                pixelData.x = (uint32_t)xPosition;
                                pixelData.y = (uint32_t)yPosition;
                                newPixelEvent.eventData = pixelData;
                                myGame.events.push_back(newPixelEvent);
                                myGame.eatenPixels.insert(std::make_pair((uint32_t)xPosition, (uint32_t)yPosition));
                            }
                            else
                            {
                                Event newPlayerEliminatedEvent;
                                newPlayerEliminatedEvent.eventNo = myGame.events.size();
                                newPlayerEliminatedEvent.eventType = 2;
                                newPlayerEliminatedEvent.len = 6;
                                PlayerEliminated playerEliminatedData;
                                playerEliminatedData.player = (uint8_t)i;
                                newPlayerEliminatedEvent.eventData = playerEliminatedData;
                                myGame.events.push_back(newPlayerEliminatedEvent);
                                Clients.find(PlayerNames.find(myGame.playerLocations[i].playerName)->second)->second.eliminated = true;
                                Clients.find(PlayerNames.find(myGame.playerLocations[i].playerName)->second)->second.isReady = false;
                                readyPlayers--;
                                playersAlive--;

                                if (playersAlive == 1)
                                {
                                    Event gameOverEvent;
                                    gameOverEvent.eventNo = myGame.events.size();
                                    gameOverEvent.eventType = 3;
                                    gameOverEvent.len = 5;
                                    gameOverEvent.eventData = {};
                                    myGame.events.push_back(gameOverEvent);
                                    gameStarted = false;
                                    Players.clear();

                                    for (auto it = Clients.begin(); it != Clients.end(); ++it)
                                    {
                                        if (it->second.eliminated == false && it->second.isReady == true)
                                        {
                                            it->second.isReady = false;
                                            readyPlayers--;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if (client[0].revents & (POLLIN | POLLERR))
                {
                    struct sockaddr_in6 recClient;
                    /* Receiving message from client and sending some response back */
                    socklen_t length = sizeof(recClient);
                    memset(buffer, 0, sizeof(buffer));
                    ret = recvfrom(client[0].fd, &buffer, 1024, 0, (sockaddr *)&recClient, &length);
                    if (ret >= 13 && ret <= 33)
                    {
                        std::string message;
                        for (uint32_t i = 0; i < ret; ++i)
                        {
                            message.push_back(buffer[i]);
                        }
                        uint16_t clientPort = recClient.sin6_port;
                        in6_addr clientAddress = recClient.sin6_addr;
                        auto clientId = std::make_pair(clientAddress, clientPort);
                        auto myClient = Clients.find(clientId);
                        /* Chechking whether client is new or not */
                        if (myClient == Clients.end())
                        {
                            Client newClient;
                            bool validRequest = true;
                            newClient.sessionId = be64toh(charsToIntExt(message.substr(0, 8)));

                            newClient.playerName = message.substr(13, message.size() - 13);
                            for (std::size_t i = 0; i < newClient.playerName.size(); ++i)
                            {
                                if (newClient.playerName[i] < 33 || (uint8_t)newClient.playerName[i] > 126)
                                {
                                    validRequest = false;
                                }
                            }
                            if ((uint8_t)message[8] > 2)
                            {
                                validRequest = false;
                            }
                            newClient.direction = (uint8_t)message[8];
                            newClient.isReady = 0;
                            newClient.sockaddr = recClient;
                            newClient.eliminated = 0;
                            newClient.connected = true;
                            newClient.lastConnection = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                            if (Clients.size() < 25 && validRequest && (newClient.playerName == "" || ClientNames.find(newClient.playerName) == ClientNames.end()))
                            {
                                if (newClient.playerName != "")
                                {
                                    ClientNames.insert({newClient.playerName, clientId});
                                    if (newClient.isReady == 0 && newClient.direction != 0)
                                    {
                                        newClient.isReady = 1;
                                        readyPlayers++;
                                    }
                                    availablePlayers++;
                                }
                                Clients.insert({clientId, newClient});
                                Observers.insert({clientId, newClient});
                            }
                        }
                        else
                        {
                            /* Checking some data and preparing  */
                            uint32_t currDirection = (uint8_t)message[8];
                            uint32_t currEventNum = ntohl(charsToIntExt(message.substr(9, 4)));
                            myClient->second.direction = currDirection;
                            myClient->second.lastConnection = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                            if (myClient->second.playerName != "" && myClient->second.isReady == 0 && currDirection != 0)
                            {
                                myClient->second.isReady = 1;
                                readyPlayers++;
                            }
                            std::string result = prepareResult(currEventNum);
                            if (result.size() > 4)
                            {
                                sendto(client[0].fd, result.c_str(), result.size(), 0, (struct sockaddr *)&(myClient->second.sockaddr), sizeof(myClient->second.sockaddr));
                            }
                        }
                    }
                }
            }
            /* Potential game start */
            if (readyPlayers >= 2 && availablePlayers >= 2 && !gameStarted)
            {
                gameStarted = true;
                startGame();
            }
        }
    }
};

int main(int argc, char *argv[])
{
    Server myServer;
    int opt;
    /* Parsing options and checking for garbage */
    while ((opt = getopt(argc, argv, "p:s:t:v:w:h:")) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            int32_t portValue = std::atoi(optarg);
            std::string port(optarg);
            for (std::size_t i = 0; i < port.size(); ++i)
            {
                if (port[i] < '0' || port[i] > '9')
                {
                    std::cerr << "Bad port number" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (port.size() > 5 || portValue < 1024 || portValue > 65535)
            {
                std::cerr << "Bad port number" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setPortNum(std::atoi(optarg));
        }
        break;
        case 's':
        {
            int64_t seed = std::atoll(optarg);
            std::string seedSize(optarg);
            for (std::size_t i = 0; i < seedSize.size(); ++i)
            {
                if (seedSize[i] < '0' || seedSize[i] > '9')
                {
                    std::cerr << "Bad seed" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (seedSize.size() > 10 || seed < 0 || seed > 4294967295)
            {
                std::cerr << "Bad seed" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setSeed(std::atoi(optarg));
        }
        break;
        case 't':
        {
            int32_t turning = std::atoi(optarg);
            std::string turningSize(optarg);
            for (std::size_t i = 0; i < turningSize.size(); ++i)
            {
                if (turningSize[i] < '0' || turningSize[i] > '9')
                {
                    std::cerr << "Bad turning speed" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (turningSize.size() > 2 || turning < 1 || turning > 90)
            {
                std::cerr << "Bad turning speed" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setTurningSpeed(turning);
        }
        break;
        case 'v':
        {
            int32_t rounds = std::atoi(optarg);
            std::string roundsSize(optarg);
            for (std::size_t i = 0; i < roundsSize.size(); ++i)
            {
                if (roundsSize[i] < '0' || roundsSize[i] > '9')
                {
                    std::cerr << "Bad round per second" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (roundsSize.size() > 3 || rounds < 1 || rounds > 250)
            {
                std::cerr << "Bad rounds per second" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setRoundsPerSec(rounds);
        }
        break;
        case 'w':
        {
            int32_t width = std::atoi(optarg);
            std::string widthString(optarg);
            for (std::size_t i = 0; i < widthString.size(); ++i)
            {
                if (widthString[i] < '0' || widthString[i] > '9')
                {
                    std::cerr << "Bad width" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (widthString.size() > 4 || width < 16 || width > 1920)
            {
                std::cerr << "Bad width" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setWidth(width);
        }
        break;
        case 'h':
        {
            int32_t height = std::atoi(optarg);
            std::string heightString(optarg);
            for (std::size_t i = 0; i < heightString.size(); ++i)
            {
                if (heightString[i] < '0' || heightString[i] > '9')
                {
                    std::cerr << "Bad height" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            if (heightString.size() > 4 || height < 16 || height > 1080)
            {
                std::cerr << "Bad height" << std::endl;
                return EXIT_FAILURE;
            }
            myServer.setHeight(height);
        }
        break;
        default:
            std::cerr << "Unknown option" << std::endl;
            return EXIT_FAILURE;
        }
    }
    if (argc - optind != 0)
    {
        std::cerr << "Garbage as parameter" << std::endl;
        return EXIT_FAILURE;
    }
    myServer.runServer();
}