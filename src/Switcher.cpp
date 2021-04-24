#include "Switcher.h"
#include "Input.h"

#include <comutil.h>

#include <iostream>

#define DEBUG 1

//https://medium.com/netscape/tutorial-building-native-c-modules-for-node-js-using-nan-part-1-755b07389c7c
//https://v8docs.nodesource.com/node-12.19/d2/dc3/namespacev8.html

namespace ATEMSwitcherNode {
	NAN_MODULE_INIT(Init) {
		//Initalize classes
		Switcher::Init(target);
		Input::Init(target);
	}

	Nan::Persistent<v8::Function> Switcher::constructor;

	Switcher::Switcher() {
		this->discovery = NULL;
		this->switcher = NULL;
		//this->mixBlockIterator = NULL;
	}

	NAN_MODULE_INIT(Switcher::Init) {
		v8::Local<v8::FunctionTemplate> SwitcherNew;

		//Constructor
		SwitcherNew = Nan::New<v8::FunctionTemplate>(New);
		SwitcherNew->SetClassName(Nan::New("Switcher").ToLocalChecked());
		SwitcherNew->InstanceTemplate()->SetInternalFieldCount(1);

		//Class functions
		Nan::SetPrototypeMethod(SwitcherNew, "connect", Connect);
		Nan::SetPrototypeMethod(SwitcherNew, "auto", Auto);
		Nan::SetPrototypeMethod(SwitcherNew, "cut", Cut);
		Nan::SetPrototypeMethod(SwitcherNew, "fadeToBlack", FadeToBlack);
		Nan::SetPrototypeMethod(SwitcherNew, "getInputs", GetInputs);
		Nan::SetPrototypeMethod(SwitcherNew, "setPreview", SetPreview);

		//Class properties
		SwitcherNew->InstanceTemplate()->Set(target->GetIsolate(), "address", Nan::New("129.168.10.240").ToLocalChecked());
		SwitcherNew->InstanceTemplate()->Set(target->GetIsolate(), "inputs", Nan::Undefined());

		//Exports
		constructor.Reset(Nan::GetFunction(SwitcherNew).ToLocalChecked());
		Nan::Set(target, Nan::New("Switcher").ToLocalChecked(), Nan::GetFunction(SwitcherNew).ToLocalChecked());
	};

	NAN_METHOD(Switcher::New) {
		v8::Local<v8::Context> context;
		Switcher* switcher;
		HRESULT instanceResult;

		//Get context of function
		context = info.GetIsolate()->GetCurrentContext();

		//Create instance of class
		switcher = new Switcher();

		//Only allow as 'new Switcher()' and not 'Switcher()'
		if(!info.IsConstructCall())
			return Nan::ThrowError("Please use the 'new' operator.");

		//Initalize COM instance
		if(FAILED(CoInitialize(NULL)))
			return Nan::ThrowError("Failed to initalize COM instance.");

		//Initalize Switcher Discovery
		instanceResult = CoCreateInstance(CLSID_CBMDSwitcherDiscovery, NULL, CLSCTX_ALL, IID_IBMDSwitcherDiscovery, (void**)&switcher->discovery);
		if(FAILED(instanceResult))
			return Nan::ThrowError("Failed to initalize switcher discovery.");

		//Setup inputs array
		//v8::Local<v8::Array> inputArray = v8::Array::New(info.GetIsolate());
		//switcher->handle()->Set(context, Nan::New("inputs"), inputArray);

		//Wrap and return object
		switcher->Wrap(info.Holder());
		info.GetReturnValue().Set(info.Holder());
	}

	NAN_METHOD(Switcher::Connect) {
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
		addressString = std::string(*v8::String::Utf8Value(info.GetIsolate(), addressValue));


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

		//Automagically populate inputs array
		//v8::Local<v8::Function>::Cast(thisSwitcher->handle()->Get(context, Nan::New("GetInputs").ToLocalChecked()).ToLocalChecked())->Call(context, Nan::Undefined(), 0, NULL);

		//End code
		info.GetReturnValue().Set(Nan::True());
	};

	NAN_METHOD(Switcher::Auto) {
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
		autoResult = mixBlock->PerformAutoTransition();
		if(FAILED(autoResult))
			return Nan::ThrowError("Auto Transition: Failed to perform transition.");

		//End code
		return info.GetReturnValue().Set(Nan::True());
	}

	NAN_METHOD(Switcher::Cut) {
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
		autoResult = mixBlock->PerformFadeToBlack();
		if(FAILED(autoResult))
			return Nan::ThrowError("Fade to Black: Failed to perform fade to black.");

		//End code
		info.GetReturnValue().Set(Nan::Undefined());
	}

