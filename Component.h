#pragma once
#include <QString>

class Component;

#include "GameObject.h"

class Component
{
private:
	QString     instanceID;
	bool        enabled;
public:
	GameObject* gameObject;
	QString name;

	inline QString getInstanceID() const
	{
		return this->instanceID;
	}

	inline bool isEnabled() const
	{
		return this->enabled;
	}

	inline void setEnabled(bool value)
	{
		this->enabled = value;
	}
};