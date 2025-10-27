#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"

// FIXME:存在しない派生量qNを指定した場合サーバがクラッシュする不具合があるはずです。
TransferFunctionEditor::TransferFunctionEditor( WebSocketPair* websockets, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::TransferFunctionEditor )
    , m_web_sockets( websockets )
    , m_model( new QStandardItemModel( this ) )
{
    initialize();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::initialize()
{
    ui->setupUi( this );
    ui->numberOfTransferFunctionLabelSpinBox->setMinimum( 0 );
    ui->numberOfTransferFunctionLabelSpinBox->setMaximum( 16 ); // FIXME:旧サーバ(WebSocket対応前のサーバでは伝達関数の最大値が17を超えるとクラッシュするという不具合があったはずです)

    reset();

    connect( ui->numberOfTransferFunctionLabelSpinBox   , &QSpinBox::valueChanged           , this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );

    connect( ui->colorSynthesizerLineEdit               , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorSynthesizerChanged );
    connect( ui->colorFunctionComboBox                  , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::onColorFunctionChanged );
    connect( ui->colorFunctionVariableLineEdit          , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorFunctionVariableChanged );
    connect( ui->colorFunctionPushButton                , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditColorVariable );
    connect( ui->colorUserDefinedMinMaxRadioButton      , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorUserDefinedMinMaxClicked );
    connect( ui->colorUserDefinedMinSpinBox             , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMinChanged );
    connect( ui->colorUserDefinedMaxSpinBox             , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMaxChanged );
    connect( ui->colorServerSideMinMaxRadioButton       , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorServerSideMinMaxClicked );
    connect( ui->editColorMapPushbutton                 , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditColorMap );

    connect( ui->opacitySynthesizerLineEdit             , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacitySynthesizerChanged );
    connect( ui->opacityFunctionComboBox                , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::onOpacityFunctionChanged );
    connect( ui->opacityFunctionVariableLineEdit        , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacityFunctionVariableChanged );
    connect( ui->opacityFunctionPushButton              , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditOpacityVariable );
    connect( ui->opacityUserDefinedMinMaxRadioButton    , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityUserDefinedMinMaxClicked );
    connect( ui->opacityUserDefinedMinSpinBox           , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMinChanged );
    connect( ui->opacityUserDefinedMaxSpinBox           , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMaxChanged );
    connect( ui->opacityServerSideMinMaxRadioButton     , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityServerSideMinMaxClicked );
    connect( ui->editOpacityMapPushbutton               , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditOpacityMap );

    connect( ui->exportPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onExport );
    connect( ui->importPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onImport );
    connect( ui->applyPushButton                        , &QPushButton::clicked             , this, &TransferFunctionEditor::onApply );
}

void TransferFunctionEditor::exportTransferFunction()
{
    if ( !m_model ) return;

    // ファイル名をユーザーに指定させる
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr( "Transfer Function をエクスポート" ),
        "",
        tr( "Transfer Function Export (*.tfe);;All Files (*)" )
        );
    if ( filePath.isEmpty() ) return;

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        qWarning() << "ファイルを開けませんでした:" << filePath;
        return;
    }

    QTextStream out( &file );

    // 総数と合成式などヘッダー部分
    int tf_count = m_model->rowCount();
    out << "TF_NUMBER=" << tf_count << "\n";
    out << "TF_SYNTH_C=" << m_color_synthesizer << "\n";
    out << "TF_SYNTH_O=" << m_opacity_synthesizer << "\n";

    // 各 Transfer Function 行を出力
    for ( int i = 0; i < tf_count; ++i )
    {
        QStandardItem* item = m_model->item( i );
        if ( !item ) continue;

        QString tagBase = QString( "TF_NAME%1_" ).arg( i + 1 );

        auto writeIfValid = [&]( const QString& key, int role )
        {
            QVariant val = item->data( role );
            if ( val.isValid() )
            {
                out << tagBase << key << "=" << val.toString() << "\n";
            }
        };

        // --- Color 関連出力 ---
        writeIfValid( "C", TransferFunctionItem::ColorFunction );
        writeIfValid( "VAR_C", TransferFunctionItem::ColorVariable );
        writeIfValid( "MIN_C", TransferFunctionItem::ColorUserRangeMin );
        writeIfValid( "MAX_C", TransferFunctionItem::ColorUserRangeMax );

        QVariant colorMapVar = item->data( TransferFunctionItem::ColorMap );
        if ( colorMapVar.isValid() )
        {
            QVector<QColor> colors = colorMapVar.value<QVector<QColor>>();
            QStringList colorStrings;
            for ( const QColor& c : colors )
            {
                colorStrings << QString::number( c.red() )
                             << QString::number( c.green() )
                             << QString::number( c.blue() );
            }
            out << tagBase << "TABLE_C=" << colorStrings.join( "," ) << "\n";
        }

        // --- Opacity 関連出力 ---
        writeIfValid( "O", TransferFunctionItem::OpacityFunction );
        writeIfValid( "VAR_O", TransferFunctionItem::OpacityVariable );
        writeIfValid( "MIN_O", TransferFunctionItem::OpacityUserRangeMin );
        writeIfValid( "MAX_O", TransferFunctionItem::OpacityUserRangeMax );

        QVariant opacityMapVar = item->data( TransferFunctionItem::OpacityMap );
        if ( opacityMapVar.isValid() )
        {
            QVector<float> opacities = opacityMapVar.value<QVector<float>>();
            QStringList opacityStrings;
            for ( float v : opacities )
            {
                opacityStrings << QString::number( v );
            }
            out << tagBase << "TABLE_O=" << opacityStrings.join( "," ) << "\n";
        }
    }

    file.close();
}

