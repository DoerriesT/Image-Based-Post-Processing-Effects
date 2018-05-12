#pragma once

struct Entity;

class IOnEntityDestructionListener
{
public:
	virtual void onDestruction(const Entity *_entity) = 0;
};