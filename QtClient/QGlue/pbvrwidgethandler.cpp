#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "pbvrwidgethandler.h"

#include "Panels/slidercontrol.h"
#include "boundingbox.h"

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_win32.h>
#include <imgui.h>
#include <imgui_internal.h>

//#ifdef DISP_MODE_VR
//#include "headmounteddisplay.h"
//#include <OVR_CAPI.h>
//#endif

#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#define INITIAL_STATE_CAMERA_IMAGE true
#else // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#define INITIAL_STATE_CAMERA_IMAGE false
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#define INITIAL_STATE_CONTROL_SPHERE true

// definitions of cursor shape & colors
static const int CursorShape[3 * 2] = {
    0, 0, 12, 20, 20, 12,
};
static const ImU32 CursorEdgeColor = (0 << 24) | (0 << 16) | (0 << 8) | 255;
// static const ImU32 CursorFillColor = (255 << 24) | (255 << 16) | (255 << 8) | 255;
static const ImU32 CursorFillColor = (255 << 24) | (0 << 16) | (0 << 8) | 255;

PBVRWidgetHandler::PBVRWidgetHandler() : WidgetHandler(), m_lefty_mode(false), slider_min(0), slider_max(0) {
  slider_val[0] = 0;
  slider_val[1] = 0;
  slider_val[2] = 0;
}

PBVRWidgetHandler::PBVRWidgetHandler(int width, int height)
    : WidgetHandler(width, height), m_lefty_mode(false), m_showBoundingBoxFlag(INITIAL_STATE_BOUNDING_BOX),
      m_showControlSphereFlag(INITIAL_STATE_CONTROL_SPHERE),
#ifdef MR_MODE
      m_showCameraImageFlag(INITIAL_STATE_CAMERA_IMAGE),
#endif // MR_MODE
      slider_min(0), slider_max(0) {
  slider_val[0] = 0;
  slider_val[1] = 0;
  slider_val[2] = 0;
}

bool PBVRWidgetHandler::initialize() {
  initializeImGui();
  initializeStatus();

  SliderSynchronizer::instance().addClient(this);
  BoundingBoxSynchronizer::instance().addClient(this);

  return true;
}

void PBVRWidgetHandler::initializeStatus() {
  // mouse
  //   cursor position
  this->m_cursorPositionX = this->m_width / 2;
  this->m_cursorPositionY = this->m_height / 2;

  //   button
  this->m_mouse_button_pushed_curr = false;
  this->m_mouse_button_pushed_prev = false;
  this->m_mouse_button_status = MOUSE_STATUS_HOVERED;

  // status of widgets

  //   check box for bounding box on/off
  this->m_showBoundingBoxFlag = INITIAL_STATE_BOUNDING_BOX;
  this->m_showControlSphereFlag = INITIAL_STATE_CONTROL_SPHERE;
#ifdef MR_MODE
  this->m_showCameraImageFlag = INITIAL_STATE_CAMERA_IMAGE;
#endif // MR_MODE

#ifdef ENABLE_TIME_MEASURE
  std::cerr << "[TIME_MEASURE][-1] camera[" << (m_showCameraImageFlag ? "o" : "x") << "]/bbox[" << (m_showBoundingBoxFlag ? "o" : "x") << "]"
            << std::endl;
#endif // ENABLE_TIME_MEASURE
}