void TransferFunctionEditor::importTransferFunction()
{
    QFileDialog fileDialog( this, tr( "Import Transfer Function File" ), ".", tr( "Transfer Function Files (*.tfe *.TFE *.tf *.TF )" ) );
    fileDialog.setFileMode( QFileDialog::ExistingFile );

    if( fileDialog.exec() )
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        QString selectedFile = selectedFiles.first();

        QFile file( selectedFile );
        if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            return;
        }

        QTextStream in( &file );

        int tfCount = 0;

        // --- ここに keyHandlers を定義 ---
        QMap<QString, std::function<void( const QString& ) >> keyHandlers =
            {
                { "TF_NUMBER", [this, &tfCount]( const QString& value ) {
                     tfCount = value.toInt();
                     ui->numberOfTransferFunctionLabelSpinBox->setValue( tfCount );
                 }},
                { "TF_SYNTH_C", [this]( const QString& value ) {
                     m_color_synthesizer = value;
                     ui->colorSynthesizerLineEdit->setText( value );
                 }},
                { "TF_SYNTH_O", [this]( const QString& value ) {
                     m_opacity_synthesizer = value;
                     ui->opacitySynthesizerLineEdit->setText( value );
                 }}
            };

        QString line;
        QRegularExpression tfRegex( R"(TF_NAME(\d+)_([A-Z]+)_([CO]))" );

        while( !in.atEnd() )
        {
            line = in.readLine().trimmed();
            if( !line.contains( '=' ) ) continue;

            QString key = line.section( '=', 0, 0 );
            QString value = line.section( '=', 1 );

            // --- 固定キー処理 ---
            if( keyHandlers.contains( key ) )
            {
                keyHandlers[key]( value );
                continue;
            }

            // --- TF_NAME{n}_KEY_{C|O} 処理 ---
            QRegularExpressionMatch match = tfRegex.match( key );
            if( match.hasMatch() )
            {
                int index = match.captured( 1 ).toInt() - 1;
                QString field = match.captured( 2 );
                QString type = match.captured( 3 ); // "C" または "O"

                if( index < 0 || index >= m_model->rowCount() ) continue;
                QStandardItem* item = m_model->item( index );
                if( !item ) continue;

                // --- カラー／不透明度の各種フィールドに対応 ---
                if( type == "C" )
                {
                    if( field == "C" )
                    {
                        item->setData( value, TransferFunctionItem::ColorFunction );
                    }
                    else if( field == "VAR" )
                    {
                        item->setData( value, TransferFunctionItem::ColorVariable );
                    }
                    else if( field == "MIN" )
                    {
                        item->setData( value.toDouble(), TransferFunctionItem::ColorUserRangeMin );
                    }
                    else if( field == "MAX" )
                    {
                        item->setData( value.toDouble(), TransferFunctionItem::ColorUserRangeMax );
                    }
                    else if( field == "TABLE" )
                    {
                        QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                        QVector<QColor> colors;
                        for( int j = 0; j + 2 < strValues.size(); j += 3 )
                        {
                            colors.append( QColor(strValues[j].toInt(), strValues[j+1].toInt(), strValues[j+2].toInt()) );
                        }
                        item->setData( QVariant::fromValue(colors), TransferFunctionItem::ColorMap );
                    }
                }
                else if( type == "O" )
                {
                    if( field == "O" )
                    {
                        item->setData( value, TransferFunctionItem::OpacityFunction );
                    }
                    else if( field == "VAR" )
                    {
                        item->setData( value, TransferFunctionItem::OpacityVariable );
                    }
                    else if( field == "MIN" )
                    {
                        item->setData( value.toDouble(), TransferFunctionItem::OpacityUserRangeMin );
                    }
                    else if( field == "MAX" )
                    {
                        item->setData( value.toDouble(), TransferFunctionItem::OpacityUserRangeMax );
                    }
                    else if( field == "TABLE" )
                    {
                        QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                        QVector<float> opacities;
                        for( const auto& s : strValues )
                        {
                            opacities.append( s.toFloat() );
                        }
                        item->setData( QVariant::fromValue(opacities), TransferFunctionItem::OpacityMap );
                    }
                }
                item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryColorRangeMode );
                item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryOpacityRangeMode );
            }
        }
        onColorFunctionChanged();
        onOpacityFunctionChanged();
    }
}

