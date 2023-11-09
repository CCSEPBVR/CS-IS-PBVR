#ifndef SLIDERSYNCHRONIZER_H
#define SLIDERSYNCHRONIZER_H

#include <iostream>
#include <set>

class SliderSynchronizationClient {
public:
  virtual void updateSliders(int min, int max, int curTime, int minTime, int maxTime) = 0;
};

class SliderSynchronizer {

private:
  int min, max, val[3];
  std::set<SliderSynchronizationClient *> clients;

  SliderSynchronizer() : min(0), max(0), val{0, 0, 0}, clients(){};
  ~SliderSynchronizer() = default;

public:
  SliderSynchronizer(SliderSynchronizer &foo) = delete;
  SliderSynchronizer &operator=(const SliderSynchronizer &foo) = delete;
  SliderSynchronizer(SliderSynchronizer &&foo) = delete;
  SliderSynchronizer &operator=(const SliderSynchronizer &&foo) = delete;

  void addClient(SliderSynchronizationClient *client) {
    if (client != nullptr) {
      clients.emplace(client);
    }
  }
  void removeClient(SliderSynchronizationClient *client) {
    if (client != nullptr) {
      clients.erase(client);
    }
  }

  void synchronize(SliderSynchronizationClient *src, int min, int max, int curTime, int minTime, int maxTime) {
    this->min = min;
    this->max = max;
    this->val[0] = curTime;
    this->val[1] = minTime;
    this->val[2] = maxTime;

    // std::cout << "Sync : min=" << min << ", max=" << max << ", curTime=" << curTime << ", minTime=" << minTime << ", maxTime=" << maxTime
    //           << std::endl;

    for (SliderSynchronizationClient *client : clients) {
      if (src != client) {
        client->updateSliders(min, max, val[0], val[1], val[2]);
      }
    }
  }

  // singleton getter
  static SliderSynchronizer &instance() {
    static SliderSynchronizer instance;
    return instance;
  }
};

#endif // SLIDERSYNCHRONIZER_H