void PBVRWidgetHandler::updateStatus() {
  const static float BORDER_WIDTH = 1.0;

  if (!m_mouse_status.updated) {
    return;
  }

  this->m_cursorPositionX += m_mouse_status.cursor_dx;
  if (this->m_cursorPositionX < BORDER_WIDTH) {
    this->m_cursorPositionX = BORDER_WIDTH;
  } else if (this->m_cursorPositionX > m_width - BORDER_WIDTH) {
    this->m_cursorPositionX = m_width - BORDER_WIDTH;
  }

  this->m_cursorPositionY += -m_mouse_status.cursor_dy;
  if (this->m_cursorPositionY < BORDER_WIDTH) {
    this->m_cursorPositionY = BORDER_WIDTH;
  } else if (this->m_cursorPositionY > m_height - BORDER_WIDTH) {
    this->m_cursorPositionY = m_height - BORDER_WIDTH;
  }

  // std::cout << "PBVRWidgetHandler::updateStatus(): mousePos=(" << m_cursorPositionX << ", " << m_cursorPositionY << ")" << std::endl;

  this->m_mouse_button_pushed_prev = this->m_mouse_button_pushed_curr;
  this->m_mouse_button_pushed_curr = m_mouse_status.buttonPressed[0];

  if (m_mouse_button_pushed_curr) {
    if (m_mouse_button_pushed_prev) {
      // dragged
      m_mouse_button_status = MOUSE_STATUS_DRAGGED;
      // std::cout << "PBVRWidgetHandler::updateStatus(): mouse is dragged." << std::endl;
    } else {
      // pressed
      m_mouse_button_status = MOUSE_STATUS_PRESSED;
      std::cout << "PBVRWidgetHandler::updateStatus(): mouse is pressed." << std::endl;
    }
  } else {
    if (m_mouse_button_pushed_prev) {
      // released
      m_mouse_button_status = MOUSE_STATUS_RELEASED;
      std::cout << "PBVRWidgetHandler::updateStatus(): mouse is released." << std::endl;
    } else {
      // hovered
      m_mouse_button_status = MOUSE_STATUS_HOVERED;
      // std::cout << "PBVRWidgetHandler::updateStatus(): mouse is hovered." << std::endl;
    }
  }

  m_mouse_status.clear();
}