#include <QDebug>

void TransferFunctionEditor::applyTransferFunction()
{
    if( !m_web_sockets->isConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    QJsonArray transferFunctionsArray;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        QStandardItem* item = m_model->item(row);
        if (!item) continue;

        // Temporary -> Current に適用
        QVariant tmpColor = item->data(TransferFunctionItem::TemporaryColorRangeMode);
        if (tmpColor.isValid())
            item->setData(tmpColor, TransferFunctionItem::CurrentColorRangeMode);

        QVariant tmpOpacity = item->data(TransferFunctionItem::TemporaryOpacityRangeMode);
        if (tmpOpacity.isValid())
            item->setData(tmpOpacity, TransferFunctionItem::CurrentOpacityRangeMode);

        QJsonObject tfObj;
        tfObj["ColorFunction"] = item->data(TransferFunctionItem::ColorFunction).toString();
        tfObj["ColorVariable"] = item->data(TransferFunctionItem::ColorVariable).toString();
        tfObj["TemporaryColorRangeMode"] = item->data(TransferFunctionItem::TemporaryColorRangeMode).toInt();
        tfObj["CurrentColorRangeMode"] = item->data(TransferFunctionItem::CurrentColorRangeMode).toInt();
        tfObj["ResultColorRangeMode"] = item->data(TransferFunctionItem::ResultColorRangeMode).toInt();
        tfObj["ColorUserRangeMin"] = item->data(TransferFunctionItem::ColorUserRangeMin).toDouble();
        tfObj["ColorUserRangeMax"] = item->data(TransferFunctionItem::ColorUserRangeMax).toDouble();
        tfObj["ColorServerRangeMin"] = item->data(TransferFunctionItem::ColorServerRangeMin).toDouble();
        tfObj["ColorServerRangeMax"] = item->data(TransferFunctionItem::ColorServerRangeMax).toDouble();

        QVector<QColor> colors = item->data(TransferFunctionItem::ColorMap).value<QVector<QColor>>();
        QJsonArray colorArray;

        for (const QColor& c : colors)
        {
            QJsonArray rgb;
            rgb.append(c.red());
            rgb.append(c.green());
            rgb.append(c.blue());
            colorArray.append(rgb); // [[R,G,B], [R,G,B], ...]
        }

        tfObj["ColorMap"] = colorArray;

        std::vector<int> colorHist = item->data(TransferFunctionItem::ColorHistogram).value<std::vector<int>>();
        QJsonArray colorHistArray;
        for (int v : colorHist) colorHistArray.append(v);
        tfObj["ColorHistogram"] = colorHistArray;

        tfObj["OpacityFunction"] = item->data(TransferFunctionItem::OpacityFunction).toString();
        tfObj["OpacityVariable"] = item->data(TransferFunctionItem::OpacityVariable).toString();
        tfObj["TemporaryOpacityRangeMode"] = item->data(TransferFunctionItem::TemporaryOpacityRangeMode).toInt();
        tfObj["CurrentOpacityRangeMode"] = item->data(TransferFunctionItem::CurrentOpacityRangeMode).toInt();
        tfObj["ResultOpacityRangeMode"] = item->data(TransferFunctionItem::ResultOpacityRangeMode).toInt();
        tfObj["OpacityUserRangeMin"] = item->data(TransferFunctionItem::OpacityUserRangeMin).toDouble();
        tfObj["OpacityUserRangeMax"] = item->data(TransferFunctionItem::OpacityUserRangeMax).toDouble();
        tfObj["OpacityServerRangeMin"] = item->data(TransferFunctionItem::OpacityServerRangeMin).toDouble();
        tfObj["OpacityServerRangeMax"] = item->data(TransferFunctionItem::OpacityServerRangeMax).toDouble();

        QVector<float> opacities = item->data(TransferFunctionItem::OpacityMap).value<QVector<float>>();
        QJsonArray opacityArray;
        for (float f : opacities) opacityArray.append(f);
        tfObj["OpacityMap"] = opacityArray;

        std::vector<int> opacityHist = item->data(TransferFunctionItem::OpacityHistogram).value<std::vector<int>>();
        QJsonArray opacityHistArray;
        for (int v : opacityHist) opacityHistArray.append(v);
        tfObj["OpacityHistogram"] = opacityHistArray;

        transferFunctionsArray.append(tfObj);
    }

    // ルート JSON に synthesize 情報も追加
    QJsonObject root;
    root["event"] = "transferfunction";
    root["color_synthesizer"] = m_color_synthesizer;
    root["opacity_synthesizer"] = m_opacity_synthesizer;
    root["data"] = transferFunctionsArray;

    m_web_sockets->text()->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}


