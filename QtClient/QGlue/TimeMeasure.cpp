#include "TimeMeasure.h"

TimeMeasure::TimeMeasure(/* args */) : m_index(0), m_max_index(100),  m_eye(-1)
{
}

TimeMeasure::~TimeMeasure()
{
}

TimeMeasure* TimeMeasure::GetInstance()
{
    static TimeMeasure* m_singleton = nullptr;
    if (!m_singleton){
        m_singleton = new TimeMeasure();
    }
    return m_singleton;
}

std::string TimeMeasure::At(std::string file, int line)
{
    return std::string("at ")
        .append(file)
        .append(":")
        .append(std::to_string(line));
}

bool TimeMeasure::isLastIndex()
{
    return (m_index == (m_max_index - 1));
}

std::vector<std::string> TimeMeasure::keys()
{
    std::vector<std::string> keys;
    for (auto const &it : m_elapsed){
        keys.push_back(it.first);
    }
    return keys;
}

void TimeMeasure::setIndex(size_t index)
{
    // Convert index into range [0, m_max_index-1]
    m_index = index % m_max_index;

    std::vector<std::string> keys = this->keys();
    for(auto const &key : keys){
        m_elapsed[key][m_index] = 0.0;
    }
}

void TimeMeasure::record(std::string key, long long int elapsed)
{
    record(key, static_cast<double>(elapsed));
}

void TimeMeasure::record(std::string key, double elapsed)
{
    if (m_elapsed.count(key) == 0){
        m_elapsed[key] = std::vector<double>(m_max_index, -1.0);
        m_elapsed[key][m_index] = 0;
    }
    m_elapsed[key][m_index] += elapsed;
}

std::string TimeMeasure::loggingString(std::string key)
{
    if (m_elapsed.count(key) == 0){
        return "";
    }
    std::vector<double>& elapsed = m_elapsed[key];
    
    std::string log;
    log += key;
    log += " ";
    for(size_t i=0; i<elapsed.size(); i++){
        log += std::to_string(elapsed[i]);
        log += " ";
    }
    return log;
}
