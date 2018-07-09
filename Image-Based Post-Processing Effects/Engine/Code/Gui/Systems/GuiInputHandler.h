#pragma once
#include "..\nuklearInclude.h"
#include ".\..\..\Input\IKeyListener.h"
#include ".\..\..\Input\ICharListener.h"
#include ".\..\..\Input\IMouseButtonListener.h"
#include ".\..\..\Input\IScrollListener.h"


#define NK_GLFW_TEXT_MAX 256

#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#define NK_GLFW_DOUBLE_CLICK_HI 0.2

class UserInput;


struct nk_glfw_input 
{
	unsigned int text[NK_GLFW_TEXT_MAX];
	int text_len;
	//double last_button_click;
	//int is_double_click_down;
	struct nk_vec2 scroll;
	//struct nk_vec2 double_click_pos;
	unsigned int keys[NK_GLFW_TEXT_MAX];
	int key_len;
} ;

class GuiInputHandler : private IKeyListener, private ICharListener, private IScrollListener
{
public:
	GuiInputHandler();
	~GuiInputHandler();
	void init();
	void input(nk_context *ctx);

private:
	nk_glfw_input glfw_input;
	UserInput *userInput;

	void inputNkKey(nk_keys key, InputAction action);

	void onKey(InputKey _key, InputAction _action) override;
	void onChar(InputKey _charKey) override;
	void onScroll(double _xOffset, double _yOffset) override;
};

