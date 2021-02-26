#include "Switcher.h"
#include <iostream>
#include <comutil.h>

//https://medium.com/netscape/tutorial-building-native-c-modules-for-node-js-using-nan-part-1-755b07389c7c
//https://v8docs.nodesource.com/node-12.19/d2/dc3/namespacev8.html

Nan::Persistent<v8::Function> Switcher::constructor;

Switcher::Switcher() {
	this->discovery = NULL;
	this->switcher = NULL;
	//this->mixBlockIterator = NULL;
};

NAN_MODULE_INIT(Switcher::Init) {
	//Constructor
	v8::Local<v8::FunctionTemplate> SwitcherNew = Nan::New<v8::FunctionTemplate>(New);
	SwitcherNew->SetClassName(Nan::New("Switcher").ToLocalChecked());
	SwitcherNew->InstanceTemplate()->SetInternalFieldCount(1);

	//Class functions
	Nan::SetPrototypeMethod(SwitcherNew, "connect", Connect);
	Nan::SetPrototypeMethod(SwitcherNew, "auto", Auto);
	Nan::SetPrototypeMethod(SwitcherNew, "cut", Cut);
	Nan::SetPrototypeMethod(SwitcherNew, "fadeToBlack", FadeToBlack);

	//Class properties
	SwitcherNew->InstanceTemplate()->Set(target->GetIsolate(), "address", Nan::New("129.168.10.240").ToLocalChecked());
	
	//Exports
	constructor.Reset(Nan::GetFunction(SwitcherNew).ToLocalChecked());
	Nan::Set(target, Nan::New("Switcher").ToLocalChecked(), Nan::GetFunction(SwitcherNew).ToLocalChecked());
};

NAN_METHOD(Switcher::New) {
	//Get context of function
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	//Create instance of class
	Switcher* switcher = new Switcher();

	//Only allow as 'new Switcher()' and not 'Switcher()'
	if (!info.IsConstructCall())
		return Nan::ThrowError("Please use the 'new' operator.");


	//Initalize COM instance
	if(FAILED(CoInitialize(NULL)))
		return Nan::ThrowError("Failed to initalize COM instance.");

	//Initalize Switcher Discovery
	HRESULT instanceResult = CoCreateInstance(CLSID_CBMDSwitcherDiscovery, NULL, CLSCTX_ALL, IID_IBMDSwitcherDiscovery, (void**)&switcher->discovery);
	if(FAILED(instanceResult))
		return Nan::ThrowError("Failed to initalize switcher discovery.");
	

	//Wrap and return object
	switcher->Wrap(info.Holder());
	info.GetReturnValue().Set(info.Holder());
};

NAN_METHOD(Switcher::Connect) {
	//Get context of function
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	Switcher* thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());

	//Parse arguments for address
	if(info.Length() == 1) {
		if(info[0]->IsString())
			thisSwitcher->handle()->Set(context, Nan::New("address").ToLocalChecked(), info[0]);
		else
			return Nan::ThrowError("Expected a string for an address.");
	}

	//Get values from JS class
	v8::Local<v8::Value> addressValue = thisSwitcher->handle()->Get(context, Nan::New("address").ToLocalChecked()).ToLocalChecked();
	std::string addressString(std::string(*v8::String::Utf8Value(info.GetIsolate(), addressValue)));
	

	//Attempt to connect to switcher
	BMDSwitcherConnectToFailure connectionFailure;
	HRESULT connectionResult = thisSwitcher->discovery->ConnectTo(_com_util::ConvertStringToBSTR(addressString.c_str()), &(thisSwitcher->switcher), &connectionFailure);
	if(FAILED(connectionResult)) {
		switch(connectionFailure) {
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureCorruptData:
				return Nan::ThrowError("Connection Failure: Corrupt data received.");
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureIncompatibleFirmware:
				return Nan::ThrowError("Connection Failure: Incompatable firmware.");
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureNoResponse:
				return Nan::ThrowError("Connection Failure: No response.");
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureStateSync:
				return Nan::ThrowError("Connection Failure: Unable to synchronise state with Switcher.");
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureStateSyncTimedOut:
				return Nan::ThrowError("Connection Failure: Unable to synchronize state due to timing out.");
			case BMDSwitcherConnectToFailure::bmdSwitcherConnectToFailureDeprecatedAfter_v7_3:
				return Nan::ThrowError("Connection Failure: Deprecated failure.");
			default:
				return Nan::ThrowError("Connection Failure: Unknown reason for failure.");
		}
	}


	//End code
	std::cout << "Successfully connected to switcher!" << std::endl;
	info.GetReturnValue().Set(Nan::True());
};

