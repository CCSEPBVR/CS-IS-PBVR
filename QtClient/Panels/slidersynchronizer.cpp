#include "slidersynchronizer.h"
#include <vector>
#include <iostream>
#include <string>

void SliderSynchronizer::synchronize (SliderSynchronizationClient *src, int min, int max, int curTime, int minTime, int maxTime) {
    this->min = min;
    this->max = max;
    this->val[0] = curTime;
    this->val[1] = minTime;
    this->val[2] = maxTime;

//    std::string src_type = std::string(typeid(*src).name());
//    std::cerr << "Sync : src=" << src_type << ", min=" << min << ", max=" << max << ", curTime=" << curTime << ", minTime=" << minTime << ", maxTime=" << maxTime << std::endl;
//    std::cerr << "broadcast slider values" << std::endl;

    for (SliderSynchronizationClient *client : clients) {

//        std::string target_type = std::string(typeid(*client).name());
//        std::cerr << "   -> " << target_type << std::endl;

        if (src != client) {
            client->updateSliders(min, max, val[0], val[1], val[2]);
        }
    }
}

void SliderSynchronizer::addClient(SliderSynchronizationClient *client) {
//        if (client != nullptr) {
//            clients.emplace(client);
//        }
    if (client == nullptr) {
        return;
    }

    for (auto& old_client : clients) {
        if (old_client == client) {
            return;
        }
    }

    clients.push_back(client);

//    std::cerr << "SliderSynchronizer : add new client : " << std::string(typeid(*client).name()) << std::endl;
}

void SliderSynchronizer::removeClient(SliderSynchronizationClient *client) {
//        if (client != nullptr) {
//            clients.erase(client);
//        }
    if (client == nullptr) {
        return;
    }

    for (auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it) == client) {
            clients.erase(it);
            return;
        }
    }
}
