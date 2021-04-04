#include <nan.h>
#include <string>

#include "../build/BMDSwitcherAPI_h.h"


#ifndef SWITCHER_H
#define SWITCHER_H

namespace ATEMSwitcherNode {
	NAN_MODULE_INIT(Init);

	class Switcher : public Nan::ObjectWrap {
	public:
		//Constructors
		Switcher();

		//Node module
		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(New);
		static Nan::Persistent<v8::Function> constructor;

		//Networking
		static NAN_METHOD(Connect);

		//Transitions
		static NAN_METHOD(Auto);
		static NAN_METHOD(Cut);
		static NAN_METHOD(FadeToBlack);

		//Util
		static NAN_METHOD(GetInputs);

	private:
		IBMDSwitcherDiscovery* discovery;
		IBMDSwitcher* switcher;
	};
}

#endif /* SWITCHER_H */