#include "VRHandControllerListener.h"

VRHandControllerListener::VRHandControllerListener( kvs::qt::jaea::Screen* screen )
    : m_screen( screen )
{

}

VRHandControllerListener::~VRHandControllerListener()
{

}

void VRHandControllerListener::onEvent( kvs::EventBase* event )
{
    auto* e = dynamic_cast<kvs::ControllerEvent*>(event);
    if( e )
    {
        auto* s = m_screen->scene();

        const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
        if ( e->type() == kvs::EventBase::ControllerPressEvent )
        {
            for ( kvs::UInt32 i = 0; i < kvs::Side::Max; i++ )
            {
                for( kvs::UInt32 j = 0; j < kvs::Controller::Button::Max; j++ )
                {
                    if ( cs.button_status[i][j].pressed )
                    {
                        if ( j == kvs::Controller::Button::Trigger )
                        {
                            kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Trigger", i );                            
                        }
                        if ( j == kvs::Controller::Button::Menu )
                        {
                        }
                        if ( j == kvs::Controller::Button::A )
                        {
                            kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed A", i );
                            emit showHidePlotOverLine();
                        }
                        if ( j == kvs::Controller::Button::B )
                        {
                            kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed B", i );
                            kvs::Xform currentObjectManagerXform = s->objectManager()->xform(); //現在の画面の平行移動、回転、スケーリングを取得する。
                            float scalingFactor = 1 / ( s->objectManager()->xform().inverse() * s->object( s->numberOfObjects() - 1 )->xform() ).scaling().x();
                            // float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->startPoint()->xform() ).scaling().x();

                            double startXTranslation = m_screen->openxrInteractor()->startInitialTranslation().x() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->startPoint()->xform() ).translation().x() ) - currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x();
                            double startYTranslation = m_screen->openxrInteractor()->startInitialTranslation().y() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->startPoint()->xform() ).translation().y() ) - currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y();
                            double startZTranslation = m_screen->openxrInteractor()->startInitialTranslation().z() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->startPoint()->xform() ).translation().z() ) - currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z();

                            double startXCoordinateMovement = ( startXTranslation * scalingFactor * -1 ) -
                                                              ( currentObjectManagerXform.translation().x() * scalingFactor * currentObjectManagerXform.inverse().scaling().x() );
                            double startYCoordinateMovement = ( startYTranslation * scalingFactor * -1 ) -
                                                              ( currentObjectManagerXform.translation().y() * scalingFactor * currentObjectManagerXform.inverse().scaling().y() );
                            double startZCoordinateMovement = ( startZTranslation * scalingFactor * -1 ) -
                                                              ( currentObjectManagerXform.translation().z() * scalingFactor * currentObjectManagerXform.inverse().scaling().z() );

                            { // ObjectEditorとの連携時に必要
                                startXCoordinateMovement += m_screen->openxrInteractor()->startPoint()->externalCenter().x();
                                startYCoordinateMovement += m_screen->openxrInteractor()->startPoint()->externalCenter().y();
                                startZCoordinateMovement += m_screen->openxrInteractor()->startPoint()->externalCenter().z();
                            }

                            double endXTranslation = m_screen->openxrInteractor()->endInitialTranslation().x() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->endPoint()->xform() ).translation().x() ) - currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x();
                            double endYTranslation = m_screen->openxrInteractor()->endInitialTranslation().y() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->endPoint()->xform() ).translation().y() ) - currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y();
                            double endZTranslation = m_screen->openxrInteractor()->endInitialTranslation().z() - ( ( currentObjectManagerXform.inverse() * m_screen->openxrInteractor()->endPoint()->xform() ).translation().z() ) - currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z();

                            double endXCoordinateMovement = ( endXTranslation * scalingFactor * -1 ) -
                                                            ( currentObjectManagerXform.translation().x() * scalingFactor * currentObjectManagerXform.inverse().scaling().x() );
                            double endYCoordinateMovement = ( endYTranslation * scalingFactor * -1 ) -
                                                            ( currentObjectManagerXform.translation().y() * scalingFactor * currentObjectManagerXform.inverse().scaling().y() );
                            double endZCoordinateMovement = ( endZTranslation * scalingFactor * -1 ) -
                                                            ( currentObjectManagerXform.translation().z() * scalingFactor * currentObjectManagerXform.inverse().scaling().z() );

                            { // ObjectEditorとの連携時に必要
                                endXCoordinateMovement += m_screen->openxrInteractor()->endPoint()->externalCenter().x();
                                endYCoordinateMovement += m_screen->openxrInteractor()->endPoint()->externalCenter().y();
                                endZCoordinateMovement += m_screen->openxrInteractor()->endPoint()->externalCenter().z();
                            }

                            // std::cout << "start : ( " << startXCoordinateMovement << ", " << startYCoordinateMovement << ", " << startZCoordinateMovement << " )" << std::endl;
                            // std::cout << "  end : ( " << endXCoordinateMovement << ", " << endYCoordinateMovement << ", " << endZCoordinateMovement << " )" << std::endl;
                            kvs::Real32 coordArray[ 2 * 3 ] =
                                {
                                    kvs::Real32( startXCoordinateMovement ), kvs::Real32( startYCoordinateMovement ), kvs::Real32( startZCoordinateMovement ),
                                    kvs::Real32( endXCoordinateMovement ), kvs::Real32( endYCoordinateMovement ), kvs::Real32( endZCoordinateMovement ),
                                };

                            emit drawPlotOverLine( coordArray );
                        }
                        if ( j == kvs::Controller::Button::X )
                        {
                            kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed X", i );
                        }
                        if ( j == kvs::Controller::Button::Y )
                        {
                            kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Y", i );
                        }
                    }
                }
            }
        }
    }
}
