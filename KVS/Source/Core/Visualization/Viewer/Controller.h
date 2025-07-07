/****************************************************************************/
/**
 *  @file   Cotroller.h
 *  @author Keisuke Tajiri
 */
/****************************************************************************/
#pragma once
#include <kvs/Xform>
#include <kvs/Side>

namespace kvs
{

struct Controller
{
	enum Button
	{
		Trigger,
		Menu,
		A,
		B,
		X,
		Y,
		Max
	};

	struct ButtonStatus
	{
		bool last;
		bool pressed;
		bool pressing;
		bool released;

		ButtonStatus() :
			last( false ),
			pressed( false ),
			pressing( false ),
			released( false ) {}

		ButtonStatus( const ButtonStatus& bs) :
			last( bs.last ),
			pressed( bs.pressed),
			pressing( bs.pressing ),
			released( bs.released ) {}
	};

	struct ControllerStatus
	{
		bool is_active[kvs::Side::Max];
		kvs::Xform xform[kvs::Side::Max];
		kvs::Xform last_xform[kvs::Side::Max];
		kvs::Vec2 axis_value[kvs::Side::Max];
		kvs::Controller::ButtonStatus button_status[kvs::Side::Max][kvs::Controller::Max];

		ControllerStatus()
		{
			for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
			{
				is_active[i] = false;
				xform[i] = kvs::Xform();
				last_xform[i] = kvs::Xform();
				axis_value[i] = kvs::Vec2::Zero();
				for (kvs::UInt32 j = 0; j < kvs::Controller::Max; ++j)
				{
					button_status[i][j] = kvs::Controller::ButtonStatus();
				}
			}
		}

		ControllerStatus( const ControllerStatus& cs )
		{
			for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
			{
				is_active[i] = cs.is_active[i];
				xform[i] = cs.xform[i];
				last_xform[i] = cs.last_xform[i];
				axis_value[i] = cs.axis_value[i];
				for (kvs::UInt32 j = 0; j < kvs::Controller::Max; ++j)
				{
					button_status[i][j] = kvs::Controller::ButtonStatus( cs.button_status[i][j] );
				}
			}
		}
	};

	enum Action
	{
		NoAction,
		Pressed,
		Released,
		Moved,
		AxisChanged
	};
};

} // end of namespace kvs
