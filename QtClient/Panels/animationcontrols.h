#ifndef ANIMATIONCONTROLS_H
#define ANIMATIONCONTROLS_H

#include <QDockWidget>

namespace Ui {
class AnimationControls;
}
class AnimationControlParameter
{
public:
    int             m_mode;
    int             m_capture;
    std::string     m_image_file;
    float           m_sampling_rate;
    std::string     m_log_file;
    int             m_load;
};

class AnimationControls : public QDockWidget
{

public:

    explicit AnimationControls(QWidget *parent = 0);

    static void RedrawAnimationControl(int ValueNumKeyFrames);
    static int getInterpolationValue();
    static AnimationControlParameter getParameters();
    static std::string getImageFileName(std::string step, std::string ext);

    ~AnimationControls();
    void logPlay(std::string funcName, std::string str[]);
    void recToggle(){ m_isRec ? m_isRec = false : m_isRec = true; }

private:
    static AnimationControls* instance;

    //Event handlers
    void onAnimationFileChange(const QString &arg1);
    void onEnableCaptureChanged(int index);
    void onImageFileChanged(const QString &arg1);
    void onInterpolationValueChanged(int arg1);

private:
    Ui::AnimationControls *ui;
    bool m_isRec = false;
};

#endif // ANIMATIONCONTROLS_H
