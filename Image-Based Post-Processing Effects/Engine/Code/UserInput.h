#pragma once
#include <vector>
#include <set>
#include <glm\vec2.hpp>
#include <functional>
#include "Framework\Window\IInputListener.h"
#include "ICharListener.h"
#include "IKeyListener.h"
#include "IScrollListener.h"
#include "IMouseButtonListener.h"

const int INPUT_RELEASE = 0;
const int INPUT_PRESS = 1;
const int INPUT_REPEAT = 2;

/* The unknown key */
const int INPUT_KEY_UNKNOWN = -1;

/* Printable keys */
const int INPUT_KEY_SPACE = 32;
const int INPUT_KEY_APOSTROPHE = 39;  /* ' */
const int INPUT_KEY_COMMA = 44;  /* , */
const int INPUT_KEY_MINUS = 45;  /* - */
const int INPUT_KEY_PERIOD = 46;  /* . */
const int INPUT_KEY_SLASH = 47;  /* / */
const int INPUT_KEY_0 = 48;
const int INPUT_KEY_1 = 49;
const int INPUT_KEY_2 = 50;
const int INPUT_KEY_3 = 51;
const int INPUT_KEY_4 = 52;
const int INPUT_KEY_5 = 53;
const int INPUT_KEY_6 = 54;
const int INPUT_KEY_7 = 55;
const int INPUT_KEY_8 = 56;
const int INPUT_KEY_9 = 57;
const int INPUT_KEY_SEMICOLON = 59;  /* ; */
const int INPUT_KEY_EQUAL = 61; /* = */
const int INPUT_KEY_A = 65;
const int INPUT_KEY_B = 66;
const int INPUT_KEY_C = 67;
const int INPUT_KEY_D = 68;
const int INPUT_KEY_E = 69;
const int INPUT_KEY_F = 70;
const int INPUT_KEY_G = 71;
const int INPUT_KEY_H = 72;
const int INPUT_KEY_I = 73;
const int INPUT_KEY_J = 74;
const int INPUT_KEY_K = 75;
const int INPUT_KEY_L = 76;
const int INPUT_KEY_M = 77;
const int INPUT_KEY_N = 78;
const int INPUT_KEY_O = 79;
const int INPUT_KEY_P = 80;
const int INPUT_KEY_Q = 81;
const int INPUT_KEY_R = 82;
const int INPUT_KEY_S = 83;
const int INPUT_KEY_T = 84;
const int INPUT_KEY_U = 85;
const int INPUT_KEY_V = 86;
const int INPUT_KEY_W = 87;
const int INPUT_KEY_X = 88;
const int INPUT_KEY_Y = 89;
const int INPUT_KEY_Z = 90;
const int INPUT_KEY_LEFT_BRACKET = 91; /* [ */
const int INPUT_KEY_BACKSLASH = 92;/* \ */
const int INPUT_KEY_RIGHT_BRACKET = 93; /* ] */
const int INPUT_KEY_GRAVE_ACCENT = 96  /* ` */;
const int INPUT_KEY_WORLD_1 = 161;/* non-US #1 */
const int INPUT_KEY_WORLD_2 = 162;/* non-US #2 */