	NAN_METHOD(Switcher::GetInputs) {
		v8::Local<v8::Context> context;
		Switcher* thisSwitcher;
		HRESULT result;
		IBMDSwitcherInputIterator* inputIterator;
		IBMDSwitcherInput* input;
		v8::Local<v8::Array> inputArray;
		v8::Local<v8::Value> switcherInputArray;
		int inputIndex;

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

		 //Iterate over inputs and get names
		result = inputIterator->Next(&input);
		if(result == E_POINTER)
			return Nan::ThrowError("Input Iteration: Invalid pointer.");

		//Set up array object
		if(thisSwitcher->handle()->Get(context, Nan::New("inputs").ToLocalChecked()).ToLocalChecked()->IsArray())
			inputArray = v8::Local<v8::Array>::Cast(thisSwitcher->handle()->Get(context, Nan::New("inputs").ToLocalChecked()).ToLocalChecked());
		else
			inputArray = v8::Array::New(info.GetIsolate());

		//Add names to inputArray
		for(inputIndex = 0; result != S_FALSE; result = inputIterator->Next(&input), inputIndex++) {
			BMDSwitcherInputId inputId;
			BMDSwitcherPortType portType;
			BMDSwitcherInputAvailability avail;
			BSTR lname;
			BSTR sname;
			BOOL isDefault;
			BOOL prgmTallied;
			BOOL prvwTallied;
			BMDSwitcherExternalPortType externPortTypes;
			v8::Local<v8::Value> inputArgs[Input::inputArgc];
			v8::Local<v8::Object> inputObject;
			v8::Local<v8::Array> availArray;
			
			//Get and apply attributes
			if(input->GetPortType(&portType) == S_OK) {
				switch(portType) {
				case bmdSwitcherPortTypeExternal:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "External", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeBlack:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "Black", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeColorBars:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "ColorBars", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeColorGenerator:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "ColorGenerator", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeMediaPlayerFill:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "MediaPlayerFill", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeMediaPlayerCut:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "MediaPlayerCut", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeSuperSource:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "SuperSource", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeMixEffectBlockOutput:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlockOutput", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeAuxOutput:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "AuxOutput", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeKeyCutOutput:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "CutOutput", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeMultiview:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "Multiview", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherPortTypeExternalDirect:
					inputArgs[Input::type] = v8::String::NewFromUtf8(info.GetIsolate(), "ExternalDirect", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				default:
					inputArgs[Input::type] = Nan::Undefined();
				}
			} else
				inputArgs[Input::type] = Nan::Undefined();

			availArray = v8::Array::New(info.GetIsolate());
			if(input->GetInputAvailability(&avail) == S_OK) {
				if(avail & bmdSwitcherInputAvailabilityMixEffectBlock0)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock0", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityMixEffectBlock1)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock1", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityMixEffectBlock2)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock2", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityMixEffectBlock3)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock3", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityAux1Output)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "Aux1Output", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityAux2Output)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "Aux2Output", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityAuxOutputs)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "AuxOutputs", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityMultiView)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "MultiView", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilitySuperSourceArt)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "SuperSourceArt", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilitySuperSourceBox)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "SuperSourceBox", v8::NewStringType::kNormal).ToLocalChecked());
				if(avail & bmdSwitcherInputAvailabilityInputCut)
					availArray->Set(context, availArray->Length(), v8::String::NewFromUtf8(info.GetIsolate(), "InputCut", v8::NewStringType::kNormal).ToLocalChecked());
			}
			inputArgs[Input::availability] = availArray;

			if(input->GetAvailableExternalPortTypes(&externPortTypes) == S_OK) {
				switch(externPortTypes) {
				case bmdSwitcherExternalPortTypeSDI:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "SDI", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeHDMI:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "HDMI", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeComponent:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "Component", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeComposite:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "Composite", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeSVideo:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "SVideo", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeInternal:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "Internal", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeXLR:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "XLR", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeAESEBU:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "AESEBU", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeRCA:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "RCA", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeTSJack:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "TSJack", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeMADI:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "MADI", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherExternalPortTypeTRS:
					inputArgs[Input::externalPortType] = v8::String::NewFromUtf8(info.GetIsolate(), "TRS", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				default:
					inputArgs[Input::externalPortType] = Nan::Undefined();
				}
			} else
				inputArgs[Input::externalPortType] = Nan::Undefined();

			if(input->GetInputId(&inputId) == S_OK)
				inputArgs[Input::id] = v8::String::NewFromUtf8(info.GetIsolate(), std::to_string(inputId).c_str(), v8::NewStringType::kNormal).ToLocalChecked();
			else
				inputArgs[Input::id] = Nan::Undefined();

			if((input->GetShortName(&sname) == S_OK))
				inputArgs[Input::shortName] = v8::String::NewFromUtf8(info.GetIsolate(), _com_util::ConvertBSTRToString(sname), v8::NewStringType::kNormal).ToLocalChecked();
			else
				inputArgs[Input::shortName] = Nan::Undefined();
			
			if(input->GetLongName(&lname) == S_OK)
				inputArgs[Input::longName] = v8::String::NewFromUtf8(info.GetIsolate(), _com_util::ConvertBSTRToString(lname), v8::NewStringType::kNormal).ToLocalChecked();
			else
				inputArgs[Input::longName] = Nan::Undefined();
			
			if(input->AreNamesDefault(&isDefault) == S_OK && isDefault)
				inputArgs[Input::defaultNames] = Nan::True();
			else
				inputArgs[Input::defaultNames] = Nan::False();
			
			if(input->IsProgramTallied(&prgmTallied) == S_OK && prgmTallied)
				inputArgs[Input::programTallied] = Nan::True();
			else
				inputArgs[Input::programTallied] = Nan::False();
			
			if(input->IsPreviewTallied(&prvwTallied) == S_OK && prvwTallied)
				inputArgs[Input::previewTallied] = Nan::True();
			else
				inputArgs[Input::previewTallied] = Nan::False();

			//Create an instance of an Input
			inputObject = Nan::New(Input::constructor)->NewInstance(context, Input::inputArgc, inputArgs).ToLocalChecked();
			
			//Add to return array
			inputArray->Set(context, inputIndex, inputObject);
		}

		//End code
		thisSwitcher->handle()->Set(context, Nan::New("inputs").ToLocalChecked(), inputArray);
		info.GetReturnValue().Set(inputArray);
	}

	NAN_METHOD(Switcher::SetPreview) {
		//End code
		info.GetReturnValue().Set(Nan::Undefined());
	}

	NODE_MODULE(ATEMSwitcherNode, Init);
}