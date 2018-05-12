#pragma once

struct Entity;

class IOnEntityCreatedListener
{
public:
	virtual void onEntityCreated(const Entity *_entity) = 0;
};