#ifndef RENDEROPTIONS_H
#define RENDEROPTIONS_H

#include <QDockWidget>
#include "Widgets/MergePanel.h"
namespace Ui {
class RenderOptions;
}

class RenderOptions : public QDockWidget
{
    Q_OBJECT

public:
    explicit RenderOptions(QWidget *parent = nullptr, MergePanel* merge = nullptr, Connect* connect_panel = nullptr);
    ~RenderOptions();

private:
    Ui::RenderOptions *ui;
    MergePanel* m_merge;
    Connect* m_connect;

private slots:
    void onApplyButtonClicked();
};

#endif // RENDEROPTIONS_H
