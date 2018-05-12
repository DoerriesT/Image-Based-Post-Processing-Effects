#pragma once
#include "..\nuklearInclude.h"
#include <functional>

#define double_offsetof(s,m1,m2) ((size_t)&reinterpret_cast<char const volatile&>((&(((s*)0)->m1))->m2))
#define triple_offsetof(s,m1,m2, m3) ((size_t)&reinterpret_cast<char const volatile&>((&((&(((s*)0)->m1))->m2))->m3))

class GuiStyleProperty
{
protected:
	size_t offset;
	
	inline void* getPointer(nk_style &style) const
	{
		return (char*)&style + offset;
	}

public:
	GuiStyleProperty(size_t offset);
	virtual ~GuiStyleProperty();

	virtual void apply(nk_style &style) = 0;
	virtual void revert(nk_style &style) const = 0;

	bool operator==(const GuiStyleProperty &rhs)
	{
		return offset == rhs.offset;
	}
};

class GuiStyleFloatProperty : public GuiStyleProperty
{
public:
	GuiStyleFloatProperty(size_t offset, const char* value);
	virtual void apply(nk_style &style) override;
	virtual void revert(nk_style &style) const override;
private:
	float prevValue;
	float value;
};

class GuiStyleColorProperty : public GuiStyleProperty
{
public:
	GuiStyleColorProperty(size_t offset, const char* value);
	virtual void apply(nk_style &style) override;
	virtual void revert(nk_style &style) const override;
private:
	nk_color prevColor;
	nk_color color;
};

class GuiStyleItemProperty : public GuiStyleProperty
{
public:
	GuiStyleItemProperty(size_t offset, const char* value);
	virtual void apply(nk_style &style) override;
	virtual void revert(nk_style &style) const override;
private:
	nk_style_item prevItem;
	nk_style_item item;
};

class GuiStyleSizeProperty : public GuiStyleProperty
{
public:
	GuiStyleSizeProperty(size_t offset, const char* value);
	virtual void apply(nk_style &style) override;
	virtual void revert(nk_style &style) const override;
private:
	struct nk_vec2 prevSize;
	struct nk_vec2 size;
};

class GuiStyleUIntProperty : public GuiStyleProperty
{
public:
	GuiStyleUIntProperty(size_t offset, const char* value);
	virtual void apply(nk_style &style) override;
	virtual void revert(nk_style &style) const override;
private:
	uint32_t prevValue;
	uint32_t value;
};