/* Function keys */;
const int INPUT_KEY_ESCAPE = 256;
const int INPUT_KEY_ENTER = 257;
const int INPUT_KEY_TAB = 258;
const int INPUT_KEY_BACKSPACE = 259;
const int INPUT_KEY_INSERT = 260;
const int INPUT_KEY_DELETE = 261;
const int INPUT_KEY_RIGHT = 262;
const int INPUT_KEY_LEFT = 263;
const int INPUT_KEY_DOWN = 264;
const int INPUT_KEY_UP = 265;
const int INPUT_KEY_PAGE_UP = 266;
const int INPUT_KEY_PAGE_DOWN = 267;
const int INPUT_KEY_HOME = 268;
const int INPUT_KEY_END = 269;
const int INPUT_KEY_CAPS_LOCK = 280;
const int INPUT_KEY_SCROLL_LOCK = 281;
const int INPUT_KEY_NUM_LOCK = 282;
const int INPUT_KEY_PRINT_SCREEN = 283;
const int INPUT_KEY_PAUSE = 284;
const int INPUT_KEY_F1 = 290;
const int INPUT_KEY_F2 = 291;
const int INPUT_KEY_F3 = 292;
const int INPUT_KEY_F4 = 293;
const int INPUT_KEY_F5 = 294;
const int INPUT_KEY_F6 = 295;
const int INPUT_KEY_F7 = 296;
const int INPUT_KEY_F8 = 297;
const int INPUT_KEY_F9 = 298;
const int INPUT_KEY_F10 = 299;
const int INPUT_KEY_F11 = 300;
const int INPUT_KEY_F12 = 301;
const int INPUT_KEY_F13 = 302;
const int INPUT_KEY_F14 = 303;
const int INPUT_KEY_F15 = 304;
const int INPUT_KEY_F16 = 305;
const int INPUT_KEY_F17 = 306;
const int INPUT_KEY_F18 = 307;
const int INPUT_KEY_F19 = 308;
const int INPUT_KEY_F20 = 309;
const int INPUT_KEY_F21 = 310;
const int INPUT_KEY_F22 = 311;
const int INPUT_KEY_F23 = 312;
const int INPUT_KEY_F24 = 313;
const int INPUT_KEY_F25 = 314;
const int INPUT_KEY_KP_0 = 320;
const int INPUT_KEY_KP_1 = 321;
const int INPUT_KEY_KP_2 = 322;
const int INPUT_KEY_KP_3 = 323;
const int INPUT_KEY_KP_4 = 324;
const int INPUT_KEY_KP_5 = 325;
const int INPUT_KEY_KP_6 = 326;
const int INPUT_KEY_KP_7 = 327;
const int INPUT_KEY_KP_8 = 328;
const int INPUT_KEY_KP_9 = 329;
const int INPUT_KEY_KP_DECIMAL = 330;
const int INPUT_KEY_KP_DIVIDE = 331;
const int INPUT_KEY_KP_MULTIPLY = 332;
const int INPUT_KEY_KP_SUBTRACT = 333;
const int INPUT_KEY_KP_ADD = 334;
const int INPUT_KEY_KP_ENTER = 335;
const int INPUT_KEY_KP_EQUAL = 336;
const int INPUT_KEY_LEFT_SHIFT = 340;
const int INPUT_KEY_LEFT_CONTROL = 341;
const int INPUT_KEY_LEFT_ALT = 342;
const int INPUT_KEY_LEFT_SUPER = 343;
const int INPUT_KEY_RIGHT_SHIFT = 344;
const int INPUT_KEY_RIGHT_CONTROL = 345;
const int INPUT_KEY_RIGHT_ALT = 346;
const int INPUT_KEY_RIGHT_SUPER = 347;
const int INPUT_KEY_MENU = 348;
const int INPUT_KEY_LAST = INPUT_KEY_MENU;

const int INPUT_MOD_SHIFT = 0x0001;
const int INPUT_MOD_CONTROL = 0x0002;
const int INPUT_MOD_ALT = 0x0004;
const int INPUT_MOD_SUPER = 0x0008;

const int INPUT_MOUSE_BUTTON_1 = 0;
const int INPUT_MOUSE_BUTTON_2 = 1;
const int INPUT_MOUSE_BUTTON_3 = 2;
const int INPUT_MOUSE_BUTTON_4 = 3;
const int INPUT_MOUSE_BUTTON_5 = 4;
const int INPUT_MOUSE_BUTTON_6 = 5;
const int INPUT_MOUSE_BUTTON_7 = 6;
const int INPUT_MOUSE_BUTTON_8 = 7;
const int INPUT_MOUSE_BUTTON_LAST = INPUT_MOUSE_BUTTON_8;
const int INPUT_MOUSE_BUTTON_LEFT = INPUT_MOUSE_BUTTON_1;
const int INPUT_MOUSE_BUTTON_RIGHT = INPUT_MOUSE_BUTTON_2;
const int INPUT_MOUSE_BUTTON_MIDDLE = INPUT_MOUSE_BUTTON_3;


