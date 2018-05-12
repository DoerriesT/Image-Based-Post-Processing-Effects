#pragma once
#include "GuiRenderer.h"
#include "..\nuklearInclude.h"


class GuiFontManager
{
public:
	GuiFontManager();
	~GuiFontManager();
	void init(GuiRenderer *renderer);
	nk_user_font* getFont(enum GuiSize size);

	static float getTextWidth(const nk_user_font *font, const char *text);
	static float getTextWidth(const nk_context *ctx, const char *text);
	static float getTextWidth(const nk_context *ctx, const std::string &text);

	static int wrappedTextLines(const nk_context *ctx, const std::string &text, float maxLineWidth);
	static std::vector<std::string> splitCharsAndWhitespaces(const std::string &str);

private:
	struct nk_font_atlas *atlas;
	struct nk_font** fonts;
};



