#include "OpacityMap.h"

OpacityMap::OpacityMap( QWidget *parent, const QVector<float> &opacities, int resolution )
{
    if( opacities.size() >= 2 ) // 渡されたcolorsのサイズが2以上であれば使用
    {
        m_opacity_map = opacities;
    }
    else // それ以下であれば、デフォルトカラーマップを使用
    {
        m_opacity_map = { 0.0, 0.5, 1.0 };
    }

    if( resolution >= 2 ) // 渡されたresolutionが2以上であれば使用
    {
        m_resolution = resolution;
    }
    else // そうでない場合はデフォルト解像度を使用(256)
    {
        m_resolution = 256; // フォールバック
    }

    // コンストラクタでカラーマップを補間
    interpolateOpacities();
}

OpacityMap::~OpacityMap() {}

void OpacityMap::setOpacities( const QVector<float> &opacities )
{
    if( opacities.size() >= 2 )
    {
        m_opacity_map = opacities;
        interpolateOpacities();  // 新しい色で補間
        update();  // 再描画
    }
}

void OpacityMap::setResolution( int resolution )
{
    if( resolution >= 2 )
    {
        m_resolution = resolution;
        interpolateOpacities();  // 新しい解像度で補間
        update();  // 再描画
    }
}

const QVector<float> OpacityMap::getOpacities()
{
    return m_opacity_map;
}

void OpacityMap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int w = width();
    const int h = height();

    // 1. チェッカーボード背景
    const int cellSize = 10;
    for (int y = 0; y < h; y += cellSize)
    {
        for (int x = 0; x < w; x += cellSize)
        {
            bool dark = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            QColor color = dark ? QColor(180, 180, 180) : QColor(255, 255, 255);
            painter.fillRect(QRect(x, y, cellSize, cellSize), color);
        }
    }

    // 2. アルファグラデーション
    // アルファグラデーション
    // if (m_opacity_map.size() >= 2)
    // {
    //     for (int i = 0; i < m_opacity_map.size() - 1; ++i)
    //     {
    //         float alpha0 = m_opacity_map[i];
    //         float alpha1 = m_opacity_map[i + 1];

    //         int x0 = static_cast<float>(i) / (m_opacity_map.size() - 1) * w;
    //         int x1 = static_cast<float>(i + 1) / (m_opacity_map.size() - 1) * w;

    //         for (int x = x0; x < x1; ++x)
    //         {
    //             float t = static_cast<float>(x - x0) / (x1 - x0);
    //             float alpha = alpha0 + t * (alpha1 - alpha0);
    //             QColor color(0, 0, 0, static_cast<int>(alpha * 255));
    //             painter.setPen(color);
    //             painter.drawLine(x, 0, x, h);
    //         }
    //     }
    // }

    // 3. 不透明度カーブ
    if (m_opacity_map.size() == m_resolution)
    {
        QPainterPath path;
        path.moveTo(0, h * (1.0f - m_opacity_map[0]));

        for (int i = 1; i < m_resolution; ++i)
        {
            float x = static_cast<float>(i) / (m_resolution - 1) * w;
            float y = h * (1.0f - m_opacity_map[i]);
            path.lineTo(x, y);
        }

        QPen pen(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

void OpacityMap::interpolateOpacities()
{
    const int num_data = m_opacity_map.size();
    if (num_data < 2 || m_resolution < 2)
    {
        return;
    }

    QVector<float> interpolatedData;
    interpolatedData.reserve(m_resolution);

    for (int i = 0; i < m_resolution; i++)
    {
        float ratio = static_cast<float>(i) / (m_resolution - 1);
        float scaled_index = ratio * (num_data - 1);
        int index_low = static_cast<int>(std::floor(scaled_index));
        int index_high = std::min(index_low + 1, num_data - 1);
        float local_ratio = scaled_index - index_low;

        float data1 = m_opacity_map[index_low];
        float data2 = m_opacity_map[index_high];

        float data = static_cast<float>(data1 + local_ratio * (data2 - data1));
        interpolatedData.append(data);
    }
    m_opacity_map = interpolatedData;
}
