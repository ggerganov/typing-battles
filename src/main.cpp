/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "incppect/incppect.h"

#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <queue>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

constexpr uint64_t kTimeBetweenRounds_ms = 20000;

// ;(( ...
namespace {
    inline float frand() { return (float)(rand())/RAND_MAX; }

    inline uint64_t timestamp_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    inline float timestamp_s() {
        static auto tStart = timestamp_ms();
        return 0.001*(timestamp_ms() - tStart);
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }
}

using Word = std::string;

struct Client {
    std::string name = "__unknown__";
    std::string color = "#000000";
};

struct PlayerInfo {
    int32_t clientId = 0;

    float score = 0.0f;

    bool active = false;
};

struct WordInfo {
    float x = 0.0f;
    float y = 0.0f;

    int32_t clientId = 0;
    float tGuessed_s = 0.0f;

    Word word;
};

struct Dictionary {
    std::vector<Word> words;

    static Dictionary load(const std::string & path) {
        Dictionary result;

        printf("Loading dictionary from '%s'\n", path.c_str());
        std::ifstream fin(path);
        for (std::string line; getline(fin, line);) {
            result.words.emplace_back(std::move(line));
        }
        fin.close();

        return result;
    }
};

struct Event {
    enum Type {
        Unknown = 0,
        StartNewRound,
        EndRound,
        PlayerJoinRoom,
        PlayerInput,
        ClientChangeName,
        ClientChangeColor,
    };

    Type type = Unknown;

    int32_t roomId = -100;
    int32_t clientId = -1;

    std::string inputStr;
};

struct Events {
    std::deque<Event> data;

    std::mutex mutex;
    std::condition_variable cv;

    void push(Event && event) {
        std::lock_guard<std::mutex> lock(mutex);
        data.push_back(std::move(event));
        cv.notify_one();
    }
};

struct Notifier {
    std::thread th;

    Events * events = nullptr;

    void notify(std::function<void()> && f) {
        if (th.joinable()) th.join();
        th = std::thread(std::move(f));
    }
};

struct Room {
    int32_t id = 0;

    std::string name = "__unknown__";

    int32_t roundId = 0;
    int32_t poolSize = 0;

    float tCreated_s = 0.0f;
    float tRoundStart_s = 0.0f;
    float tRoundLength_s = 0.0f;
    float tNextRoundStart_s = 0.0f;

    Dictionary dictionary;

    std::vector<WordInfo> pool;
    std::vector<WordInfo> poolOld;
    std::vector<PlayerInfo> players;

    Notifier notifier;

    WordInfo generate() {
        float dist = 0.0f;

        float x = 0.0f;
        float y = 0.0f;

        for (int i = 0; i < 32; ++i) {
            float cx = 1.50f*frand() - 0.75f;
            float cy = 1.50f*frand() - 0.75f;

            float curd = 1e6;
            for (auto & word : pool) {
                float dx = cx - word.x;
                float dy = cy - word.y;

                float d = dx*dx + dy*dy;
                if (d < curd) {
                    curd = d;
                }
            }

            if (curd > dist) {
                dist = curd;
                x = cx;
                y = cy;
            }
        }

        int wordId = rand()%dictionary.words.size();

        return { x, y, 0, 0.0f, dictionary.words[wordId] };
    }

