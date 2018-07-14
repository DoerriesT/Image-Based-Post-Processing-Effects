#include "GuiFontManager.h"
#include "Utilities\Utility.h"
#include "..\Gui.h"
#include <cctype>

GuiFontManager::GuiFontManager()
{
	atlas = new nk_font_atlas();
	fonts = new nk_font*[GUI_SIZE_COUNT];
}


GuiFontManager::~GuiFontManager()
{
	nk_font_atlas_clear(atlas);
	delete[] fonts;
	delete atlas;
}

void GuiFontManager::init(GuiRenderer *renderer)
{
	renderer->fontStashBegin(atlas);

	std::vector<char> buffer = Utility::readBinaryFile("Resources/Gui/Fonts/FiraSans.ttf");
	if (!buffer.empty())
	{
		for (uint32_t i = 0; i < GUI_SIZE_COUNT; ++i)
		{
			float fontSize = Gui::fontSizes[i];
			struct nk_font_config config = nk_font_config(fontSize);
			config.oversample_h = 4;
			config.oversample_v = 4;
			fonts[i] = nk_font_atlas_add_from_memory(atlas, buffer.data(), (nk_size)buffer.size(), fontSize, &config);
		}
	}
	renderer->fontStashEnd(atlas);
}

nk_user_font* GuiFontManager::getFont(GuiSize size)
{
	return &fonts[size]->handle;
}

float GuiFontManager::getTextWidth(const nk_user_font *font, const char *text)
{
	return font->width(font->userdata, font->height, text, (int)strlen(text));
}

float GuiFontManager::getTextWidth(const nk_context *ctx, const char *text)
{
	return getTextWidth(ctx->style.font, text);
}

float GuiFontManager::getTextWidth(const nk_context *ctx, const std::string &text)
{
	return getTextWidth(ctx, text.c_str());
}

int GuiFontManager::wrappedTextLines(const nk_context *ctx, const std::string &text, float maxLineWidth)
{

	std::vector<std::string> list = splitCharsAndWhitespaces(text);
	int lines = 1;
	float availableWidth = maxLineWidth;
	for (const std::string &str : list)
	{
		float length = GuiFontManager::getTextWidth(ctx, str);
		// word is larger than one whole line
		while (length >= maxLineWidth)
		{
			if (availableWidth != maxLineWidth)
			{
				// add an extra line, if the line isn't complete anymore
				++lines;
			}
			length -= maxLineWidth;
			++lines;
			availableWidth = maxLineWidth;
		}

		// word doesn't fit in line anymore
		if (length > availableWidth)
		{
			++lines;
			availableWidth = maxLineWidth;
		}
		// substract word from current line
		availableWidth -= length;
	}

	return lines;
}

std::vector<std::string> GuiFontManager::splitCharsAndWhitespaces(const std::string &str)
{
	std::vector<std::string> bufferlist;

	std::string buffer;
	bool lastCharWasWhitespace = false;
	for (int ch : str)
	{
		bool isWhitespace = ch >=0 && ch <= 255 && std::isspace(ch);
		if (isWhitespace || lastCharWasWhitespace)
		{
			if (!buffer.empty())
			{
				bufferlist.push_back(buffer);
				buffer.clear();
			}
		}
		buffer += ch;
		lastCharWasWhitespace = isWhitespace;
	}

	if (!buffer.empty())
	{
		bufferlist.push_back(buffer);
	}

	return bufferlist;
}