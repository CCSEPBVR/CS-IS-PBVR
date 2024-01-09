#include "ColorMapEditor.h"
#include "TFEColorMapBar.h"
#include "ui_ColorMapEditor.h"

#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <kvs/DivergingColorMap>

ColorMapEditor::ColorMapEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorMapEditor)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(40);

//    QFile file("/Users/t0603/Work/json/test.json");
        QFile file("/Users/t0603/Work/json/test2.json");
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Failed to open the file.";
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // JSON Document
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    // Root JSON Array
    QJsonArray rootArray = jsonDoc.array();

    int count = 0;
    for (const QJsonValue &value : rootArray)
    {
        {
            kvs::ColorMap color_map;

            int row_position = ui->tableWidget->rowCount();
            if( count % 2 == 0)
            {
                ui->tableWidget->insertRow( row_position );
                count = 0;
            }
            // Convert value to object
            QJsonObject jsonObject = value.toObject();
            QString colorSpace = jsonObject.value("ColorSpace").toString();
            QString name = jsonObject.value("Name").toString();
            QJsonArray rgbPointsArray = jsonObject.value("RGBPoints").toArray();

            float min_value = rgbPointsArray.at(0).toDouble();
            float max_value = rgbPointsArray.at(rgbPointsArray.size() - 4).toDouble();

            if( colorSpace == "Diverging" )
            {
                if( rgbPointsArray.size() / 4 == 3 )
                {
                    int r1 = rgbPointsArray.at( 1 ).toDouble() * 255;
                    int g1 = rgbPointsArray.at( 2 ).toDouble() * 255;
                    int b1 = rgbPointsArray.at( 3 ).toDouble() * 255;

                    int r2 = rgbPointsArray.at( rgbPointsArray.size() - 3 ).toDouble() * 255;
                    int g2 = rgbPointsArray.at( rgbPointsArray.size() - 2 ).toDouble() * 255;
                    int b2 = rgbPointsArray.at( rgbPointsArray.size() - 1 ).toDouble() * 255;

                    color_map = kvs::DivergingColorMap::Create( kvs::RGBColor( r1, g1, b1 ), kvs::RGBColor( r2, g2, b2 ), 256 );
                }
                else
                {
                    color_map = kvs::ColorMap( 256, min_value, max_value );
                    for (int i = 0; i < rgbPointsArray.size(); i += 4)
                    {
                        float value = rgbPointsArray.at(i).toDouble();
                        int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                        int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                        int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                        color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                    }
                    color_map.create();
                }
            }
            else if( colorSpace == "Lab" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "RGB" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "CIELAB" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "HSV" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                if( name == "Blue to Red Rainbow" )
                {
                    color_map.addPoint(0/4.0, kvs::RGBColor( 0, 0, 255 ));
                    color_map.addPoint(1/4.0, kvs::RGBColor( 0, 255, 255 ));
                    color_map.addPoint(2/4.0, kvs::RGBColor( 0, 255, 0 ));
                    color_map.addPoint(3/4.0, kvs::RGBColor( 255, 255, 0 ));
                    color_map.addPoint(4/4.0, kvs::RGBColor( 255, 0, 0 ));
                }
                else
                {
                    for (int i = 0; i < rgbPointsArray.size(); i += 4)
                    {
                        float value = rgbPointsArray.at(i).toDouble();
                        int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                        int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                        int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                        color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                    }
                }
                color_map.create();
            }
            else if( colorSpace == "Step" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for ( int i = 0; i < rgbPointsArray.size(); i += 4 )
                {
                    float value1 = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    if( i < 8 )
                    {
                        color_map.addPoint( value1, kvs::RGBColor( r, g, b ) );
                    }
                    else
                    {
                        float value2 = rgbPointsArray.at(i - 4).toDouble();
                        color_map.addPoint( value2, kvs::RGBColor( r, g, b ) );
                        color_map.addPoint( value1, kvs::RGBColor( r, g, b ) );
                    }
                }
                color_map.create();
            }


            TFEColorMapBar* colorMapBar = new TFEColorMapBar();
            colorMapBar->setColorMap( color_map );
            QLabel* label = new QLabel( name );

            QHBoxLayout* layout = new QHBoxLayout();
            layout->addWidget(colorMapBar);
            layout->addWidget(label);

            QWidget* containerWidget = new QWidget();
            containerWidget->setLayout(layout);
            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount() - 1, count, containerWidget);
            count++;
        }
    }
}

ColorMapEditor::~ColorMapEditor()
{
    delete ui;
}