void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged(int numberOfTransferFunction)
{
    if (!m_model) return;

    TransferFunctionItem itemCreator;
    const int currentCount = m_model->rowCount();

    // --- モデル行数の増減 ---
    if (numberOfTransferFunction > currentCount)
    {
        for (int i = currentCount; i < numberOfTransferFunction; ++i)
        {
            auto items = itemCreator.createItem(i + 1);
            if (!items.isEmpty()) m_model->appendRow(items);
        }
    }
    else if (numberOfTransferFunction < currentCount)
    {
        m_model->removeRows(numberOfTransferFunction, currentCount - numberOfTransferFunction);
    }

    // --- ComboBox 更新用ラムダ ---
    auto syncComboBox = [this, numberOfTransferFunction](QComboBox* combo, int role)
    {
        while (combo->count() > numberOfTransferFunction)
            combo->removeItem(combo->count() - 1);

        for (int i = combo->count(); i < numberOfTransferFunction; ++i)
        {
            if (auto item = m_model->item(i))
            {
                const QVariant data = item->data(role);
                combo->addItem(data.toString(), data);
            }
        }
    };

    syncComboBox(ui->colorFunctionComboBox, TransferFunctionItem::ColorFunction);
    syncComboBox(ui->opacityFunctionComboBox, TransferFunctionItem::OpacityFunction);

    if( numberOfTransferFunction == 0 ) reset();
}

