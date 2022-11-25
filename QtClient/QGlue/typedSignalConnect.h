#ifndef TYPED_SIGNAL_CONNECT_H
#define TYPED_SIGNAL_CONNECT_H

/**
 * The connect_T macro, mimics the Qt connect function for cases where the connect function
 *  fails to find the signal with the correct type.
 *
 *    To replace a connect call,
 *    connect(ui->component, &QWidgetClass::signalName,
 *            this, &ObjectClass::on_signalName);
 *
 *
 *    becomes:
 *    connect_T(ui->component, QWidgetClass, signalName, type,
 *              &ObjectClass::on_signalName);
 *
 *
*/
#define GET_SIGNAL_REF(CLASS, EVT ,T) void( CLASS::* SELECTEDSIG )(T) = &CLASS::EVT;
#define connect_T(OBJ,OCLASS,EVT,T,HANDLER) { GET_SIGNAL_REF( OCLASS , EVT , T); connect( OBJ , SELECTEDSIG , this,  HANDLER );}


#endif // TYPED_SIGNAL_CONNECT_H
