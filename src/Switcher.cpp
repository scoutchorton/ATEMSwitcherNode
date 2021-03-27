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
	Nan::SetPrototypeMethod(SwitcherNew, "getInputs", GetInputs);

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
	if(!info.IsConstructCall())
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
	//Variables
	v8::Local<v8::Context> context;
	Switcher* thisSwitcher;
	v8::Local<v8::Value> addressValue;
	std::string addressString;
	BMDSwitcherConnectToFailure connectionFailure;
	HRESULT connectionResult;

	//Get context of function
	context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());

	//Parse arguments for address
	if(info.Length() == 1) {
		if(info[0]->IsString())
			thisSwitcher->handle()->Set(context, Nan::New("address").ToLocalChecked(), info[0]);
		else
			return Nan::ThrowError("Expected a string for an address.");
	}

	//Get values from JS class
	addressValue = thisSwitcher->handle()->Get(context, Nan::New("address").ToLocalChecked()).ToLocalChecked();
	addressString = std::string(std::string(*v8::String::Utf8Value(info.GetIsolate(), addressValue)));
	

	//Attempt to connect to switcher
	connectionResult = thisSwitcher->discovery->ConnectTo(_com_util::ConvertStringToBSTR(addressString.c_str()), &(thisSwitcher->switcher), &connectionFailure);
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
	//Variables
	v8::Local<v8::Context> context;
	Switcher* thisSwitcher;
	IBMDSwitcherMixEffectBlockIterator* mixBlockIterator;
	HRESULT iteratorResult;
	IBMDSwitcherMixEffectBlock* mixBlock;
	HRESULT mixBlockResult;
	HRESULT autoResult;

	//Get context of function
	context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());

	//Create mix block iterator if not existant
	iteratorResult = thisSwitcher->switcher->CreateIterator(IID_IBMDSwitcherMixEffectBlockIterator, (LPVOID*)&mixBlockIterator);
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
	mixBlock = NULL;
	mixBlockResult = mixBlockIterator->Next(&mixBlock);
	if(FAILED(mixBlockResult))
		return Nan::ThrowError("Mix Effect Block: Failed to create.");

	//Do transition
	autoResult = mixBlock->PerformCut();
	if(FAILED(autoResult))
		return Nan::ThrowError("Cut Transition: Failed to perform transition.");

	//End code
	info.GetReturnValue().Set(Nan::Undefined());
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
	if(FAILED(mixBlockResult))
		return Nan::ThrowError("Mix Effect Block: Failed to create.");

	//Do transition
	HRESULT autoResult = mixBlock->PerformFadeToBlack();
	if(FAILED(autoResult))
		return Nan::ThrowError("Fade to Black: Failed to perform fade to black.");

	//End code
	info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Switcher::GetInputs) {
	//Variables
	v8::Local<v8::Context> context;
	Switcher* thisSwitcher;
	HRESULT result;
	IBMDSwitcherInputIterator* inputIterator;
	IBMDSwitcherInput* input;
	v8::Local<v8::Array> inputArray;
	v8::Local<v8::Object> inputObject;
	
	//Get context of the funcation
	context = info.GetIsolate()->GetCurrentContext();

	//Get Switcher instance
	thisSwitcher = Nan::ObjectWrap::Unwrap<Switcher>(info.This());

	//Create input iterator if not existant
	result = thisSwitcher->switcher->CreateIterator(IID_IBMDSwitcherInputIterator, (LPVOID*)&inputIterator);
	if(FAILED(result)) {
		switch(result) {
		case E_POINTER:
			return Nan::ThrowError("Input Iterator: Invalid pointer.");
		case E_OUTOFMEMORY:
			return Nan::ThrowError("Input Iterator: Not enough memory.");
		case E_NOINTERFACE:
			return Nan::ThrowError("Input Iterator: Interface not found.");
		default:
			return Nan::ThrowError("Input Iterator: Invalid pointer.");
		}
	}

	/**
	 * Iterate over all inputs, and make a JS array to store information about them.
	 * Make a custom class for inputs?
	 */

	//Iterate over inputs and get names
	result = inputIterator->Next(&input);
	if(result == E_POINTER)
		return Nan::ThrowError("Input Iteration: Invalid pointer.");

	//Add names to inputArray
	inputArray = v8::Array::New(info.GetIsolate());
	for(; result != S_FALSE; result = inputIterator->Next(&input)) {
		//Initalize variables
		inputObject = v8::Object::New(info.GetIsolate());
		BMDSwitcherInputId id;
		BMDSwitcherPortType portType;
		BMDSwitcherInputAvailability availability;
		BSTR lname;
		BSTR sname;
		BOOL isDefault;
		BOOL prgmTallied;
		BOOL prvwTallied;
		BMDSwitcherExternalPortType externPortTypes;

		//Get and apply attributes
		if(input->GetInputId(&id) == S_OK)
			inputObject->Set(context, Nan::New("id").ToLocalChecked(), Nan::New(std::to_string(id)).ToLocalChecked());
		else
			inputObject->Set(context, Nan::New("id").ToLocalChecked(), Nan::Undefined());

		if(input->GetPortType(&portType) == S_OK) {
			switch(portType) {
			case bmdSwitcherPortTypeExternal:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("External").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeBlack:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("Black").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeColorBars:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("ColorBars").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeColorGenerator:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("ColorGenerator").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeMediaPlayerFill:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("MediaPlayerFill").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeMediaPlayerCut:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("MediaPlayerCut").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeSuperSource:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("SuperSource").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeMixEffectBlockOutput:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("MixEffectBlockOutput").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeAuxOutput:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("AuxOutput").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeKeyCutOutput:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("CutOutput").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeMultiview:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("Multiview").ToLocalChecked());
				break;
			case bmdSwitcherPortTypeExternalDirect:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::New("ExternalDirect").ToLocalChecked());
				break;
			default:
				inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::Undefined());
			}
		} else
			inputObject->Set(context, Nan::New("type").ToLocalChecked(), Nan::Undefined());

		if(input->GetInputAvailability(&availability) == S_OK) {
			switch(availability) {
			case bmdSwitcherInputAvailabilityMixEffectBlock0:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("MixEffectBlock0").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityMixEffectBlock1:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("MixEffectBlock1").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityMixEffectBlock2:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("MixEffectBlock2").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityMixEffectBlock3:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("MixEffectBlock3").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityAux1Output:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("Aux1Output").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityAux2Output:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("Aux2Output").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityAuxOutputs:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("AuxOutputs").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityMultiView:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("MultiView").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilitySuperSourceArt:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("SuperSourceArt").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilitySuperSourceBox:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("SuperSourceBox").ToLocalChecked());
				break;
			case bmdSwitcherInputAvailabilityInputCut:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::New("InputCut").ToLocalChecked());
				break;
			default:
				inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::Undefined());
			}
		} else
			inputObject->Set(context, Nan::New("availability").ToLocalChecked(), Nan::Undefined());

		if(input->GetLongName(&sname) == S_OK)
			inputObject->Set(context, Nan::New("shortName").ToLocalChecked(), Nan::New(_com_util::ConvertBSTRToString(sname)).ToLocalChecked());
		else
			inputObject->Set(context, Nan::New("shortName").ToLocalChecked(), Nan::Undefined());
		
		if(input->GetShortName(&lname) == S_OK)
			inputObject->Set(context, Nan::New("longName").ToLocalChecked(), Nan::New(_com_util::ConvertBSTRToString(lname)).ToLocalChecked());
		else
			inputObject->Set(context, Nan::New("longName").ToLocalChecked(), Nan::Undefined());
		
		if(input->AreNamesDefault(&isDefault) == S_OK)
			inputObject->Set(context, Nan::New("defaultNames").ToLocalChecked(), (isDefault) ? Nan::True() : Nan::False());
		else
			inputObject->Set(context, Nan::New("defaultNames").ToLocalChecked(), Nan::Undefined());
		
		if(input->IsProgramTallied(&prgmTallied) == S_OK)
			inputObject->Set(context, Nan::New("programTallied").ToLocalChecked(), (prgmTallied) ? Nan::True() : Nan::False());
		else
			inputObject->Set(context, Nan::New("programTallied").ToLocalChecked(), Nan::Undefined());
		
		if(input->IsPreviewTallied(&prvwTallied) == S_OK)
			inputObject->Set(context, Nan::New("previewTallied").ToLocalChecked(), (prvwTallied) ? Nan::True() : Nan::False());
		else
			inputObject->Set(context, Nan::New("previewTallied").ToLocalChecked(), Nan::Undefined());
		
		if(input->GetAvailableExternalPortTypes(&externPortTypes) == S_OK) {
			switch(externPortTypes) {
			case bmdSwitcherExternalPortTypeSDI:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("SDI").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeHDMI:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("HDMI").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeComponent:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("Component").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeComposite:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("Composite").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeSVideo:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("SVideo").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeInternal:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("Internal").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeXLR:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("XLR").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeAESEBU:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("AESEBU").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeRCA:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("RCA").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeTSJack:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("TSJack").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeMADI:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("MADI").ToLocalChecked());
				break;
			case bmdSwitcherExternalPortTypeTRS:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::New("TRS").ToLocalChecked());
				break;
			default:
				inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::Undefined());
			}
		} else
			inputObject->Set(context, Nan::New("externalPortType").ToLocalChecked(), Nan::Undefined());

		//Add to return array
		inputArray->Set(context, inputArray->Length(), inputObject);
	}


	//End code
	info.GetReturnValue().Set(inputArray);
}

NODE_MODULE(ATEMSwitcherNode, Switcher::Init);