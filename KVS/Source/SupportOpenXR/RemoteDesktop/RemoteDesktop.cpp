/*****************************************************************************/
/**
 *  @file   RemoteDesktop.cpp
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#include "RemoteDesktop.h"
#include <kvs/Key>
#include <thread>
#include <wingdi.h>

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new RemoteDesktop class.
 */
/*===========================================================================*/
RemoteDesktop::RemoteDesktop()
{
	kvsMessageDebug( "RemoteDesktop::RemoteDesktop()" );

	const int VK_PANEL_SIZE_W = 1280;
	const int VK_NUM_BUTTON_H = 18;
	const int VK_NUM_BUTTON_V = 4;

	int button_size = VK_PANEL_SIZE_W / ( VK_NUM_BUTTON_H + 1 );
	m_vk_button_size = kvs::Vec2i( button_size, button_size );
	m_vk_space_size = m_vk_button_size / 4;
	m_vk_size = kvs::Vec2i( VK_PANEL_SIZE_W, button_size * ( VK_NUM_BUTTON_V + 1 ) );
}

/*===========================================================================*/
/**
 *  @brief  Destructs a new RemoteDesktop class.
 */
 /*===========================================================================*/
RemoteDesktop::~RemoteDesktop()
{
	kvsMessageDebug("RemoteDesktop::~RemoteDesktop()");

	m_rdp_texture.release();
	m_vk_texture.release();
	m_vk_fbo.release();

	if ( m_hbitmap )
	{
		DeleteObject( m_hbitmap );
		m_hbitmap = nullptr;
	}
	if ( m_hdc )
	{
		DeleteDC( m_hdc );
		m_hdc = nullptr;
	}

	ImGui_ImplOpenGL3_Shutdown();
}

/*===========================================================================*/
/**
 *  @brief  
 */
 /*===========================================================================*/
