#include <nan.h>
#include <string>

#include "../build/BMDSwitcherAPI_h.h"


#ifndef SWITCHER_H
#define SWITCHER_H

class Switcher : public Nan::ObjectWrap {
public:
	//Constructors
	Switcher();

	//Members

	//Node Module
	static NAN_MODULE_INIT(Init);
	static NAN_METHOD(New);
	static NAN_METHOD(Connect);
	static NAN_METHOD(Auto);
	static NAN_METHOD(Cut);
	static NAN_METHOD(FadeToBlack);
	static Nan::Persistent<v8::Function> constructor;

private:
	IBMDSwitcherDiscovery* discovery;
	IBMDSwitcher* switcher;
};

#endif