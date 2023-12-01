#pragma once

#include <map>
#include <set>

#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>
#include <kvs/Timer>

class PolygonObjectProxy : public kvs::PolygonObject {
private:
  PolygonObject list[2];
  int m_index = 0;
  //    std::string m_name;

  PolygonObject *active() { return &list[m_index]; }

public:
  PolygonObjectProxy() : m_index(0) {
    list[0] = PolygonObject();
    list[1] = PolygonObject();
    list[0].setName("proxy");
    list[1].setName("proxy");
  }

  PolygonObject *swap() {
    m_index = 1 - m_index;
    return active();
  }
  PolygonObject *operator->() { return active(); }
  operator PolygonObject *() { return active(); }
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