void PBVRWidgetHandler::update() {

  // std::cout << "PBVRWidgetHandler::update(): start." << std::endl;

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  // io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  //  This flag requires explicit font load...
  // io.MouseDrawCursor = true;

  io.DisplaySize.x = this->m_width;
  io.DisplaySize.y = this->m_height;

  // std::cout << "PBVRWidgetHandler::update(): set up ImGuiIO / dispSize=(" << io.DisplaySize.x << ", " << io.DisplaySize.y << ")." << std::endl;

  // input devices set disabled
  io.WantCaptureKeyboard = false;
  io.NavActive = false;
  io.NavVisible = false;

  // set mouse status
  io.WantCaptureMouse = false;
  io.WantSetMousePos = true;
  io.MousePos.x = m_cursorPositionX;
  io.MousePos.y = m_cursorPositionY;
  io.MouseDown[0] = m_mouse_button_pushed_curr;
  io.MouseDown[1] = false;
  io.MouseDown[2] = false;
  io.MouseDown[3] = false;
  io.MouseDown[4] = false;

  ImGui::SetNextWindowFocus();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowSize(ImVec2(200, 150));
  ImGui::SetNextWindowFocus();

  ImGui::Begin("Control");

  char label_bbox[] = "Bounding Box";
  bool prev_enableBoudingBox = this->m_showBoundingBoxFlag;
  drawCheckBox(label_bbox, &m_showBoundingBoxFlag);
  if (prev_enableBoudingBox != this->m_showBoundingBoxFlag) {
    BoundingBoxSynchronizer::instance().synchronize(this, this->m_showBoundingBoxFlag);
  }

  char label_csphere[] = "Control Sphere";
  drawCheckBox(label_csphere, &m_showControlSphereFlag);

#ifdef MR_MODE
  char label_camera[] = "Background Camera Image";
  drawCheckBox(label_camera, &m_showCameraImageFlag);
#endif // MR_MODE

  drawSliders();

  ImGui::End();

  drawPseudoMouseCursor();

  ImGui::EndFrame();
  ImGui::Render();

  glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PBVRWidgetHandler::drawPseudoMouseCursor() {
  ImVec2 pos[3];
  ImVec2 mousePos = ImGui::GetMousePos();
  for (int i = 0; i < 3; i++) {
    pos[i].x = mousePos.x + CursorShape[i * 2];
    pos[i].y = mousePos.y + CursorShape[i * 2 + 1];
  }

  // Draw
  ImGui::GetForegroundDrawList()->AddTriangleFilled(pos[0], pos[1], pos[2], CursorFillColor);
  ImGui::GetForegroundDrawList()->AddTriangle(pos[0], pos[1], pos[2], CursorEdgeColor);
}

void PBVRWidgetHandler::drawCheckBox(char *label, bool *enable) {
  bool pressed = ImGui::Checkbox(label, enable);
  if (ImGui::IsItemClicked()) {
    *enable = !(*enable);
  }
}

void PBVRWidgetHandler::updateBoundingBoxVisibility(bool isVisible){
  m_showBoundingBoxFlag = isVisible;
}

void PBVRWidgetHandler::updateSliders(int min, int max, int curTime, int minTime, int maxTime) {
  slider_min = min;
  slider_max = max;
  slider_val[0] = curTime;
  slider_val[1] = minTime;
  slider_val[2] = maxTime;

  // std::cerr << "PBVRWidgetHandler::updateSliders():" << std::endl;
  // std::cerr << "  min= " << min << ", max= " << max << ", cur= " << curTime << ", minTime= " << minTime << ", maxTime= " << maxTime << std::endl;

  // time slider : slider_val[1] <= slider_val[0] <= slider_val[2]
  // min slider : slider_min <= slider_val[1] <= slider_val[2]
  // time slider : slider_val[1] <= slider_val[2] <= slider_max
  // overall : slider_min <= slider_val[1] <= slider_val[0] <= slider_val2[ <= slider_max
  if (slider_val[1] < slider_min) {
    slider_val[1] = slider_min;
  }
  if (slider_val[1] > slider_max) {
    slider_val[1] = slider_max;
  }
  if (slider_val[2] < slider_min) {
    slider_val[2] = slider_min;
  }
  if (slider_val[2] > slider_max) {
    slider_val[2] = slider_max;
  }

  // std::cerr << "    -> slider_val[0]= " << slider_val[0]
  //           << ", slider_val[1]= " << slider_val[1]
  //           << ", slider_val[2]= " << slider_val[2]
  //           << std::endl;

}

void PBVRWidgetHandler::drawSliders() {
  int prev_val[3] = {slider_val[0], slider_val[1], slider_val[2]};

  ImGui::SliderInt("Time", &slider_val[0], slider_min, slider_max, "%d", ImGuiSliderFlags_NoInput);
  ImGui::SliderInt("Min", &slider_val[1], slider_min, slider_max, "%d", ImGuiSliderFlags_NoInput);
  ImGui::SliderInt("Max", &slider_val[2], slider_min, slider_max, "%d", ImGuiSliderFlags_NoInput);

  if (prev_val[0] != slider_val[0] || prev_val[1] != slider_val[1] || prev_val[2] != slider_val[2]) {
    // int min = slider_min / SliderControl::stepSize;
    // int max = slider_max / SliderControl::stepSize;
    // int curTime = slider_val[0] / SliderControl::stepSize;
    // int minTime = slider_val[1] / SliderControl::stepSize;
    // int maxTime = slider_val[2] / SliderControl::stepSize;
    int min = slider_min;
    int max = slider_max;
    int curTime = slider_val[0];
    int minTime = slider_val[1];
    int maxTime = slider_val[2];

    // std::cerr << "PBVRWidgetHandler::drawSliders(): stepSize= " << SliderControl::stepSize << std::endl;
    // std::cerr << "  min= " << min
    //           << ", max= " << max
    //           << ", cur= " << curTime
    //           << ", minTime= " << minTime
    //           << ", maxTime= " << maxTime
    //           << std::endl;

    SliderSynchronizer::instance().synchronize(this, min, max, curTime, minTime, maxTime);
  }
}

void PBVRWidgetHandler::initializeImGui() {
  IMGUI_CHECKVERSION();
  ImGuiContext *context = ImGui::CreateContext();
  GImGui->NavDisableHighlight = false;

#ifdef DEBUG_HUD
  std::cerr << "ImGuiWidgetHandler::initialize() : ImGui::CreateContext() finished. (context=" << context << ")" << std::endl;
#endif // DEBUG_HUD

  ImGuiIO &io = ImGui::GetIO();
  (void)io; // ???

#ifdef DEBUG_HUD
  std::cerr << "ImGuiWidgetHandler::initialize() : ImGui::GetIO()() finished." << std::endl;
#endif // DEBUG_HUD

  ImGui::StyleColorsDark();
  // ImGui_ImplWin32_InitForOpenGL();
  ImGui_ImplOpenGL3_Init();

#ifdef DEBUG_HUD
  std::cerr << "ImGuiWidgetHandler::initialize() : ImGui_ImplOpenGL3_Init() finished." << std::endl;
#endif // DEBUG_HUD
}

bool PBVRWidgetHandler::destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
  return true;
}

static const int default_hud_screen_height = 640;
static const int default_hud_screen_width = 640;

static PBVRWidgetHandler *instance = nullptr;
PBVRWidgetHandler *PBVRWidgetHandlerInstance() {
  if (instance == nullptr) {
    instance = new PBVRWidgetHandler(default_hud_screen_width, default_hud_screen_height);
  }
  return instance;
}
