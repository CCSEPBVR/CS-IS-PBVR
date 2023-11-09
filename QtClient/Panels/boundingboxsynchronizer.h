//#ifndef BOUNDINGBOXSYNCHRONIZER_H
//#define BOUNDINGBOXSYNCHRONIZER_H

//#include <Client/v3defines.h>

//#include <iostream>
//#include <set>
//#include <QGlue/boundingbox.h>

//class BoundingBoxSynchronizerClient {
//public:
//    virtual void updateBoundingBoxVisibility (bool isVisible) = 0;
//};

//class BoundingBoxSynchronizer
//{
//private:
//    int m_isVisible;
//    std::set<BoundingBoxSynchronizerClient *> clients;

//    BoundingBoxSynchronizer() : m_isVisible(INITIAL_STATE_BOUNDING_BOX), clients() {
//    };
//    ~BoundingBoxSynchronizer() = default;

//public:
//    BoundingBoxSynchronizer(BoundingBoxSynchronizer& foo) = delete;
//    BoundingBoxSynchronizer& operator=(const BoundingBoxSynchronizer& foo) = delete;
//    BoundingBoxSynchronizer(BoundingBoxSynchronizer&& foo) = delete;
//    BoundingBoxSynchronizer& operator=(const BoundingBoxSynchronizer&& foo) = delete;

//    void addClient(BoundingBoxSynchronizerClient *client) {
//        if (client != nullptr) {
//            clients.emplace(client);
//        }
//    }
//    void removeClient(BoundingBoxSynchronizerClient *client) {
//        if (client != nullptr) {
//            clients.erase(client);
//        }
//    }

//    void synchronize (BoundingBoxSynchronizerClient *src, bool isVisible) {
//        this->m_isVisible = isVisible;

//        for (BoundingBoxSynchronizerClient *client : clients) {
//            if (src != client) {
//                client->updateBoundingBoxVisibility(m_isVisible);
//            }
//        }
//    }

//    // singleton getter
//    static BoundingBoxSynchronizer& instance () {
//        static BoundingBoxSynchronizer instance;
//        return instance;
//    }
//};

//#endif // BOUNDINGBOXSYNCHRONIZER_H