class UserInput :public IInputListener
{
public:
	static UserInput &getInstance();

	UserInput(const UserInput &) = delete;
	UserInput(const UserInput &&) = delete;
	UserInput &operator= (const UserInput &) = delete;
	UserInput &operator= (const UserInput &&) = delete;
	void input();
	glm::vec2 getPreviousMousePos();
	glm::vec2 getCurrentMousePos();
	glm::vec2 getMousePosDelta();
	glm::vec2 getScrollOffset();
	const std::set<int> &getPressedKeys();
	const std::set<int> &getPressedMouseButtons();
	bool isMouseInsideWindow();
	bool isKeyPressed(const int &key);
	bool isMouseButtonPressed(const int &mouseButton);
	void addKeyListener(IKeyListener *_listener);
	void removeKeyListener(IKeyListener *_listener);
	void addCharListener(ICharListener *_listener);
	void removeCharListener(ICharListener *_listener);
	void addScrollListener(IScrollListener *_listener);
	void removeScrollListener(IScrollListener *_listener);
	void addMouseButtonListener(IMouseButtonListener *_listener);
	void removeMouseButtonListener(IMouseButtonListener *_listener);
	template<typename CallbackType>
	void addKeyCallback(CallbackType &&_callback);
	template<typename CallbackType>
	void addCharCallback(CallbackType &&_callback);
	template<typename CallbackType>
	void addScrollCallback(CallbackType &&_callback);
	template<typename CallbackType>
	void addMouseButtonCallback(CallbackType &&_callback);
	void onKey(const int &_key, const int &_action) override;
	void onChar(const int &_charKey) override;
	void onMouseButton(const int &_mouseButton, const int &_action) override;
	void onMouseMove(const double &_x, const double &_y) override;
	void onMouseEnter(const bool &_entered) override;
	void onMouseScroll(const double &_xOffset, const double &_yOffset) override;

private:
	glm::vec2 previousMousePos;
	glm::vec2 currentMousePos;
	glm::vec2 mousePosDelta;
	glm::vec2 scrollOffset;
	bool insideWindow;
	std::vector<IKeyListener*> keyListeners;
	std::vector<ICharListener*> charListeners;
	std::vector<IScrollListener*> scrollListeners;
	std::vector<IMouseButtonListener*> mouseButtonlisteners;
	std::vector<std::function<void(int, int)>> keyCallbacks;
	std::vector<std::function<void(int)>> charCallbacks;
	std::vector<std::function<void(double, double)>> scrollCallbacks;
	std::vector<std::function<void(int, int)>> mouseButtonCallbacks;
	std::set<int> pressedKeys;
	std::set<int> pressedMouseButtons;
	std::set<int> pressedChars;

	UserInput() = default;
};

template<typename CallbackType>
inline void UserInput::addKeyCallback(CallbackType &&_callback)
{
	keyCallbacks.push_back(std::forward<CallbackType>(_callback));
}

template<typename CallbackType>
inline void UserInput::addCharCallback(CallbackType &&_callback)
{
	charCallbacks.push_back(std::forward<CallbackType>(_callback));
}

template<typename CallbackType>
inline void UserInput::addScrollCallback(CallbackType &&_callback)
{
	scrollCallbacks.push_back(std::forward<CallbackType>(_callback));
}

template<typename CallbackType>
inline void UserInput::addMouseButtonCallback(CallbackType &&_callback)
{
	mouseButtonCallbacks.push_back(std::forward<CallbackType>(_callback));
}