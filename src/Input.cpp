#include "Input.h"

#include <comutil.h>
#include <iostream>


namespace ATEMSwitcherNode {
	Nan::Persistent<v8::Function> Input::constructor;

	Input::Input() {}

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
		//Get context of function
		v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

		//Create instance of class
		Input* input = new Input();

		//Only allow constructing of class
		if(!info.IsConstructCall())
			return Nan::ThrowError("Please use the 'new' operator.");

		//Check for argument count
		if(info.Length() != Input::inputArgc)
			return Nan::ThrowError("Not enough arguments provided.");

		//Apply parameters to instance
		for(int index = 0; index < inputArgc; index++) {
			switch(index) {
			case Input::id:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "id", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::type:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "type", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::availability:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "availability", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::shortName:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "shortName", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::longName:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "longName", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::defaultNames:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "defaultNames", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::previewTallied:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "previewTallied", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::programTallied:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "programTallied", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			case Input::externalPortType:
				info.Holder()->Set(context, v8::String::NewFromUtf8(info.GetIsolate(), "externalPortType", v8::NewStringType::kNormal).ToLocalChecked(), info[index]);
				break;
			default:
				return Nan::ThrowError("Inavlid argument.");
			}
		}
		
		//Wrap and return object
		input->Wrap(info.Holder());
		info.GetReturnValue().Set(info.Holder());
	}
}