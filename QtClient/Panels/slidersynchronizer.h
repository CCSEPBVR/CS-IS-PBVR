#ifndef SLIDERSYNCHRONIZER_H
#define SLIDERSYNCHRONIZER_H

#include <iostream>
//#include <set>
#include <vector>

class SliderSynchronizationClient {
public:
    virtual void updateSliders (int min, int max, int curTime, int minTime, int maxTime) = 0;
};

class SliderSynchronizer
{

private:
    int min, max, val[3];
//    std::set<SliderSynchronizationClient *> clients;
    std::vector<SliderSynchronizationClient *> clients;

    SliderSynchronizer() : min(0), max(0), val{0,0,0}, clients() {
    };
    ~SliderSynchronizer() = default;

public:
    SliderSynchronizer(SliderSynchronizer& foo) = delete;
    SliderSynchronizer& operator=(const SliderSynchronizer& foo) = delete;
    SliderSynchronizer(SliderSynchronizer&& foo) = delete;
    SliderSynchronizer& operator=(const SliderSynchronizer&& foo) = delete;

    void addClient(SliderSynchronizationClient *client);
    void removeClient(SliderSynchronizationClient *client);

    void synchronize (SliderSynchronizationClient *src, int min, int max, int curTime, int minTime, int maxTime);

public:
    // singleton getter
    static SliderSynchronizer& instance () {
        static SliderSynchronizer instance;
        return instance;
    }

};


#endif // SLIDERSYNCHRONIZER_H
