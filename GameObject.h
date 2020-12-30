#pragma once
#include <QTransform>

class GameObject;
#include "Component.h"

class GameObject
{
private:
	bool                   active;
	std::vector<Component> components;
public:
	QString                name;
	QTransform             transform;
	
	inline bool isActive() const
	{
		return this->active;
	}

	inline void setActive(bool value)
	{
		this->active = value;
	}
};

