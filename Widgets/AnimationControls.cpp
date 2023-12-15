#include "AnimationControls.h"
#include "ui_AnimationControls.h"
#include <kvs/ObjectManager>

AnimationControls::AnimationControls(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AnimationControls)
{
    ui->setupUi(this);
    m_interpolation_counter = 0;
    m_xform_index = 0;
}

AnimationControls::~AnimationControls()
{
    delete ui;
}

void AnimationControls::InitializeKeyFrame()
{

}

void AnimationControls::addKeyFrameAdd( kvs::Xform xform )
{
    qInfo() << __func__;
    m_xforms.push_back( xform );
    qInfo() << m_xforms.size();
}

void AnimationControls::removeLasrKeyFrame()
{
    qInfo() << __func__;
    if( m_xforms.size() > 1 )
    {
        m_xforms.pop_back();
    }
    qInfo() << m_xforms.size();
}

void AnimationControls::clearKeyFrame()
{
    qInfo() << __func__;
    m_xforms.clear();
}

void AnimationControls::playKeyFrame()
{
    // 関数の開始をログに出力
    qInfo() << __func__;

    // キーフレームの数を取得
    const int num_frames = m_xforms.size();

    // 補間ステップ数をUIから取得
    const int interp_steps = ui->interpolationSBox->value();

    // キーフレームが不足している場合、警告をログに出力して関数を終了
    if (num_frames < 2) {
        qWarning() << "Insufficient keyframes for animation.";
        return;
    }

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

            // アニメーション速度を調整するための遅延を追加（オプション）
            QCoreApplication::processEvents();
            QThread::msleep(10);  // 適宜遅延を調整
        }
    }
}

void AnimationControls::loadKeyFrameFile()
{
    qInfo() << __func__;
}

void AnimationControls::saveKeyFrameFile()
{
    qInfo() << __func__;
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
    //MOD BY)T.Osaki 2020.06.04
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
    //MOD BY)T.Osaki 2020.06.04
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
