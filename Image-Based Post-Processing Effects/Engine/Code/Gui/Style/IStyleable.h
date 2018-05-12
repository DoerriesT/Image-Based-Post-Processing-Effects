#pragma once
#include <string>
#include "GuiStyleBlock.h"
#include "..\nuklearInclude.h"

class GuiStyleBlock;

class IStyleable
{
public:
	IStyleable();
	~IStyleable();
	GuiStyleBlock *getStyleBlock() const;
	void setStyleClass(const std::string &name);
	void clearStyleClass();
	void updateStyleBlock();

protected:
	GuiStyleBlock *styleBlock;
	std::string className;

	inline void applyStyle(nk_context *ctx) const;
	inline void revertStyle(nk_context *ctx) const;
};


inline void IStyleable::applyStyle(nk_context *ctx) const
{
	if (styleBlock)
	{
		styleBlock->apply(ctx->style);
	}
}

inline void IStyleable::revertStyle(nk_context *ctx) const
{
	if (styleBlock)
	{
		styleBlock->revert(ctx->style);
	}
}