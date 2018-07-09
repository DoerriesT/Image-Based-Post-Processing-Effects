#include "GuiInputHandler.h"
#include <cstring>
#include ".\..\..\Input\UserInput.h"


GuiInputHandler::GuiInputHandler()
{
}


GuiInputHandler::~GuiInputHandler()
{
}

void GuiInputHandler::init()
{
	userInput = &UserInput::getInstance();

	userInput->addScrollListener(this);
	userInput->addCharListener(this);
	userInput->addKeyListener(this);

	//glfw_input.last_button_click = 0;
	//glfw_input.is_double_click_down = nk_false;
	//glfw_input.double_click_pos = nk_vec2(0, 0);

	/* glfw.win = win;
	if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
		glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
		glfwSetCharCallback(win, nk_glfw3_char_callback);
		glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
	}*/
	//glfw.ctx.clip.copy = nk_glfw3_clipbard_copy;
	//glfw.ctx.clip.paste = nk_glfw3_clipbard_paste;
	//glfw.ctx.clip.userdata = nk_handle_ptr(0);




}

void GuiInputHandler::input(nk_context *ctx)
{

	nk_input_begin(ctx);

	int i;
	for (i = 0; i < glfw_input.text_len; ++i) {
		nk_input_unicode(ctx, glfw_input.text[i]);
	}
	for (i = 0; i < glfw_input.key_len - 1; i += 2) {
		nk_input_key(ctx, (nk_keys)glfw_input.keys[i], glfw_input.keys[i + 1]);
	}

	//#if NK_GLFW_GL3_MOUSE_GRABBING
	//	/* optional grabbing behavior */
	//	if (ctx->input.mouse.grab)
	//		glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	//	else if (ctx->input.mouse.ungrab)
	//		glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//#endif


	//glfwGetCursorPos(win, &x, &y);
	glm::vec2 pos = userInput->getCurrentMousePos();
	int mx = (int)pos.x;
	int my = (int)pos.y;

	nk_input_motion(ctx, mx, my);

	//#if NK_GLFW_GL3_MOUSE_GRABBING
	//	if (ctx->input.mouse.grabbed) {
	//		glfwSetCursorPos(glfw.win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
	//		ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
	//		ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
	//	}
	//#endif

	nk_input_button(ctx, NK_BUTTON_LEFT, mx, my, userInput->isMouseButtonPressed(InputMouse::BUTTON_LEFT));
	nk_input_button(ctx, NK_BUTTON_MIDDLE, mx, my, userInput->isMouseButtonPressed(InputMouse::BUTTON_MIDDLE));
	nk_input_button(ctx, NK_BUTTON_RIGHT, mx, my, userInput->isMouseButtonPressed(InputMouse::BUTTON_RIGHT));

	nk_input_scroll(ctx, glfw_input.scroll);
	nk_input_end(ctx);
	glfw_input.text_len = 0;
	glfw_input.key_len = 0;
	glfw_input.scroll = nk_vec2(0, 0);

}


//NK_INTERN void nk_glfw3_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
//{
//    const char *text = glfwGetClipboardString(glfw.win);
//    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
//    (void)usr;
//}

//NK_INTERN void nk_glfw3_clipbard_copy(nk_handle usr, const char *text, int len)
//{
//    char *str = 0;
//    (void)usr;
//    if (!len) return;
//    str = (char*)malloc((size_t)len+1);
//    if (!str) return;
//    memcpy(str, text, (size_t)len);
//    str[len] = '\0';
//    glfwSetClipboardString(glfw.win, str);
//    free(str);
//}