    void handle(Event && event) {
        switch (event.type) {
            case Event::StartNewRound:
                {
                    if (event.roomId != id) return;

                    ++roundId;
                    printf("Room %d: starting new round %d\n", id, roundId);

                    pool.clear();
                    poolOld.clear();
                    while (pool.size() < poolSize) {
                        pool.emplace_back(generate());
                    }

                    players.erase(std::remove_if(players.begin(), players.end(),
                                                 [](const auto & p) { return p.active == false; }),
                                  players.end());

                    for (auto & player : players) {
                        player.score = 0.0f;
                    }

                    tRoundStart_s = timestamp_s();

                    notifier.notify([&]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(1000.0*tRoundLength_s)));

                        Event event;
                        event.roomId = id;
                        event.type = Event::EndRound;

                        notifier.events->push(std::move(event));
                    });
                }
                break;
            case Event::EndRound:
                {
                    if (event.roomId != id) return;

                    printf("Room %d: ending round %d\n", id, roundId);

                    tNextRoundStart_s = timestamp_s() + 0.001f*kTimeBetweenRounds_ms;

                    notifier.notify([&]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(kTimeBetweenRounds_ms));

                        Event event;
                        event.roomId = id;
                        event.type = Event::StartNewRound;

                        notifier.events->push(std::move(event));
                    });
                }
                break;
            case Event::PlayerJoinRoom:
                {
                    if (event.roomId == id) {
                        for (auto & player : players) {
                            if (player.clientId == event.clientId) {
                                player.active = true;
                                return;
                            }
                        }

                        PlayerInfo player;

                        player.active = true;
                        player.clientId = event.clientId;

                        players.emplace_back(std::move(player));

                        printf("Client (%d) joined room '%s' (%d)\n", event.clientId, name.c_str(), id);
                    } else {
                        for (auto & player : players) {
                            if (player.clientId == event.clientId) {
                                player.active = false;

                                printf("Client (%d) left room '%s' (%d)\n", event.clientId, name.c_str(), id);
                            }
                        }
                    }
                }
                break;
            case Event::PlayerInput:
                {
                    if (event.roomId != id) return;
                    if (::timestamp_s() > tRoundStart_s + tRoundLength_s) return;

                    int playerId = -1;
                    for (int i = 0; i < (int) players.size(); ++i) {
                        if (players[i].clientId == event.clientId) {
                            playerId = i;
                            break;
                        }
                    }

                    if (playerId == -1) return;

                    printf("Room %d: client %d submitted '%s'\n", id, event.clientId, event.inputStr.data());

                    auto & player = players[playerId];

                    bool correct = false;
                    for (auto & word : pool) {
                        if (event.inputStr == word.word) {
                            player.score += 3.0f + 0.2f*word.word.size();
                            correct = true;
                            word.clientId = event.clientId;
                            word.tGuessed_s = ::timestamp_s();
                            poolOld.emplace_back(std::move(word));
                            word = generate();
                            printf("correct!\n");

                            break;
                        }
                    }

                    if (correct == false) {
                        printf("wrong!\n");
                        player.score -= 1.0f;
                        if (player.score < 0.0) player.score = 0.0f;
                    }
                }
                break;
            default:
                {
                    printf("Unhandled event, type = %d\n", event.type);
                }
                break;
        };
    }

    struct Parameters {
        std::string pathData = "./";
    };

    static Room getCpp(Parameters parameters) {
        Room result;

        result.name = "C++";
        result.dictionary = Dictionary::load(parameters.pathData + "words_cpp.txt");
        result.poolSize = 30;
        result.tCreated_s = timestamp_s();
        result.tRoundLength_s = 1.0f*60.f;
        result.tRoundStart_s = result.tCreated_s - result.tRoundLength_s;

        return result;
    }

    static Room getEnglish(Parameters parameters) {
        Room result;

        result.name = "English";
        result.dictionary = Dictionary::load(parameters.pathData + "words_alpha.txt");
        result.poolSize = 30;
        result.tCreated_s = timestamp_s();
        result.tRoundLength_s = 1.0f*60.f;
        result.tRoundStart_s = result.tCreated_s - result.tRoundLength_s;

        return result;
    }
};

struct State {
    Events events;
    std::vector<Room> rooms;
    std::map<int, Client> clients;
};