void TransferFunctionEditor::onColorSynthesizerChanged( const QString &colorSynthesizer )
{
    m_color_synthesizer = colorSynthesizer;
}

void TransferFunctionEditor::onColorFunctionChanged()
{
    if (!m_model) return;

    int index = ui->colorFunctionComboBox->currentIndex();
    if (index < 0 || index >= m_model->rowCount()) return;

    QStandardItem* item = m_model->item(index);
    if (!item) return;

    // UI 更新
    ui->colorFunctionVariableLineEdit->setText(item->data(TransferFunctionItem::ColorVariable).toString());

    int tempMode = item->data(TransferFunctionItem::TemporaryColorRangeMode).toInt();
    ui->colorUserDefinedMinMaxRadioButton->setChecked(tempMode == TransferFunctionItem::UserRange);
    ui->colorServerSideMinMaxRadioButton->setChecked(tempMode == TransferFunctionItem::ServerRange);

    ui->colorUserDefinedMinSpinBox->setValue(item->data(TransferFunctionItem::ColorUserRangeMin).toDouble());
    ui->colorUserDefinedMaxSpinBox->setValue(item->data(TransferFunctionItem::ColorUserRangeMax).toDouble());
    ui->colorServerSideMinSpinBox->setValue(item->data(TransferFunctionItem::ColorServerRangeMin).toDouble());
    ui->colorServerSideMaxSpinBox->setValue(item->data(TransferFunctionItem::ColorServerRangeMax).toDouble());

    // カラーマップとヒストグラム
    QVector<QColor> colors = item->data(TransferFunctionItem::ColorMap).value<QVector<QColor>>();
    ui->colorMapBar->setColors(colors);

    std::vector<int> hist = item->data(TransferFunctionItem::ColorHistogram).value<std::vector<int>>();
    ui->colorHistogram->setDatas(hist);
    ui->colorHistogram->update();
}

void TransferFunctionEditor::onColorFunctionVariableChanged( const QString &colorFunctionVariable )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorFunctionVariable, TransferFunctionItem::ColorVariable );
}

void TransferFunctionEditor::onEditColorVariable()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if (!m_model || index < 0 || index >= m_model->rowCount()) return;

    QStandardItem* item = m_model->item(index);
    if (!item) return;

    // 後付けでタイプとモデルをセット
    m_variable_editor.setType(VariableEditor::COLOR);
    m_variable_editor.setModel(*m_model);
    m_variable_editor.initialize();

    m_variable_editor.adjustSize();

    if (m_variable_editor.exec() == QDialog::Accepted)
    {
        const QStandardItemModel& editedModel = m_variable_editor.editedModel();

        for (int row = 0; row < m_model->rowCount() && row < editedModel.rowCount(); ++row)
        {
            QStandardItem* item = m_model->item(row);
            const QStandardItem* editedVarItem = editedModel.item(row, 1); // 2列目が variable

            if (editedVarItem)
            {
                item->setData(editedVarItem->text(), TransferFunctionItem::ColorVariable);
            }
        }
        onColorFunctionChanged();
    }
}

void TransferFunctionEditor::onColorUserDefinedMinMaxClicked()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->colorUserDefinedMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::UserRange, TransferFunctionItem::TemporaryColorRangeMode );
    }
}

void TransferFunctionEditor::onColorUserDefinedMinChanged( const float &colorUserDefinedMin )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorUserDefinedMin, TransferFunctionItem::ColorUserRangeMin );
}

void TransferFunctionEditor::onColorUserDefinedMaxChanged( const float &colorUserDefinedMax )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorUserDefinedMax, TransferFunctionItem::ColorUserRangeMax );
}

