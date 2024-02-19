#pragma once

#include <map>
#include <set>

#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>
#include <kvs/Timer>

#include "TexturedPolygonObject.h"

class TexturedPolygonObjectProxy : public kvs::jaea::TexturedPolygonObject {
private:
  TexturedPolygonObject list[2];
  int m_index = 0;
  //    std::string m_name;

  TexturedPolygonObject *active() { return &list[m_index]; }

public:
  TexturedPolygonObjectProxy() : m_index(0) {
    list[0] = TexturedPolygonObject();
    list[1] = TexturedPolygonObject();
    list[0].setName("proxy");
    list[1].setName("proxy");
  }

  TexturedPolygonObject *swap() {
    m_index = 1 - m_index;
    return active();
  }
  TexturedPolygonObject *operator->() { return active(); }
  operator TexturedPolygonObject *() { return active(); }
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
