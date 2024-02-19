#pragma once

#include <map>
#include <set>

#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>
#include <kvs/Timer>

class LineObjectProxy : public kvs::LineObject {
private:
  LineObject list[2];
  int m_index = 0;
  //    std::string m_name;

  LineObject *active() { return &list[m_index]; }

public:
  LineObjectProxy() : m_index(0) {
    list[0] = LineObject();
    list[1] = LineObject();
    list[0].setName("proxy");
    list[1].setName("proxy");
  }

  LineObject *swap() {
    m_index = 1 - m_index;
    return active();
  }
  LineObject *operator->() { return active(); }
  operator LineObject *() { return active(); }

  void showAll() {
    list[0].show();
    list[1].show();
  }
  void hideAll() {
    list[0].hide();
    list[1].hide();
  }
  int index() { return m_index; }
};
