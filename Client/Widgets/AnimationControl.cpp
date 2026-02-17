#include "AnimationControl.h"
#include "ui_AnimationControl.h"

#include <QFileDialog>
#include <QDir>
#include <QDataStream>
#include <QDebug>
#include <QCoreApplication>

AnimationControl::AnimationControl( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::AnimationControl )
    , m_screen( screen )
    , m_animation_timer( new QTimer( this ) )
    , m_animation_paused( false )
    , m_xforms()
    , m_xform_index( 0 )
    , m_interpolation_counter( 0 )
{
    ui->setupUi( this );

    ui->captureComboBox->addItem( "On", QVariant( true ) );
    ui->captureComboBox->addItem( "Off", QVariant( false ) );
    ui->captureComboBox->setCurrentIndex( 1 );

    connect( ui->interpolationSpinBox, &QSpinBox::valueChanged, this, &AnimationControl::onInterpolationValueChanged );
}

AnimationControl::~AnimationControl()
{
    delete ui;
}

static inline int CalcTotalAnimationFramesInclusiveEnds( int keyframes, int interp )
{
    if ( keyframes < 2 ) return 0;
    if ( interp <= 0 ) return 0;

    return 1 + ( keyframes - 1 ) * interp;
}

void AnimationControl::onAddKeyFrameAdd( kvs::Xform xform )
{
    m_xforms.push_back( xform );

    ui->totalKeyFramesDisplayLabel->setText( QString::number( m_xforms.size() ) );

    const int interp = ui->interpolationSpinBox->value();
    ui->totalAnimationFramesDisplayLabel->setText(
        QString::number( CalcTotalAnimationFramesInclusiveEnds( static_cast<int>( m_xforms.size() ), interp ) )
        );
}

void AnimationControl::onRemoveLastKeyFrame()
{
    if( m_xforms.size() >= 1 )
    {
        m_xforms.pop_back();
    }

    ui->totalKeyFramesDisplayLabel->setText( QString::number( m_xforms.size() ) );

    const int interp = ui->interpolationSpinBox->value();
    ui->totalAnimationFramesDisplayLabel->setText(
        QString::number( CalcTotalAnimationFramesInclusiveEnds( static_cast<int>( m_xforms.size() ), interp ) )
        );
}

void AnimationControl::onClearKeyFrame()
{
    m_xforms.clear();
    ui->totalKeyFramesDisplayLabel->setText( QString::number( m_xforms.size() ) );
    ui->totalAnimationFramesDisplayLabel->setText( "0" );
}

static inline void ApplyXform( kvs::qt::jaea::Screen* screen, const kvs::Xform& xf )
{
    screen->scene()->reset();
    screen->scene()->objectManager()->translate( xf.translation() );
    screen->scene()->objectManager()->scale( xf.scaling() );
    screen->scene()->objectManager()->rotate( xf.rotation() );
    screen->update();
}

void AnimationControl::onPlayKeyFrame()
{
    const int num_frames = static_cast<int>( m_xforms.size() );

    const int interp = ui->interpolationSpinBox->value();

    if( num_frames < 2 )
    {
        qWarning() << "Insufficient keyframes for animation.";
        return;
    }

    if( interp <= 0 )
    {
        qWarning() << "Interpolation must be > 0.";
        return;
    }

    if( m_animation_timer->isActive() )
    {
        m_animation_paused = !m_animation_paused;
        if( m_animation_paused )
        {
            m_animation_timer->stop();
            return;
        }
    }

    m_animation_paused = false;

    int loop_counter = 0;
    const bool capture = ui->captureComboBox->currentData().toBool();

    {
        const kvs::Xform& first = m_xforms.front();
        ApplyXform( m_screen, first );

        if( capture ) { onScreenShot( loop_counter ); }
        ++loop_counter;

        QCoreApplication::processEvents();
        m_animation_timer->start( 1000 );

        if( m_animation_paused ) { return; }
    }

    for( int i = 0; i < num_frames - 1; ++i )
    {
        const kvs::Xform& start = m_xforms[i];
        const kvs::Xform& end   = m_xforms[i + 1];

        for( int step = 1; step <= interp; ++step )
        {
            kvs::Xform xform_new = InterpolateXform( step, interp, start, end );

            ApplyXform( m_screen, xform_new );

            if( capture ) { onScreenShot( loop_counter ); }
            ++loop_counter;

            QCoreApplication::processEvents();
            m_animation_timer->start( 1000 );

            if( m_animation_paused ) { return; }
        }
    }

    m_animation_timer->stop();
}

void AnimationControl::onLoadKeyFrameFile()
{
    onClearKeyFrame();

    QString file_name = QFileDialog::getOpenFileName( this, "Load Keyframes", QDir::currentPath(), "Binary Files (*.anim)" );
    if( file_name.isEmpty() )
        return;

    QFile file( file_name );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        qWarning() << "Could not open file for reading.";
        return;
    }

    QDataStream in( &file );
    while( !in.atEnd() )
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
            kvs::Vec3( translation_x, translation_y, translation_z ),
            kvs::Vec3( scaling_x, scaling_y, scaling_z ),
            kvs::Mat3(
                rotation_0, rotation_1, rotation_2,
                rotation_3, rotation_4, rotation_5,
                rotation_6, rotation_7, rotation_8 ) );

        onAddKeyFrameAdd( xform );
    }

    file.close();
}

void AnimationControl::onSaveKeyFrameFile()
{
    QString file_name = QFileDialog::getSaveFileName( this, "Save Keyframes", QDir::currentPath(), "Binary Files (*.anim)" );

    if( !file_name.endsWith( ".anim", Qt::CaseInsensitive ) )
    {
        file_name += ".anim";
    }

    if( file_name.isEmpty() )
        return;

    QFile file(file_name);
    if( !file.open( QIODevice::WriteOnly ) )
    {
        qWarning() << "Could not open file for writing.";
        return;
    }

    QDataStream out( &file );
    for( const kvs::Xform& xform : m_xforms )
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

void AnimationControl::onScreenShot(int loopCounter)
{
    QString frame_number = QString::asprintf( "%06d", loopCounter + 1 );
    QString file_name = ui->imageFileLineEdit->text();

    QImage image = m_screen->grabFramebuffer();

    QString full_file_name = file_name + "_" + frame_number + ".bmp";
    image.save( full_file_name );
}

void AnimationControl::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void AnimationControl::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

kvs::Xform AnimationControl::InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end )
{
    float delta = 1.0f / ( float )num_frame;
    float t = delta * ( float )interp_step;

    kvs::Matrix33f rotation_0( start.rotation() );
    kvs::Vector3f translation_0( start.translation() );
    kvs::Vector3f scaling_0( start.scaling() );

    kvs::Matrix33f rotation_1( end.rotation() );
    kvs::Vector3f translation_1( end.translation() );
    kvs::Vector3f scaling_1( end.scaling() );

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

kvs::Quaternion AnimationControl::RtoQ( const kvs::Matrix33f& R )
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

    return kvs::Quaternion( q1, q2, q3, q0 );
}

float AnimationControl::Sign( const float x )
{
    return ( x >= 0.0f ) ? +1.0f : -1.0f;
}

float AnimationControl::Norm( const float a, const float b, const float c, const float d )
{
    return sqrt( a * a + b * b + c * c + d * d );
}

void AnimationControl::onInterpolationValueChanged()
{
    const int interp = ui->interpolationSpinBox->value();
    ui->totalAnimationFramesDisplayLabel->setText(
        QString::number( CalcTotalAnimationFramesInclusiveEnds( static_cast<int>( m_xforms.size() ), interp ) )
        );
}
