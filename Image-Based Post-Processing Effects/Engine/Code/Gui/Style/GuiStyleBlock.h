#pragma once
#include <vector>
#include "GuiStyleProperty.h"
#include "..\nuklearInclude.h"

class GuiStyleSheet;

class GuiStyleBlock
{
	friend GuiStyleSheet;
private:
	std::vector<GuiStyleProperty*> properties;

public:
	GuiStyleBlock();
	~GuiStyleBlock();

	void apply(nk_style &style);
	void revert(nk_style &style) const;

	inline void apply(nk_context *ctx);
	inline void revert(nk_context *ctx) const;

	void addProperty(GuiStyleProperty *p);
};


inline void GuiStyleBlock::apply(nk_context *ctx)
{
	apply(ctx->style);
}

inline void GuiStyleBlock::revert(nk_context *ctx) const
{
	revert(ctx->style);
}