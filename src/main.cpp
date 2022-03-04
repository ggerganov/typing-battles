/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "incppect/incppect.h"

#include "common.h"
#include "room-words.h"
#include "room-math.h"

#include <map>
#include <sstream>

using incppect = Incppect<true>;

struct State {
    Events events;
    std::vector<std::shared_ptr<RoomBase>> rooms;
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

    rooms.emplace_back(RoomWords::getCpp(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomWords::getCppBR(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomWords::getBash(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomWords::getBashBR(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomWords::getGit(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomWords::getGitBR(RoomWords::Parameters { .pathData = pathData + "/" }));
    //rooms.emplace_back(RoomWords::getEnglish(RoomWords::Parameters { .pathData = pathData + "/" }));
    //rooms.emplace_back(RoomWords::getEnglishBR(RoomWords::Parameters { .pathData = pathData + "/" }));
    rooms.emplace_back(RoomMath::getMathAdd(RoomMath::Parameters { }));
    rooms.emplace_back(RoomMath::getMathAddBR(RoomMath::Parameters { }));
    rooms.emplace_back(RoomMath::getMathSqrt(RoomMath::Parameters { }));
    rooms.emplace_back(RoomMath::getMathSqrtBR(RoomMath::Parameters { }));

    incppect::getInstance().var("tcur_s", [&](const auto & ) { return incppect::view(timestamp_s()); });
    incppect::getInstance().var("nrooms", [&](const auto & ) { return incppect::view(rooms.size()); });
    incppect::getInstance().var("rooms[%d].name", [&](const auto & idxs) { return rooms[idxs[0]]->name.data(); });
    incppect::getInstance().var("rooms[%d].mode", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->mode); });
    incppect::getInstance().var("rooms[%d].roundid", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->roundId); });
    incppect::getInstance().var("rooms[%d].nplayers", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->players.size()); });
    incppect::getInstance().var("rooms[%d].nqueries", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->poolSize); });
    incppect::getInstance().var("rooms[%d].nold", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->poolOld.size()); });
    incppect::getInstance().var("rooms[%d].tstart_s", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->tRoundStart_s); });
    incppect::getInstance().var("rooms[%d].tnextstart_s", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->tNextRoundStart_s); });
    incppect::getInstance().var("rooms[%d].tlength_s", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->tRoundLength_s); });
    incppect::getInstance().var("rooms[%d].pool[%d].x", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->pool[idxs[1]].x); });
    incppect::getInstance().var("rooms[%d].pool[%d].y", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->pool[idxs[1]].y); });
    incppect::getInstance().var("rooms[%d].pool[%d].text", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->pool[idxs[1]].text); });
    incppect::getInstance().var("rooms[%d].pool_old[%d].x", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->lastOldQuery(idxs[1]).x); });
    incppect::getInstance().var("rooms[%d].pool_old[%d].y", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->lastOldQuery(idxs[1]).y); });
    incppect::getInstance().var("rooms[%d].pool_old[%d].text", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->lastOldQuery(idxs[1]).text); });
    incppect::getInstance().var("rooms[%d].pool_old[%d].color", [&](const auto & idxs) { return incppect::view(clients[rooms[idxs[0]]->lastOldQuery(idxs[1]).clientId].color); });
    incppect::getInstance().var("rooms[%d].pool_old[%d].tguessed_s", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->lastOldQuery(idxs[1]).tGuessed_s); });
    incppect::getInstance().var("rooms[%d].my_id[%d]", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->playerId(idxs[1])); });
    incppect::getInstance().var("rooms[%d].players[%d].name", [&](const auto & idxs) { return incppect::view(clients[rooms[idxs[0]]->players[idxs[1]].clientId].name); });
    incppect::getInstance().var("rooms[%d].players[%d].color", [&](const auto & idxs) { return incppect::view(clients[rooms[idxs[0]]->players[idxs[1]].clientId].color); });
    incppect::getInstance().var("rooms[%d].players[%d].score", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->players[idxs[1]].score); });
    incppect::getInstance().var("rooms[%d].players[%d].active", [&](const auto & idxs) { return incppect::view(rooms[idxs[0]]->players[idxs[1]].active); });

    incppect::getInstance().handler([&](int clientId, incppect::EventType etype, std::string_view data) {
        Event event;

        event.clientId = clientId;

        switch (etype) {
            case incppect::Connect:
                {
                    return;
                }
                break;
            case incppect::Disconnect:
                {
                    event.type = Event::PlayerJoinRoom;
                    event.roomId = -1;
                }
                break;
            case incppect::Custom:
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

    incppect::Parameters parameters;

    parameters.portListen = port;
    parameters.maxPayloadLength_bytes = 256*1024;
    parameters.httpRoot = pathHttp;
    parameters.resources = { "", "index.html", "names.js", "particles.js", };
    parameters.sslKey = "key.pem";
    parameters.sslCert = "cert.pem";

    auto incppect = incppect::getInstance().runAsync(parameters);

    for (int i = 0; i < (int) rooms.size(); ++i) {
        rooms[i]->id = i;
        rooms[i]->notifier.events = &events;
        rooms[i]->notifier.notify([&, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            Event event;
            event.roomId = i;
            event.type = Event::CheckStart;

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
            case Event::CheckStart:
            case Event::EndRound:
            case Event::PlayerInput:
            case Event::PlayerJoinRoom:
            case Event::EndBRRound:
                {
                    for (auto & room : rooms) {
                        room->handle(std::move(event));
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