void GuiInputHandler::onKey(InputKey _key, InputAction _action)
{
	switch (_key) {
	case InputKey::DELETE:
		inputNkKey(NK_KEY_DEL, _action); break;
	case InputKey::ENTER:
		inputNkKey(NK_KEY_ENTER, _action); break;
	case InputKey::TAB:
		inputNkKey(NK_KEY_TAB, _action); break;
	case InputKey::BACKSPACE:
		inputNkKey(NK_KEY_BACKSPACE, _action); break;
	case InputKey::UP:
		inputNkKey(NK_KEY_UP, _action); break;
	case InputKey::DOWN:
		inputNkKey(NK_KEY_DOWN, _action); break;
	case InputKey::HOME:
		inputNkKey(NK_KEY_TEXT_START, _action);
		inputNkKey(NK_KEY_SCROLL_START, _action); break;
	case InputKey::END:
		inputNkKey(NK_KEY_TEXT_END, _action);
		inputNkKey(NK_KEY_SCROLL_END, _action); break;
	case InputKey::PAGE_DOWN:
		inputNkKey(NK_KEY_SCROLL_DOWN, _action); break;
	case InputKey::PAGE_UP:
		inputNkKey(NK_KEY_SCROLL_UP, _action); break;

	case InputKey::LEFT_SHIFT:
	case InputKey::RIGHT_SHIFT:
		inputNkKey(NK_KEY_SHIFT, _action);
		break;
	}

	if (userInput->isKeyPressed(InputKey::LEFT_SHIFT) || userInput->isKeyPressed(InputKey::RIGHT_SHIFT)) {
		switch (_key) {
		case InputKey::C:
			inputNkKey(NK_KEY_COPY, _action); break;
		case InputKey::V:
			inputNkKey(NK_KEY_PASTE, _action); break;
		case InputKey::X:
			inputNkKey(NK_KEY_CUT, _action); break;
		case InputKey::Z:
			inputNkKey(NK_KEY_TEXT_UNDO, _action); break;
		case InputKey::R:
			inputNkKey(NK_KEY_TEXT_REDO, _action); break;
		case InputKey::LEFT:
			inputNkKey(NK_KEY_TEXT_WORD_LEFT, _action); break;
		case InputKey::RIGHT:
			inputNkKey(NK_KEY_TEXT_WORD_RIGHT, _action); break;
		case InputKey::B:
			inputNkKey(NK_KEY_TEXT_LINE_START, _action); break;
		case InputKey::E:
			inputNkKey(NK_KEY_TEXT_LINE_END, _action); break;
		}
	}
	else
	{
		if (_key == InputKey::LEFT) {
			inputNkKey(NK_KEY_LEFT, _action);
		}
		if (_key == InputKey::RIGHT) {
			inputNkKey(NK_KEY_RIGHT, _action);
		}
		inputNkKey(NK_KEY_COPY, InputAction::RELEASE);
		inputNkKey(NK_KEY_PASTE, InputAction::RELEASE);
		inputNkKey(NK_KEY_CUT, InputAction::RELEASE);
		inputNkKey(NK_KEY_SHIFT, InputAction::RELEASE);
	}
}

void GuiInputHandler::inputNkKey(nk_keys key, InputAction action) {
	if (glfw_input.key_len < NK_GLFW_TEXT_MAX - 3) //-3 because of repeat
	{
		switch (action) {
		case InputAction::RELEASE:
			glfw_input.keys[glfw_input.key_len++] = key;
			glfw_input.keys[glfw_input.key_len++] = 0;
			break;

		case InputAction::REPEAT:
			glfw_input.keys[glfw_input.key_len++] = key;
			glfw_input.keys[glfw_input.key_len++] = 0;
			//no break
		case InputAction::PRESS:
			glfw_input.keys[glfw_input.key_len++] = key;
			glfw_input.keys[glfw_input.key_len++] = 1;
			break;
		}
	}
}

void GuiInputHandler::onChar(InputKey _key) {
	if (glfw_input.text_len < NK_GLFW_TEXT_MAX) {
		glfw_input.text[glfw_input.text_len++] = static_cast<int>(_key);
	}
}

void GuiInputHandler::onScroll(double _xOffset, double _yOffset)
{
	glfw_input.scroll.x += (float)_xOffset;
	glfw_input.scroll.y += (float)_yOffset;
}