void TransferFunctionEditor::onColorServerSideMinMaxClicked()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->colorServerSideMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::ServerRange, TransferFunctionItem::TemporaryColorRangeMode );
    }
}

void TransferFunctionEditor::onEditColorMap()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    m_color_map_editor.adjustSize();
    m_color_map_editor.setDefaultColorMap( ui->colorMapBar->getColors() );

    if( m_color_map_editor.exec() == QDialog::Accepted )
    {
        QVector<QColor> qcolors = m_color_map_editor.getColorMap();
        item->setData( QVariant::fromValue( qcolors ), TransferFunctionItem::ColorMap );
        onColorFunctionChanged();
    }
}

void TransferFunctionEditor::onOpacitySynthesizerChanged( const QString &opacitySynthesizer )
{
    m_opacity_synthesizer = opacitySynthesizer;
}

void TransferFunctionEditor::onOpacityFunctionChanged()
{
    if (!m_model) return;

    int index = ui->opacityFunctionComboBox->currentIndex();
    if (index < 0 || index >= m_model->rowCount()) return;

    QStandardItem* item = m_model->item(index);
    if (!item) return;

    // UI 更新
    ui->opacityFunctionVariableLineEdit->setText(item->data(TransferFunctionItem::OpacityVariable).toString());

    int tempMode = item->data(TransferFunctionItem::TemporaryOpacityRangeMode).toInt();
    ui->opacityUserDefinedMinMaxRadioButton->setChecked(tempMode == TransferFunctionItem::UserRange);
    ui->opacityServerSideMinMaxRadioButton->setChecked(tempMode == TransferFunctionItem::ServerRange);

    ui->opacityUserDefinedMinSpinBox->setValue(item->data(TransferFunctionItem::OpacityUserRangeMin).toDouble());
    ui->opacityUserDefinedMaxSpinBox->setValue(item->data(TransferFunctionItem::OpacityUserRangeMax).toDouble());
    ui->opacityServerSideMinSpinBox->setValue(item->data(TransferFunctionItem::OpacityServerRangeMin).toDouble());
    ui->opacityServerSideMaxSpinBox->setValue(item->data(TransferFunctionItem::OpacityServerRangeMax).toDouble());

    // 不透明度マップとヒストグラム
    QVector<float> opacities = item->data(TransferFunctionItem::OpacityMap).value<QVector<float>>();
    ui->opacityMapBar->setOpacities(opacities);

    std::vector<int> hist = item->data(TransferFunctionItem::OpacityHistogram).value<std::vector<int>>();
    ui->opacityHistogram->setDatas(hist);
    ui->opacityHistogram->update();
}

void TransferFunctionEditor::onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( opacityFunctionVariable, TransferFunctionItem::OpacityVariable );
}

void TransferFunctionEditor::onEditOpacityVariable()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if (!m_model || index < 0 || index >= m_model->rowCount()) return;

    QStandardItem* item = m_model->item(index);
    if (!item) return;

    // 後付けでタイプとモデルをセット
    m_variable_editor.setType(VariableEditor::OPACITY);
    m_variable_editor.setModel(*m_model);
    m_variable_editor.initialize();

    m_variable_editor.adjustSize();

    if (m_variable_editor.exec() == QDialog::Accepted)
    {
        const QStandardItemModel& editedModel = m_variable_editor.editedModel();

        for (int row = 0; row < m_model->rowCount() && row < editedModel.rowCount(); ++row)
        {
            QStandardItem* item = m_model->item(row);
            const QStandardItem* editedVarItem = editedModel.item(row, 1); // 2列目が variable

            if (editedVarItem)
            {
                item->setData(editedVarItem->text(), TransferFunctionItem::OpacityVariable);
            }
        }
        onOpacityFunctionChanged();
    }
}

void TransferFunctionEditor::onOpacityUserDefinedMinMaxClicked()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->opacityUserDefinedMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::UserRange, TransferFunctionItem::TemporaryOpacityRangeMode );
    }
}

