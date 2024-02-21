#ifndef MERGEPANEL_H
#define MERGEPANEL_H

#include <QDockWidget>

#include <QFileDialog>
namespace Ui {
class MergePanel;
}

class MergePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit MergePanel(QWidget *parent = nullptr);
    ~MergePanel();

private:
    Ui::MergePanel *ui;
    void checkMinMaxTimeStep( QFileInfo &fileInfo );
    void checkFileFormat(QFileInfo &fileInfo );

private slots:
    void onFilesTWidgetCellDoubleClicked( int row, int column );
    void onBrowserButtonClicked();
    void onAddButtonClicked();
    void onApplyButtonClicked();
};

#endif // MERGEPANEL_H
