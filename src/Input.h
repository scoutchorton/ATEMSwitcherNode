#include <nan.h>
#include <string>

#include "../build/BMDSwitcherAPI_h.h"


#ifndef INPUT_H
#define INPUT_H

namespace ATEMSwitcherNode {
	class Input : public Nan::ObjectWrap {
	public:
		//Constructors
		Input();

		//Node module
		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(New);
		static Nan::Persistent<v8::Function> constructor;
	};
}

#endif /* INPUT_H */