void TransferFunctionEditor::onOpacityUserDefinedMinChanged( const float &OpacityUserDefinedMin )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( OpacityUserDefinedMin, TransferFunctionItem::OpacityUserRangeMin );
}

void TransferFunctionEditor::onOpacityUserDefinedMaxChanged( const float &OpacityUserDefinedMax )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( OpacityUserDefinedMax, TransferFunctionItem::OpacityUserRangeMax );
}

void TransferFunctionEditor::onOpacityServerSideMinMaxClicked()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->opacityServerSideMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::ServerRange, TransferFunctionItem::TemporaryOpacityRangeMode );
    }
}

void TransferFunctionEditor::onEditOpacityMap()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    m_opacity_map_editor.adjustSize();
    m_opacity_map_editor.setDefaultOpacityMap( ui->opacityMapBar->getOpacities() );

    if( m_opacity_map_editor.exec() == QDialog::Accepted )
    {
        QVector<float> opacities = m_opacity_map_editor.getOpacityMap();
        item->setData( QVariant::fromValue( opacities ), TransferFunctionItem::OpacityMap );
        onOpacityFunctionChanged();
    }
}

void TransferFunctionEditor::onExport()
{
    exportTransferFunction();
}

void TransferFunctionEditor::onImport()
{
    importTransferFunction();
}

void TransferFunctionEditor::onApply()
{
    applyTransferFunction();
}

void TransferFunctionEditor::updateOperatorState( bool operatorState )
{
    if( m_variable_editor.isVisible() ) m_variable_editor.close();
    if( m_color_map_editor.isVisible() ) m_color_map_editor.close();
    if( m_opacity_map_editor.isVisible() ) m_opacity_map_editor.close();

    ui->numberOfTransferFunctionLabelSpinBox->setEnabled( operatorState );

    ui->colorSynthesizerLineEdit->setEnabled( operatorState );
    ui->colorFunctionVariableLineEdit->setEnabled( operatorState );
    ui->colorFunctionPushButton->setEnabled( operatorState );
    ui->colorUserDefinedMinMaxRadioButton->setEnabled( operatorState );
    ui->colorUserDefinedMinSpinBox->setEnabled( operatorState );
    ui->colorUserDefinedMaxSpinBox->setEnabled( operatorState );
    ui->colorServerSideMinMaxRadioButton->setEnabled( operatorState );
    ui->editColorMapPushbutton->setEnabled( operatorState );

    ui->opacitySynthesizerLineEdit->setEnabled( operatorState );
    ui->opacityFunctionVariableLineEdit->setEnabled( operatorState );
    ui->opacityFunctionPushButton->setEnabled( operatorState );
    ui->opacityUserDefinedMinMaxRadioButton->setEnabled( operatorState );
    ui->opacityUserDefinedMinSpinBox->setEnabled( operatorState );
    ui->opacityUserDefinedMaxSpinBox->setEnabled( operatorState );
    ui->opacityServerSideMinMaxRadioButton->setEnabled( operatorState );
    ui->editOpacityMapPushbutton->setEnabled( operatorState );

    ui->exportPushButton->setEnabled( operatorState );
    ui->importPushButton->setEnabled( operatorState );
    ui->applyPushButton->setEnabled( operatorState );
}

void TransferFunctionEditor::reset()
{
    ui->numberOfTransferFunctionLabelSpinBox->setValue( 0 );
    QVector<QColor> initialColors( 256, QColor( 0, 0, 0 ) );
    ui->colorMapBar->setColors( initialColors );
    QVector<float> initialOpacity( 256, 0.0f );
    ui->opacityMapBar->setOpacities( initialOpacity );
    std::vector<int> initialHistogram( 256, 0 );
    ui->colorHistogram->setDatas( initialHistogram );
    ui->opacityHistogram->setDatas( initialHistogram );
}

void TransferFunctionEditor::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TransferFunctionEditor::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}
