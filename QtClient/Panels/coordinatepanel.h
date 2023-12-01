#ifndef COORDINATEPANEL_H
#define COORDINATEPANEL_H

#include <QDockWidget>
#include "Client/VisualizationParameter.h"

namespace Ui {
class CoordinatePanel;
}

/**
 * @brief
 *
 */
class CoordinatePanel : public QDockWidget
{

public:

    explicit CoordinatePanel(QWidget *parent = 0);
    ~CoordinatePanel();

    void setParam( kvs::visclient::VisualizationParameter* param );
    void resetChangeFlagForStoreParticle();
    bool getChangeCoordFlag();
    bool getChangeCoordFlagForStoreParticle();
    void resetResetViewFlag();
    bool getResetViewFlag();
    void reScale();
//    bool getShowUI();
//    void changeShowUI();
    void setXCoordinateExpression();
    void setYCoordinateExpression();
    void setZCoordinateExpression();

    void setUISynthesizer();

    static std::string param_x_synthesis;
    static std::string param_y_synthesis;
    static std::string param_z_synthesis;

    bool m_use_particle_side_coords = false;

    void logPlay(std::string funcName, std::string str[]);
    void recToggle(){ m_isRec ? m_isRec = false : m_isRec = true; }

protected:
     bool m_coordinate_for_store_particle; // 座標式に変更があった場合(粒子保存機能用)
     bool m_coordinate;
     bool m_reset_flag = false;

private:
     //Event handlers
     void onApplyButton();
     void onResetScaleButton();

private:


     Ui::CoordinatePanel *ui;
     bool m_isRec = false;
};

#endif // COORDINATEPANEL_H
