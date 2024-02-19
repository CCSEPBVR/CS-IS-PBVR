#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

#include <map>
#include <string>
#include <vector>

class TimeMeasure {
private:
  TimeMeasure(/* args */);
  ~TimeMeasure();

  size_t m_max_index;
  size_t m_index;
  std::map<std::string, std::vector<double>> m_elapsed;

  int m_eye;

public:
  static TimeMeasure *GetInstance();
  static std::string At(std::string file, int line);

  bool isLastIndex();
  std::vector<std::string> keys();
  void setIndex(size_t index);
  void record(std::string key, long long int elapsed);
  void record(std::string key, double elapsed);
  std::string loggingString(std::string key);

  inline void setEye(int eye) { m_eye = eye; }
  inline int eye() { return m_eye; }
};

#endif
