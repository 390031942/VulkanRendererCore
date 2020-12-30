#pragma once

class AbstractRenderer/*:public AbstractSubSystem*/
{
public:
	virtual bool startUp() = 0;
	//virtual bool readConfig() = 0;
	virtual bool shutdown() = 0;

	/*
	virtual bool add_BasicShape_Triangle() = 0;
	virtual bool add_BasicShape_Ellipse() = 0;
	virtual bool add_BasicShape_Quadrangle() = 0;
	virtual bool add_BasicShape_Line() = 0;

	virtual bool bind_Texture() = 0;
	virtual bool test_Hit() = 0;
	virtual bool update_Frame() = 0;

	virtual bool update_Transform_Translate() = 0;
	virtual bool update_Transform_Rotate() = 0;
	virtual bool update_Transform_Scale() = 0;
	*/

};