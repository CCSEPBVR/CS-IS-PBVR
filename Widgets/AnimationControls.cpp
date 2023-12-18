#include "AnimationControls.h"
#include "ui_AnimationControls.h"
#include <kvs/ObjectManager>
#include <QFileDialog>

AnimationControls::AnimationControls(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AnimationControls)
{
    ui->setupUi(this);
    m_interpolation_counter = 0;
    m_xform_index = 0;
    m_animationTimer = new QTimer(this);
    m_animation_paused = false;

    ui->captureCBox->addItem( "On", QVariant( true ) );
    ui->captureCBox->addItem( "Off", QVariant( false ) );
    ui->captureCBox->setCurrentIndex( 1 );

    connect( ui->interpolationSBox, &QSpinBox::valueChanged, this, &AnimationControls::onInterpolationValueChanged);
}

AnimationControls::~AnimationControls()
{
    delete ui;
}

void AnimationControls::addKeyFrameAdd( kvs::Xform xform )
{
    m_xforms.push_back( xform );

    ui->totalKeyFramesValueLbl->setText( QString::number( m_xforms.size() ) );
    if( m_xforms.size() < 2 == false)
    {
        ui->totalAnimationFramesValueLbl->setText( QString::number( ( m_xforms.size() - 1 ) * ( ui->interpolationSBox->value() + 1 ) ) );
    }
}

void AnimationControls::removeLasrKeyFrame()
{
    if( m_xforms.size() >= 1 )
    {
        m_xforms.pop_back();
    }

    ui->totalKeyFramesValueLbl->setText( QString::number( m_xforms.size() ) );
    if( m_xforms.size() < 2 == false)
    {
        ui->totalAnimationFramesValueLbl->setText( QString::number( ( m_xforms.size() - 1 ) * ( ui->interpolationSBox->value() + 1 ) ) );
    }
    else
    {
        ui->totalAnimationFramesValueLbl->setText("0");
    }
}

void AnimationControls::clearKeyFrame()
{
    m_xforms.clear();
    ui->totalKeyFramesValueLbl->setText( QString::number( m_xforms.size() ) );

    ui->totalAnimationFramesValueLbl->setText("0");
}

void AnimationControls::playKeyFrame()
{
    // キーフレームの数を取得
    const int num_frames = m_xforms.size();

    // 補間ステップ数をUIから取得
    const int interp_steps = ui->interpolationSBox->value();

    // キーフレームが不足している場合、警告をログに出力して関数を終了
    if (num_frames < 2) {
        qWarning() << "Insufficient keyframes for animation.";
        return;
    }

    // アニメーションが既に再生中であれば一時停止フラグをトグルし、タイマーを停止する
    if (m_animationTimer->isActive())
    {
        m_animation_paused = !m_animation_paused;
        if (m_animation_paused)
        {
            m_animationTimer->stop();
            return;
        }
    }

    int loop_counter = 0;

    // 各キーフレーム間で補間を行う
    for (int i = 0; i < num_frames - 1; i++)
    {
        // 各キーフレームの間を指定されたステップ数で補間
        for (int step = 0; step <= interp_steps; step++)
        {
            // キーフレーム i と i+1 の間を補間して新しい変換行列を取得
            kvs::Xform Xform_new = InterpolateXform(step, interp_steps, m_xforms[i], m_xforms[i + 1]);

            // オブジェクトマネージャーに新しい変換を適用
            m_screen->scene()->reset();
            m_screen->scene()->objectManager()->translate(Xform_new.translation());
            m_screen->scene()->objectManager()->scale(Xform_new.scaling());
            m_screen->scene()->objectManager()->rotate(Xform_new.rotation());

            // 画面を更新
            m_screen->update();
            if( ui->captureCBox->currentData().toBool() == true )
            {
                screenShot( loop_counter );
            }
            loop_counter++;

            // アニメーション速度を調整するための遅延を追加（オプション）
            QCoreApplication::processEvents();
            m_animationTimer->start(1000);

            // アニメーションが一時停止されている場合はここでループを抜ける
            if (m_animation_paused)
            {
                return;
            }

        }
    }

    // アニメーションが終了した場合はタイマーを停止
    m_animationTimer->stop();
}


void AnimationControls::loadKeyFrameFile()
{
    clearKeyFrame();

    QString file_name = QFileDialog::getOpenFileName(this, "Load Keyframes", QDir::currentPath(), "Binary Files (*.bin)");
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open file for reading.";
        return;
    }

    QDataStream in(&file);
    while (!in.atEnd())
    {
        float translation_x, translation_y, translation_z;
        float scaling_x, scaling_y, scaling_z;
        float rotation_0, rotation_1, rotation_2,
            rotation_3, rotation_4, rotation_5,
            rotation_6, rotation_7, rotation_8;

        in >> translation_x >> translation_y >> translation_z
            >> scaling_x >> scaling_y >> scaling_z
            >> rotation_0 >> rotation_1 >> rotation_2
            >> rotation_3 >> rotation_4 >> rotation_5
            >> rotation_6 >> rotation_7 >> rotation_8;

        kvs::Xform xform(
            kvs::Vec3(translation_x, translation_y, translation_z),
            kvs::Vec3(scaling_x, scaling_y, scaling_z),
            kvs::Mat3(
                rotation_0, rotation_1, rotation_2,
                rotation_3, rotation_4, rotation_5,
                rotation_6, rotation_7, rotation_8));

        addKeyFrameAdd(xform);
    }

    file.close();
}