void RemoteDesktop::initialize()
{
	HDC hdc = GetDC( NULL );
	m_hdc = CreateCompatibleDC( hdc );
	ReleaseDC( nullptr, hdc );

	m_screen_pos = kvs::Vec2i( GetSystemMetrics( SM_XVIRTUALSCREEN ), GetSystemMetrics( SM_YVIRTUALSCREEN ) );
	m_screen_size = kvs::Vec2i( GetSystemMetrics( SM_CXVIRTUALSCREEN ), GetSystemMetrics( SM_CYVIRTUALSCREEN ) );

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth = m_screen_size[0];
	bmi.bmiHeader.biHeight = m_screen_size[1];
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = m_screen_size[0] * m_screen_size[1] * ( 24 / 8 );
	m_hbitmap = CreateDIBSection( nullptr, &bmi, DIB_RGB_COLORS, (void**)&m_rdp_image, nullptr, 0 );
	SelectObject( m_hdc, m_hbitmap );

	m_rdp_texture.setWrapS( GL_CLAMP_TO_EDGE );
	m_rdp_texture.setWrapT( GL_CLAMP_TO_EDGE );
	m_rdp_texture.setMagFilter( GL_LINEAR );
	m_rdp_texture.setMinFilter( GL_LINEAR );
	m_rdp_texture.setPixelFormat( GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE );
	m_rdp_texture.create( m_screen_size[0], m_screen_size[1] );

	m_vk_texture.setWrapS( GL_CLAMP_TO_EDGE );
	m_vk_texture.setWrapT( GL_CLAMP_TO_EDGE );
	m_vk_texture.setMagFilter( GL_LINEAR );
	m_vk_texture.setMinFilter( GL_LINEAR );
	m_vk_texture.setPixelFormat( GL_RGB8, GL_RGB, GL_FLOAT );
	m_vk_texture.create( m_vk_size[0], m_vk_size[1] );

	m_vk_fbo.create();
	m_vk_fbo.attachColorTexture( m_vk_texture );

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplOpenGL3_Init();
}

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
void RemoteDesktop::update( const kvs::Xform& controller_xform, 
							bool pressed, 
							bool pressing, 
							bool released, 
							float& distance )
{
	if ( !m_visible )
	{
		return;
	}

	distance = 1.0f;
	
	kvs::Vec3 ray_origin = controller_xform.translation();
	kvs::Mat4 m = controller_xform.toMatrix();
	kvs::Vec3 ray_dir = -kvs::Vec3( m[0][2], m[1][2], m[2][2] );
	ray_dir.normalize();

	kvs::Vec3 plane_origin = m_rdp_xform.translation();
	m = m_rdp_xform.toMatrix();
	kvs::Vec3 plane_front_dir = kvs::Vec3( m[0][2], m[1][2], m[2][2] );
	kvs::Vec3 plane_up_dir = kvs::Vec3( m[0][1], m[1][1], m[2][1] );
	kvs::Vec3 plane_right_dir = kvs::Vec3( m[0][0], m[1][0], m[2][0] );
	plane_front_dir.normalize();
	plane_up_dir.normalize();
	plane_right_dir.normalize();

	float aspect = (float)m_screen_size[0] / (float)m_screen_size[1];
	kvs::Vec2 plane_size = kvs::Vec2( m_rdp_height * aspect, m_rdp_height );
	kvs::Vec2 uv = kvs::Vec2::Zero();
	bool hit_rdp = raycast( ray_origin, 
						    ray_dir, 
							plane_origin, 
							plane_front_dir, 
							plane_up_dir, 
							plane_right_dir, 
							plane_size, 
							uv, 
							distance );
	if ( hit_rdp )
	{
		int dx = uv[0] * m_screen_size[0];
		int dy = uv[1] * m_screen_size[1];
	
		POINT pos = { dx, dy };
		HWND hwnd = WindowFromDC( m_hdc );
		ClientToScreen( hwnd, &pos );
		SetCursorPos( pos.x, pos.y );

		if ( pressed )
		{
			clickedMouseLeft();
		}
		else if ( pressing && !m_vk_pressing )
		{
			m_vk_pressing = true;
			pressMouseLeft();
		}
		else if ( released && m_vk_pressing )
		{
			m_vk_pressing = false;
			releaseMouseLeft();
		}
	}
	grabDesktopImage();

	aspect = static_cast<float>( m_vk_size[0] ) / static_cast<float>( m_vk_size[1] );
	float vk_height = m_rdp_height * 0.5f;
	plane_size = kvs::Vec2( vk_height * aspect, vk_height );
	kvs::Vec3 vk_offset = kvs::Vec3( 0.0f, 
								     -( m_rdp_height + vk_height + 0.05f ) / 2.0f,
								     0.0f);
	m_vk_xform = m_rdp_xform * kvs::Xform::Translation( vk_offset );
	plane_origin = m_vk_xform.translation();
	m = m_vk_xform.toMatrix();
	plane_front_dir = kvs::Vec3( m[0][2], m[1][2], m[2][2] );
	plane_up_dir = kvs::Vec3( m[0][1], m[1][1], m[2][1] );
	plane_right_dir = kvs::Vec3( m[0][0], m[1][0], m[2][0] );
	plane_front_dir.normalize();
	plane_up_dir.normalize();
	plane_right_dir.normalize();
	m_vk_mouse_pos = kvs::Vec2i::Zero();
	m_vk_pressed = false;

	if ( !hit_rdp )
	{
		bool hit_vk = raycast( ray_origin, 
							   ray_dir, 
							   plane_origin, 
							   plane_front_dir, 
							   plane_up_dir, 
							   plane_right_dir, 
							   plane_size, 
							   uv, 
							   distance );
		if ( hit_vk )
		{
			m_vk_mouse_pos[0] = uv[0] * m_vk_size[0];
			m_vk_mouse_pos[1] = uv[1] * m_vk_size[1];
			m_vk_pressed = pressed;
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.DisplaySize.x = (float)m_vk_size[0];
	io.DisplaySize.y = (float)m_vk_size[1];
	io.WantCaptureKeyboard = false;
	io.NavActive = false;
	io.NavVisible = false;
	io.WantCaptureMouse = false;
	io.WantSetMousePos = true;
	io.MouseDrawCursor = false;
	io.MousePos.x = m_vk_mouse_pos[0];
	io.MousePos.y = m_vk_mouse_pos[1];
	io.MouseDown[0] = m_vk_pressed;
	io.MouseDown[1] = false;
	io.MouseDown[2] = false;
	io.MouseDown[3] = false;
	io.MouseDown[4] = false;
	io.FontGlobalScale = 3.0f;

	static const ImVec4 VK_BUTTON_COLOR = ImVec4( 0.17f, 0.24f, 0.27f, 1.0f );
	static const ImVec4 VK_BACKGROUND_COLOR = ImVec4( 0.09f, 0.16f, 0.2f, 1.0f );

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2() );
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2() );
	ImGui::PushStyleColor( ImGuiCol_Button, VK_BUTTON_COLOR );
	ImGui::PushStyleColor( ImGuiCol_WindowBg, VK_BACKGROUND_COLOR );
	ImGui::PushStyleColor( ImGuiCol_WindowBg, VK_BACKGROUND_COLOR );

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImVec2 button_size = ImVec2( m_vk_button_size[0], m_vk_button_size[1] );
	ImVec2 space_size = ImVec2( m_vk_space_size[0], m_vk_space_size[1] );

	ImVec2 func_pos = ImVec2( 0, space_size[1] );
	ImVec2 func_size = ImVec2( button_size[0], button_size[1] * 4 );
	ImVec2 esc_key_size = ImVec2( button_size[0], button_size[1] * 2 );
	ImVec2 close_key_size = ImVec2( button_size[0], button_size[1] * 2 );
	ImGui::SetNextWindowPos( func_pos );
	ImGui::SetNextWindowSize( func_size );

	ImGui::Begin( "Function", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove );
	if ( ImGui::Button( "ESC", esc_key_size ) ) { pressedKey( VK_ESCAPE, false ); }
	if ( ImGui::Button( "[X]", close_key_size ) ) { pressedKey( VK_MENU, false ); }
	ImGui::End();

	ImVec2 numpad_pos = ImVec2( func_size[0] + space_size[0], space_size[1] );
	ImVec2 numpad_size = ImVec2( button_size[0] * 3, button_size[1] * 4 );
	ImGui::SetNextWindowPos( numpad_pos );
	ImGui::SetNextWindowSize( numpad_size );

	ImGui::Begin( "NumPad", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove );
	if ( ImGui::Button( "1", button_size ) ) { pressedKey( kvs::Key::ASCIICode::One, false); }		ImGui::SameLine();
	if ( ImGui::Button( "2", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Two, false); }		ImGui::SameLine();
	if ( ImGui::Button( "3", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Three, false); }
	
	if ( ImGui::Button( "4", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Four, false); }		ImGui::SameLine();
	if ( ImGui::Button( "5", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Five, false); }		ImGui::SameLine();
	if ( ImGui::Button( "6", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Six, false); }

	if ( ImGui::Button( "7", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Seven, false); }	ImGui::SameLine();
	if ( ImGui::Button( "8", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Eight, false); }	ImGui::SameLine();
	if ( ImGui::Button( "9", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Nine, false); }
	
	ImGui::Dummy( button_size );																	ImGui::SameLine();
	if ( ImGui::Button( "0", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Zero, false ); }
	ImGui::End();

	ImVec2 keyboard_pos = ImVec2( numpad_pos[0] + numpad_size[0], space_size[1] );
	ImVec2 keyboard_size = ImVec2( button_size[0] * 11, button_size[1] * 4 );
	ImVec2 shift_key_size = ImVec2( button_size[0] * 2, button_size[1] );
	ImVec2 symbol_key_size = ImVec2( button_size[0] * 2, button_size[1] );
	ImVec2 space_key_size = ImVec2( button_size[0] * 4, button_size[1] );
	ImGui::SetNextWindowPos( keyboard_pos );
	ImGui::SetNextWindowSize( keyboard_size );
	
	ImGui::Begin( "Keyboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove );
	if ( m_symbol_mode )
	{
		if ( ImGui::Button( "!", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::One, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "\"", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Two, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "#", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Three, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "$", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Four, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "%", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Five, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "&", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Six, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "'", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Seven, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "(", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Eight, true ); }	ImGui::SameLine();
		if ( ImGui::Button( ")", button_size ) )	{ pressedKey( kvs::Key::ASCIICode::Nine, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "=", button_size ) )	{ pressedKey( VK_OEM_MINUS, true ); }				ImGui::SameLine();
		if ( ImGui::Button( "*", button_size ) )	{ pressedKey( VK_MULTIPLY, false ); }

		if ( ImGui::Button( "+", button_size ) )	{ pressedKey( VK_ADD, false ); }		ImGui::SameLine();
		if ( ImGui::Button( ",", button_size ) )	{ pressedKey( VK_OEM_COMMA, false ); }	ImGui::SameLine();
		if ( ImGui::Button( "-", button_size ) )	{ pressedKey( VK_SUBTRACT, false ); }	ImGui::SameLine();
		if ( ImGui::Button( ".", button_size ) )	{ pressedKey( VK_OEM_PERIOD, false ); } ImGui::SameLine();
		if ( ImGui::Button( "/", button_size ) )	{ pressedKey( VK_OEM_2, false ); }		ImGui::SameLine();
		if ( ImGui::Button( ":", button_size ) )	{ pressedKey( VK_OEM_1, false ); }		ImGui::SameLine();
		if ( ImGui::Button( ";", button_size ) )	{ pressedKey( VK_OEM_PLUS, false ); }	ImGui::SameLine();
		if ( ImGui::Button( "<", button_size ) )	{ pressedKey( VK_OEM_COMMA, true ); }	ImGui::SameLine();
		if ( ImGui::Button( ">", button_size ) )	{ pressedKey( VK_OEM_PERIOD, true ); }	ImGui::SameLine();
		if ( ImGui::Button( "?", button_size ) )	{ pressedKey( VK_OEM_2, true ); }		ImGui::SameLine();
		if ( ImGui::Button( "@", button_size ) )	{ pressedKey( VK_OEM_3, false ); }

		if ( ImGui::Button( "[", button_size ) )	{ pressedKey( VK_OEM_4, false ); }		ImGui::SameLine();
		if ( ImGui::Button( "\\", button_size ) )	{ pressedKey( VK_OEM_102, false ); }	ImGui::SameLine();
		if ( ImGui::Button( "]", button_size ) )	{ pressedKey( VK_OEM_6, false ); }		ImGui::SameLine();
		if ( ImGui::Button( "_", button_size ) )	{ pressedKey( VK_OEM_102, true ); }		ImGui::SameLine();
		if ( ImGui::Button( "`", button_size ) )	{ pressedKey( VK_OEM_3, true ); }		ImGui::SameLine();
		if ( ImGui::Button( "{", button_size ) )	{ pressedKey( VK_OEM_4, true ); }		ImGui::SameLine();
		if ( ImGui::Button( "|", button_size ) )	{ pressedKey( VK_OEM_5, true ); }		ImGui::SameLine();
		if ( ImGui::Button( "}", button_size ) )	{ pressedKey( VK_OEM_6, true ); }
	}
	else
	{
		ImGui::Dummy( space_size );																							  ImGui::SameLine();
		ImGui::Dummy( space_size );																							  ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "Q" : "q", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Q, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "W" : "w", button_size ) ) { pressedKey( kvs::Key::ASCIICode::W, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "E" : "e", button_size ) ) { pressedKey( kvs::Key::ASCIICode::E, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "R" : "r", button_size ) ) { pressedKey( kvs::Key::ASCIICode::R, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "T" : "t", button_size ) ) { pressedKey( kvs::Key::ASCIICode::T, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "Y" : "y", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Y, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "U" : "u", button_size ) ) { pressedKey( kvs::Key::ASCIICode::U, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "I" : "i", button_size ) ) { pressedKey( kvs::Key::ASCIICode::I, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "O" : "o", button_size ) ) { pressedKey( kvs::Key::ASCIICode::O, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "P" : "p", button_size ) ) { pressedKey( kvs::Key::ASCIICode::P, m_shift_mode ); }
	
		ImGui::Dummy( space_size );																							  ImGui::SameLine();
		ImGui::Dummy( space_size );																							  ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "A" : "a", button_size ) ) { pressedKey( kvs::Key::ASCIICode::A, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "S" : "s", button_size ) ) { pressedKey( kvs::Key::ASCIICode::S, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "D" : "d", button_size ) ) { pressedKey( kvs::Key::ASCIICode::D, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "F" : "f", button_size ) ) { pressedKey( kvs::Key::ASCIICode::F, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "G" : "g", button_size ) ) { pressedKey( kvs::Key::ASCIICode::G, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "H" : "h", button_size ) ) { pressedKey( kvs::Key::ASCIICode::H, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "J" : "j", button_size ) ) { pressedKey( kvs::Key::ASCIICode::J, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "K" : "k", button_size ) ) { pressedKey( kvs::Key::ASCIICode::K, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "L" : "l", button_size ) ) { pressedKey( kvs::Key::ASCIICode::L, m_shift_mode ); } 
	
		ImGui::Dummy( button_size );																						  ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "Z" : "z", button_size ) ) { pressedKey( kvs::Key::ASCIICode::Z, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "X" : "x", button_size ) ) { pressedKey( kvs::Key::ASCIICode::X, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "C" : "c", button_size ) ) { pressedKey( kvs::Key::ASCIICode::C, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "V" : "v", button_size ) ) { pressedKey( kvs::Key::ASCIICode::V, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "B" : "b", button_size ) ) { pressedKey( kvs::Key::ASCIICode::B, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "N" : "n", button_size ) ) { pressedKey( kvs::Key::ASCIICode::N, m_shift_mode ); } ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "M" : "m", button_size ) ) { pressedKey( kvs::Key::ASCIICode::M, m_shift_mode ); }
	}

	if ( ImGui::Button( m_symbol_mode ? "abc" : "#+=", symbol_key_size ) ) { m_symbol_mode = !m_symbol_mode; } ImGui::SameLine();
	ImGui::Dummy( button_size );																			   ImGui::SameLine();
	if ( ImGui::Button( "Space", space_key_size ) ) { pressedKey( VK_SPACE, false ); }						   ImGui::SameLine();
	if ( m_symbol_mode )
	{
		ImGui::Dummy( button_size ); ImGui::SameLine();
		ImGui::Dummy( shift_key_size );
	}
	else
	{
		ImGui::Dummy( button_size ); ImGui::SameLine();
		if ( ImGui::Button( m_shift_mode ? "abc" : "ABC", shift_key_size ) ) { m_shift_mode = !m_shift_mode; }
	}
	ImGui::End();

	ImVec2 enter_pos = ImVec2( keyboard_pos[0] + keyboard_size[0], space_size[1] );
	ImVec2 enter_size = ImVec2( button_size[0] * 2, button_size[1] * 4 );
	ImVec2 backspace_key_size = ImVec2( button_size[0] * 2, button_size[1] );
	ImVec2 enter_key_size = ImVec2( button_size[0] * 2, button_size[1] * 2 );
	ImVec2 home_key_size = ImVec2( button_size[0] * 2, button_size[1] );
	ImGui::SetNextWindowPos( enter_pos );
	ImGui::SetNextWindowSize( enter_size );
	
	ImGui::Begin( "Enter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove );
	if ( ImGui::Button( "BS", backspace_key_size ) ) { pressedKey( VK_BACK, false ); }
	if ( ImGui::Button( "Enter", enter_key_size ) )  { pressedKey( VK_RETURN, false ); }
	if ( ImGui::Button( "Home", home_key_size ) )	 { pressedKey( VK_HOME, false ); }
	ImGui::End();

	ImVec2 size_control_pos = ImVec2( enter_pos[0] + enter_size[0] + space_size[0], space_size[1] );
	ImVec2 size_controll_size = ImVec2( button_size[0], button_size[1] * 4 );
	ImVec2 size_controll_button_size = ImVec2( button_size[0], button_size[1] * 2 );
	ImGui::SetNextWindowPos( size_control_pos );
	ImGui::SetNextWindowSize( size_controll_size );

	ImGui::Begin("SizeControl", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
	if (ImGui::Button("[+]", size_controll_button_size)) { if ( m_rdp_height < 2.0f ) m_rdp_height += 0.2f; }
	if (ImGui::Button("[-]", size_controll_button_size)) { if ( m_rdp_height > 0.4f ) m_rdp_height -= 0.2f; }
	ImGui::End();
	
	ImGui::EndFrame();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::Render();

	m_vk_fbo.bind();
	kvs::OpenGL::SetViewport( 0, 0, m_vk_size[0], m_vk_size[1] );
	kvs::OpenGL::SetClearColor( VK_BACKGROUND_COLOR.x, VK_BACKGROUND_COLOR.y , VK_BACKGROUND_COLOR.z, VK_BACKGROUND_COLOR.w );
	kvs::OpenGL::Clear( GL_COLOR_BUFFER_BIT );
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
	m_vk_fbo.unbind();
}

/*===========================================================================*/
/**
 *  @brief Render RemoteDesktop and Virtual Keyboard
 */
/*===========================================================================*/
void RemoteDesktop::render()
{
	static const kvs::Vec2 texcoords[4] = {
		kvs::Vec2( 0.0f, 0.0f ),
		kvs::Vec2( 1.0f, 0.0f ),
		kvs::Vec2( 1.0f, 1.0f ),
		kvs::Vec2( 0.0f, 1.0f ),
	};

	if ( !m_visible )
	{
		return;
	}

	kvs::OpenGL::WithPushedAttrib a( GL_ALL_ATTRIB_BITS );
	kvs::OpenGL::Enable( GL_DEPTH_TEST );
	kvs::OpenGL::Enable( GL_BLEND );
	kvs::OpenGL::Enable( GL_TEXTURE_2D );
	kvs::OpenGL::Disable( GL_LIGHTING );

	kvs::OpenGL::WithPushedClientAttrib ca( GL_CLIENT_ALL_ATTRIB_BITS );
	kvs::OpenGL::EnableClientState( GL_VERTEX_ARRAY );
	kvs::OpenGL::EnableClientState( GL_TEXTURE_COORD_ARRAY );

	{
		float aspect = static_cast<float>( m_screen_size[0] ) / static_cast<float>( m_screen_size[1] );
		kvs::Vec2 size = kvs::Vec2(m_rdp_height * aspect, m_rdp_height);
		kvs::Vec2 vertices[4] = {
			kvs::Vec2(size[0] * -0.5f, size[1] * -0.5f),
			kvs::Vec2(size[0] *  0.5f, size[1] * -0.5f),
			kvs::Vec2(size[0] *  0.5f, size[1] *  0.5f),
			kvs::Vec2(size[0] * -0.5f, size[1] *  0.5f)
		};
	
		float m[16];
		m_rdp_xform.toArray( m );
	
		kvs::OpenGL::SetMatrixMode( GL_MODELVIEW );
		kvs::OpenGL::PushMatrix();
		kvs::OpenGL::MultMatrix( m );
		kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		kvs::OpenGL::TexCoordPointer( 2, GL_FLOAT, 0, texcoords );
		kvs::OpenGL::VertexPointer( 2, GL_FLOAT, 0, vertices );
		m_rdp_texture.bind();
		kvs::OpenGL::DrawArrays( GL_QUADS, 0, 4 );
		m_rdp_texture.unbind();
		kvs::OpenGL::PopMatrix();
	}

	{
		float aspect = static_cast<float>( m_vk_size[0] ) / static_cast<float>( m_vk_size[1] );
		float vk_height = m_rdp_height * 0.5f;
		kvs::Vec2 size = kvs::Vec2( vk_height * aspect, vk_height );
		kvs::Vec2 vertices[4] = {
			kvs::Vec2( size[0] * -0.5f, size[1] * -0.5f ),
			kvs::Vec2( size[0] *  0.5f, size[1] * -0.5f ),
			kvs::Vec2( size[0] *  0.5f, size[1] *  0.5f ),
			kvs::Vec2( size[0] * -0.5f, size[1] *  0.5f )
		};

		float m[16];
		m_vk_xform.toArray( m );

		kvs::OpenGL::SetMatrixMode( GL_MODELVIEW );
		kvs::OpenGL::PushMatrix();
		kvs::OpenGL::MultMatrix( m );
		kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		kvs::OpenGL::TexCoordPointer( 2, GL_FLOAT, 0, texcoords );
		kvs::OpenGL::VertexPointer( 2, GL_FLOAT, 0, vertices );
		m_vk_texture.bind();
		kvs::OpenGL::DrawArrays( GL_QUADS, 0, 4 );
		m_vk_texture.unbind();
		kvs::OpenGL::PopMatrix();
	}
}

/*===========================================================================*/
/**
 *  @brief Grab desktop image.
 */
/*===========================================================================*/
void RemoteDesktop::grabDesktopImage()
{
	HDC hdc = GetDC( nullptr );
	if ( hdc == nullptr )
	{
		kvsMessageError( "RemoteDesktop::grab() GetDC(null) returns null." );
		return;
	}
	BitBlt( m_hdc, 0, 0, m_screen_size[0], m_screen_size[1], hdc, m_screen_pos[0], m_screen_pos[1], SRCCOPY );
	ReleaseDC( nullptr, hdc );

	CURSORINFO cursorInfo = { 0 };
	cursorInfo.cbSize = sizeof( CURSORINFO );
	GetCursorInfo( &cursorInfo );

	ICONINFO iconInfo = { 0 };
	GetIconInfo( cursorInfo.hCursor, &iconInfo );

	int cursorX = cursorInfo.ptScreenPos.x - iconInfo.xHotspot;
	int cursorY = cursorInfo.ptScreenPos.y - iconInfo.yHotspot;
	DrawIcon( m_hdc, cursorX - m_screen_pos[0], cursorY - m_screen_pos[1], cursorInfo.hCursor );

	if ( iconInfo.hbmMask != nullptr )
	{
		DeleteObject( iconInfo.hbmMask );
	}
	if ( iconInfo.hbmColor != nullptr )
	{
		DeleteObject( iconInfo.hbmColor );
	}

	BITMAP bmp = { 0 };
	GetObject( m_hbitmap, sizeof( BITMAP ), &bmp );
	m_rdp_image = static_cast<kvs::UInt8*>( bmp.bmBits );
	m_rdp_texture.create( m_screen_size[0], m_screen_size[1], m_rdp_image );
}

/*===========================================================================*/
/**
 *  @brief Click mouse left function.
 */
 /*===========================================================================*/
void RemoteDesktop::clickedMouseLeft()
{
	std::thread t([]() {
		POINT mouse_pos;
		GetCursorPos( &mouse_pos );
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dx = mouse_pos.x * 65535 / GetSystemMetrics( SM_CXVIRTUALSCREEN );
		input.mi.dy = mouse_pos.y * 65535 / GetSystemMetrics( SM_CYVIRTUALSCREEN );
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput( 1, &input, sizeof( INPUT ) );
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput( 1, &input, sizeof( INPUT ) );
		});
	t.detach();
}

/*===========================================================================*/
/**
 *  @brief Press mouse left function.
 */
/*===========================================================================*/
void RemoteDesktop::pressMouseLeft()
{
	std::thread t([]() {
		POINT mouse_pos;
		GetCursorPos( &mouse_pos );
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dx = mouse_pos.x * 65535 / GetSystemMetrics( SM_CXVIRTUALSCREEN );
		input.mi.dy = mouse_pos.y * 65535 / GetSystemMetrics( SM_CYVIRTUALSCREEN );
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput( 1, &input, sizeof( INPUT ) );
	});
	t.detach();
}

/*===========================================================================*/
/**
 *  @brief Release mouse left function.
 */
 /*===========================================================================*/
void RemoteDesktop::releaseMouseLeft()
{
	std::thread t([]() {
		POINT mouse_pos;
		GetCursorPos( &mouse_pos );
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dx = mouse_pos.x * 65535 / GetSystemMetrics( SM_CXVIRTUALSCREEN );
		input.mi.dy = mouse_pos.y * 65535 / GetSystemMetrics( SM_CYVIRTUALSCREEN );
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput( 1, &input, sizeof( INPUT ) );
	});
	t.detach();
}

/*===========================================================================*/
/**
 *  @brief Pressed a key function.
 *  @param key [in] key
 *  @param shift [in] shift mode
 */
/*===========================================================================*/
void RemoteDesktop::pressedKey( kvs::Int32 key, bool shift )
{
	std::thread t([key, shift]() {
		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;

		if ( key == VK_MENU )
		{
			input.ki.wVk = VK_MENU;
			SendInput( 1, &input, sizeof( INPUT ) );

			input.ki.wVk = VK_F4;
			SendInput( 1, &input, sizeof( INPUT ) );
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput( 1, &input, sizeof( INPUT ) );

			input.ki.wVk = VK_MENU;
			SendInput( 1, &input, sizeof( INPUT ) );
		}
		else
		{
			if ( shift )
			{
				input.ki.wVk = VK_SHIFT;
				SendInput( 1, &input, sizeof( INPUT ) );
			}

			input.ki.wVk = key;
			SendInput( 1, &input, sizeof( INPUT ) );
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput( 1, &input, sizeof( INPUT ) );

			if ( shift )
			{
				input.ki.wVk = VK_SHIFT;
				SendInput( 1, &input, sizeof( INPUT ) );
			}
		}
	});
	t.detach();
}

/*===========================================================================*/
/**
 *  @brief Raycast to a plane function.
 *  @param ray_origin [in] ray origin position
 *  @param ray_dir [in] ray direction
 *  @param plane_origin [in] plane origin position
 *  @param plane_front_dir [in] plane front direction
 *  @param plane_up_dir [in] plane up direction
 *  @param plane_right_dir [in] plane right direction
 *  @param plane_size [in] plane rect size
 *  @param out_uv [out] uv
 *  @param out_distance [out] distance
 *  @return hit
 */
/*===========================================================================*/
bool RemoteDesktop::raycast( const kvs::Vec3& ray_origin,
							 const kvs::Vec3& ray_dir,
							 const kvs::Vec3& plane_origin,
							 const kvs::Vec3& plane_front_dir,
							 const kvs::Vec3& plane_up_dir,
							 const kvs::Vec3& plane_right_dir,
							 const kvs::Vec2& plane_size,
							 kvs::Vec2& out_uv,
							 float& out_distance )
{
	if ( kvs::Math::Abs( plane_front_dir.dot( ray_dir ) ) > 1e-6 )
	{
		kvs::Vec3 origin_to_plane = plane_origin - ray_origin;
		float t = origin_to_plane.dot( plane_front_dir ) / plane_front_dir.dot( ray_dir );
		if ( t >= 0 )
		{
			kvs::Vec3 hit_point = ray_origin + t * ray_dir;
			kvs::Vec3 local_point = hit_point - plane_origin;
			float u = local_point.dot( plane_right_dir ) / plane_size[0] + 0.5f;
			float v = local_point.dot( plane_up_dir) / plane_size[1] + 0.5f;
			if ( u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f )
			{
				return false;
			}
			else
			{
				out_uv[0] = u;
				out_uv[1] = 1.0f - v;

				out_distance = ( hit_point - ray_origin ).length();
				return true;
			}
		}
	}
	return false;
}

} // end of namespace openxr

} // end of namespace kvs