int main(int argc, char ** argv) {
	printf("Usage: %s [port] [path-data]\n", argv[0]);

    int port = argc > 1 ? atoi(argv[1]) : 3000;
    std::string pathData = argc > 2 ? argv[2] : "../data";
    std::string pathHttp = pathData + "/";

    State state;

    auto & events = state.events;
    auto & rooms = state.rooms;
    auto & clients = state.clients;

    rooms.emplace_back(Room::getCpp(Room::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(Room::getEnglish(Room::Parameters { .pathData = pathData + "/" }));

    Incppect::getInstance().var("tcur_s", [&](const auto & ) { static float t; t = timestamp_s(); return Incppect::view(t); });
    Incppect::getInstance().var("nrooms", [&](const auto & ) { static int n; n = rooms.size(); return Incppect::view(n); });
    Incppect::getInstance().var("rooms[%d].name", [&](const auto & idxs) { return rooms[idxs[0]].name.data(); });
    Incppect::getInstance().var("rooms[%d].roundid", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].roundId); });
    Incppect::getInstance().var("rooms[%d].nplayers", [&](const auto & idxs) { static int n; n = rooms[idxs[0]].players.size(); return Incppect::view(n); });
    Incppect::getInstance().var("rooms[%d].nwords", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].poolSize); });
    Incppect::getInstance().var("rooms[%d].nold", [&](const auto & idxs) { static int n; n = rooms[idxs[0]].poolOld.size(); return Incppect::view(n); });
    Incppect::getInstance().var("rooms[%d].tstart_s", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].tRoundStart_s); });
    Incppect::getInstance().var("rooms[%d].tnextstart_s", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].tNextRoundStart_s); });
    Incppect::getInstance().var("rooms[%d].tlength_s", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].tRoundLength_s); });
    Incppect::getInstance().var("rooms[%d].words[%d].x", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].pool[idxs[1]].x); });
    Incppect::getInstance().var("rooms[%d].words[%d].y", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].pool[idxs[1]].y); });
    Incppect::getInstance().var("rooms[%d].words[%d].word", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].pool[idxs[1]].word); });
    Incppect::getInstance().var("rooms[%d].old[%d].x", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].poolOld[idxs[1]].x); });
    Incppect::getInstance().var("rooms[%d].old[%d].y", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].poolOld[idxs[1]].y); });
    Incppect::getInstance().var("rooms[%d].old[%d].word", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].poolOld[idxs[1]].word); });
    Incppect::getInstance().var("rooms[%d].old[%d].color", [&](const auto & idxs) { return Incppect::view(clients[rooms[idxs[0]].poolOld[idxs[1]].clientId].color); });
    Incppect::getInstance().var("rooms[%d].old[%d].tguessed_s", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].poolOld[idxs[1]].tGuessed_s); });
    Incppect::getInstance().var("rooms[%d].players[%d].name", [&](const auto & idxs) { return Incppect::view(clients[rooms[idxs[0]].players[idxs[1]].clientId].name); });
    Incppect::getInstance().var("rooms[%d].players[%d].color", [&](const auto & idxs) { return Incppect::view(clients[rooms[idxs[0]].players[idxs[1]].clientId].color); });
    Incppect::getInstance().var("rooms[%d].players[%d].score", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].players[idxs[1]].score); });
    Incppect::getInstance().var("rooms[%d].players[%d].active", [&](const auto & idxs) { return Incppect::view(rooms[idxs[0]].players[idxs[1]].active); });

    Incppect::getInstance().handler([&](int clientId, Incppect::EventType etype, std::string_view data) {
        Event event;

        event.clientId = clientId;

        switch (etype) {
            case Incppect::Connect:
                {
                    return;
                }
                break;
            case Incppect::Disconnect:
                {
                    event.type = Event::PlayerJoinRoom;
                    event.roomId = -1;
                }
                break;
            case Incppect::Custom:
                {
                    std::stringstream ss;
                    ss << data;

                    int type = -1;
                    ss >> type;

                    switch (type) {
                        case 10:
                            {
                                // change name
                                event.type = Event::ClientChangeName;
                                std::getline(ss, event.inputStr);
                                event.inputStr = std::string(event.inputStr.data());
                                ::trim(event.inputStr);
                            }
                            break;
                        case 11:
                            {
                                // change color
                                event.type = Event::ClientChangeColor;
                                std::getline(ss, event.inputStr);
                                event.inputStr = std::string(event.inputStr.data());
                                ::trim(event.inputStr);
                            }
                            break;
                        case 20:
                            {
                                // join room
                                event.type = Event::PlayerJoinRoom;
                                ss >> event.roomId;
                            }
                            break;
                        case 30:
                            {
                                // player input
                                event.type = Event::PlayerInput;
                                ss >> event.roomId;
                                std::getline(ss, event.inputStr);
                                event.inputStr = std::string(event.inputStr.data());
                                ::trim(event.inputStr);
                            }
                            break;
                        default:
                            {
                                printf("Unknown input received from client: id = %d, type = %d\n", clientId, type);
                                return;
                            }
                            break;
                    };
                }
                break;
        };

        std::lock_guard<std::mutex> lock(events.mutex);
        events.data.push_back(std::move(event));
        events.cv.notify_one();
    });

    auto incppect = Incppect::getInstance().runAsync(Incppect::Parameters {
        .portListen = port,
        .maxPayloadLength_bytes = 256*1024,
        .httpRoot = pathHttp,
    });

    for (int i = 0; i < (int) rooms.size(); ++i) {
        rooms[i].id = i;
        rooms[i].notifier.events = &events;
        rooms[i].notifier.notify([&, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            Event event;
            event.roomId = i;
            event.type = Event::StartNewRound;

            events.push(std::move(event));
        });
    }

    while (true) {
        Event event;
        {
            std::unique_lock<std::mutex> lock(events.mutex);
            if (events.data.size() == 0) {
                events.cv.wait(lock, [&]() { return events.data.size() > 0; });
            }
            event = std::move(events.data.front());
            events.data.pop_front();
        }

        switch (event.type) {
            case Event::ClientChangeName:
                {
                    std::string name = event.inputStr;
                    clients[event.clientId].name = name;

                    printf("Client %d changed name: '%s'\n", event.clientId, name.c_str());
                }
                break;
            case Event::ClientChangeColor:
                {
                    std::string color = event.inputStr;
                    clients[event.clientId].color = color;

                    printf("Client %d changed color: '%s'\n", event.clientId, color.c_str());
                }
                break;
            case Event::StartNewRound:
            case Event::EndRound:
            case Event::PlayerInput:
            case Event::PlayerJoinRoom:
                {
                    for (auto & room : rooms) {
                        room.handle(std::move(event));
                    }
                }
                break;
            default:
                {
                    printf("(main) Unhandled event, type = %d\n", event.type);
                }
                break;
        };
    }

    incppect.join();

    return 0;
}
