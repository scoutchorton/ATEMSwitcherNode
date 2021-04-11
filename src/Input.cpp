#include "Input.h"

#include <comutil.h>
#include <iostream>


namespace ATEMSwitcherNode {
	Nan::Persistent<v8::Function> Input::constructor;

	Input::Input() {

	}

	NAN_MODULE_INIT(Input::Init) {
		//Constructor
		v8::Local<v8::FunctionTemplate> InputNew = Nan::New<v8::FunctionTemplate>(New);
		InputNew->SetClassName(Nan::New("Input").ToLocalChecked());
		InputNew->InstanceTemplate()->SetInternalFieldCount(1);

		//Class functions

		//Class properties
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "id", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "type", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "availability", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "shortName", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "longName", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "defaultNames", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "previewTallied", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "programTallied", Nan::Undefined());
		InputNew->InstanceTemplate()->Set(target->GetIsolate(), "externalPortType", Nan::Undefined());

		//Exports
		constructor.Reset(Nan::GetFunction(InputNew).ToLocalChecked());
		Nan::Set(target, Nan::New("Input").ToLocalChecked(), Nan::GetFunction(InputNew).ToLocalChecked());
	}

	NAN_METHOD(Input::New) {
		/**
		 * TODO: APPLY DATA IN INPUT'S NEW METHOD
		 */

		//Get context of function
		v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

		//Create instance of class
		Input* input = new Input();

		//Only allow constructing of class
		if(!info.IsConstructCall())
			return Nan::ThrowError("Please use the 'new' operator.");
		
		//Wrap and return object
		input->Wrap(info.Holder());
		info.GetReturnValue().Set(info.Holder());
	}
}