NAN_METHOD(Switcher::Auto) {
	//Get context of function
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	Switcher* thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());
	
	//Create mix block iterator if not existant
	IBMDSwitcherMixEffectBlockIterator* mixBlockIterator;
	HRESULT iteratorResult = thisSwitcher->switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (LPVOID*)&mixBlockIterator);
	if(FAILED(iteratorResult)) {
		switch (iteratorResult) {
		case E_POINTER:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		case E_OUTOFMEMORY:
			return Nan::ThrowError("Mix Block Iterator: Not enough memory.");
		case E_NOINTERFACE:
			return Nan::ThrowError("Mix Block Iterator: Interface not found.");
		default:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		}
	}

	//Create mix effect block
	IBMDSwitcherMixEffectBlock* mixBlock = NULL;
	HRESULT mixBlockResult = mixBlockIterator->Next(&mixBlock);
	if(FAILED(mixBlockResult))
		return Nan::ThrowError("Mix Effect Block: Failed to create.");

	//Do transition
	HRESULT autoResult = mixBlock->PerformAutoTransition();
	if(FAILED(autoResult))
		return Nan::ThrowError("Auto Transition: Failed to perform transition.");

	//End code
	return info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(Switcher::Cut) {
	//Get context of function
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	Switcher* thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());

	//Create mix block iterator if not existant
	IBMDSwitcherMixEffectBlockIterator* mixBlockIterator;
	HRESULT iteratorResult = thisSwitcher->switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (LPVOID*)&mixBlockIterator);
	if(FAILED(iteratorResult)) {
		switch (iteratorResult) {
		case E_POINTER:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		case E_OUTOFMEMORY:
			return Nan::ThrowError("Mix Block Iterator: Not enough memory.");
		case E_NOINTERFACE:
			return Nan::ThrowError("Mix Block Iterator: Interface not found.");
		default:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		}
	}

	//Create mix effect block
	IBMDSwitcherMixEffectBlock* mixBlock = NULL;
	HRESULT mixBlockResult = mixBlockIterator->Next(&mixBlock);
	if(FAILED(mixBlockResult))
		return Nan::ThrowError("Mix Effect Block: Failed to create.");

	//Do transition
	HRESULT autoResult = mixBlock->PerformCut();
	if (FAILED(autoResult))
		return Nan::ThrowError("Cut Transition: Failed to perform transition.");

	//End code
	info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(Switcher::FadeToBlack) {
	//Get context of function
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	Switcher* thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());


	//Create mix block iterator if not existant
	IBMDSwitcherMixEffectBlockIterator* mixBlockIterator;
	HRESULT iteratorResult = thisSwitcher->switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (LPVOID*)&mixBlockIterator);
	if(FAILED(iteratorResult)) {
		switch(iteratorResult) {
		case E_POINTER:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		case E_OUTOFMEMORY:
			return Nan::ThrowError("Mix Block Iterator: Not enough memory.");
		case E_NOINTERFACE:
			return Nan::ThrowError("Mix Block Iterator: Interface not found.");
		default:
			return Nan::ThrowError("Mix Block Iterator: Invalid pointer.");
		}
	}

	//Create mix effect block
	IBMDSwitcherMixEffectBlock* mixBlock = NULL;
	HRESULT mixBlockResult = mixBlockIterator->Next(&mixBlock);
	if (FAILED(mixBlockResult))
		return Nan::ThrowError("Mix Effect Block: Failed to create.");

	//Do transition
	HRESULT autoResult = mixBlock->PerformFadeToBlack();
	if (FAILED(autoResult))
		return Nan::ThrowError("Fade to Black: Failed to perform fade to black.");

	//End code
	info.GetReturnValue().Set(Nan::True());
}

NODE_MODULE(ATEMSwitcherNode, Switcher::Init);