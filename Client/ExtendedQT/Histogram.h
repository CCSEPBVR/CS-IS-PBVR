#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QWidget>
#include <QMouseEvent>

class Histogram : public QWidget
{
    Q_OBJECT

public:
    Histogram(QWidget* parent = nullptr);
    ~Histogram();
    void setDatas( std::vector<int>& data );
    std::vector<int> getDatas() const { return m_data; }

protected:
    void paintEvent( QPaintEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;

private:
    std::vector<int> m_data;
    float m_bias_parameter;
    QPoint m_previous_pos;
};

#endif // HISTOGRAM_H