void AnimationControls::saveKeyFrameFile()
{
    QString file_name = QFileDialog::getSaveFileName(this, "Save Keyframes", QDir::currentPath(), "Binary Files (*.bin)");
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not open file for writing.";
        return;
    }

    QDataStream out(&file);
    for (const kvs::Xform& xform : m_xforms)
    {
        out << xform.translation().x()
            << xform.translation().y()
            << xform.translation().z()
            << xform.scaling().x()
            << xform.scaling().y()
            << xform.scaling().z()
            << xform.rotation()[0].x()
            << xform.rotation()[0].y()
            << xform.rotation()[0].z()
            << xform.rotation()[1].x()
            << xform.rotation()[1].y()
            << xform.rotation()[1].z()
            << xform.rotation()[2].x()
            << xform.rotation()[2].y()
            << xform.rotation()[2].z();
    }

    file.close();
}

void AnimationControls::screenShot(int loop_counter)
{
    QString frame_number = QString::asprintf("%06d", loop_counter + 1);
    QString file_name = ui->imageFileLEdit->text();

    QImage image = m_screen->grabFramebuffer();

    // ファイル名を作成して保存
    QString full_file_name = file_name + "_" + frame_number + ".bmp";
    image.save(full_file_name);
}


kvs::Xform AnimationControls::InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end )
{
    // range of the interpolation parametar t = [0,1].
    float delta = 1.0 / ( float )num_frame;
    float t = delta * ( float )interp_step;

    kvs::Matrix33f rotation_0( start.rotation() );
    kvs::Vector3f translation_0( start.translation() );
    kvs::Vector3f scaling_0( start.scaling() );

    kvs::Matrix33f rotation_1( end.rotation() );
    kvs::Vector3f translation_1( end.translation() );
    kvs::Vector3f scaling_1( end.scaling() );

    //KVS2.7.0
    //MOD BY)T0603 2020.06.04
    //kvs::Quaternion<float> q_0 = RtoQ( rotation_0 );
    //kvs::Quaternion<float> q_1 = RtoQ( rotation_1 );
    //kvs::Quaternion<float> q =
    //      kvs::Quaternion<float>::sphericalLinearInterpolation( q_0, q_1, t, true, true );

    kvs::Quaternion q_0 = RtoQ( rotation_0 );
    kvs::Quaternion q_1 = RtoQ( rotation_1 );
    kvs::Quaternion q =
        kvs::Quaternion::SphericalLinearInterpolation( q_0, q_1, t, true, true );

    kvs::Matrix33f rotation = q.toMatrix();

    kvs::Vector3f translation = translation_1 * t + translation_0 * ( 1 - t );

    kvs::Vector3f scaling = scaling_1 * t + scaling_0 * ( 1 - t );

    kvs::Xform xform( translation, scaling, rotation );
    return xform;
}

kvs::Quaternion AnimationControls::RtoQ( const kvs::Matrix33f& R )
{
    float r11 = R[0][0];
    float r12 = R[0][1];
    float r13 = R[0][2];
    float r21 = R[1][0];
    float r22 = R[1][1];
    float r23 = R[1][2];
    float r31 = R[2][0];
    float r32 = R[2][1];
    float r33 = R[2][2];

    float q0 = ( r11 + r22 + r33 + 1.0f ) / 4.0f;
    float q1 = ( r11 - r22 - r33 + 1.0f ) / 4.0f;
    float q2 = ( -r11 + r22 - r33 + 1.0f ) / 4.0f;
    float q3 = ( -r11 - r22 + r33 + 1.0f ) / 4.0f;
    if ( q0 < 0.0f ) q0 = 0.0f;
    if ( q1 < 0.0f ) q1 = 0.0f;
    if ( q2 < 0.0f ) q2 = 0.0f;
    if ( q3 < 0.0f ) q3 = 0.0f;
    q0 = sqrt( q0 );
    q1 = sqrt( q1 );
    q2 = sqrt( q2 );
    q3 = sqrt( q3 );
    if ( q0 >= q1 && q0 >= q2 && q0 >= q3 )
    {
        q0 *= +1.0f;
        q1 *= Sign( r32 - r23 );
        q2 *= Sign( r13 - r31 );
        q3 *= Sign( r21 - r12 );
    }
    else if ( q1 >= q0 && q1 >= q2 && q1 >= q3 )
    {
        q0 *= Sign( r32 - r23 );
        q1 *= +1.0f;
        q2 *= Sign( r21 + r12 );
        q3 *= Sign( r13 + r31 );
    }
    else if ( q2 >= q0 && q2 >= q1 && q2 >= q3 )
    {
        q0 *= Sign( r13 - r31 );
        q1 *= Sign( r21 + r12 );
        q2 *= +1.0f;
        q3 *= Sign( r32 + r23 );
    }
    else if ( q3 >= q0 && q3 >= q1 && q3 >= q2 )
    {
        q0 *= Sign( r21 - r12 );
        q1 *= Sign( r31 + r13 );
        q2 *= Sign( r32 + r23 );
        q3 *= +1.0f;
    }
    else
    {
        printf( "coding error\n" );
    }
    float r = Norm( q0, q1, q2, q3 );
    q0 /= r;
    q1 /= r;
    q2 /= r;
    q3 /= r;

    //KVS2.7.0
    //MOD BY)T0603 2020.06.04
    //return kvs::Quaternion<float>( q1, q2, q3, q0 );
    return kvs::Quaternion( q1, q2, q3, q0 );
}

float AnimationControls::Sign( const float x )
{
    return ( x >= 0.0f ) ? +1.0f : -1.0f;
}

float AnimationControls::Norm( const float a, const float b, const float c, const float d )
{
    return sqrt( a * a + b * b + c * c + d * d );
}

void AnimationControls::onInterpolationValueChanged()
{
    if( m_xforms.size() < 2 == false)
    {
        ui->totalAnimationFramesValueLbl->setText( QString::number( ( m_xforms.size() - 1 ) * ( ui->interpolationSBox->value() + 1 ) ) );
    }
}
