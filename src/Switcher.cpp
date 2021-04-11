#include "Switcher.h"
#include "Input.h"

#include <comutil.h>

#include <iostream>

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

			//Create an instance of an Input
			//inputObject = Nan::NewInstance(Nan::New(Input::constructor), inputArgc, inputArgs).ToLocalChecked();
			//inputObject = v8::Object::New(info.GetIsolate());
			
			//Get and apply attributes
			//inputArgs[Input::id] = (input->GetInputId(&inputId) == S_OK) ? v8::String::NewFromUtf8(info.GetIsolate(), std::to_string(inputId).c_str()) : Nan::Undefined();

			inputArgs[Input::type] = Nan::Undefined();
			std::cout << "Undefined: " << *v8::String::Utf8Value(info.GetIsolate(), inputArgs[Input::type]) << std::endl;
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
			std::cout << "Value after GetPortType: " << *v8::String::Utf8Value(info.GetIsolate(), inputArgs[Input::type]) << std::endl;

			if(input->GetInputAvailability(&avail) == S_OK) {
				switch(avail) {
				case bmdSwitcherInputAvailabilityMixEffectBlock0:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock0", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityMixEffectBlock1:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock1", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityMixEffectBlock2:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock2", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityMixEffectBlock3:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "MixEffectBlock3", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityAux1Output:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "Aux1Output", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityAux2Output:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "Aux2Output", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityAuxOutputs:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "AuxOutputs", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityMultiView:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "MultiView", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilitySuperSourceArt:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "SuperSourceArt", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilitySuperSourceBox:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "SuperSourceBox", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				case bmdSwitcherInputAvailabilityInputCut:
					inputArgs[Input::availability] = v8::String::NewFromUtf8(info.GetIsolate(), "InputCut", v8::NewStringType::kNormal).ToLocalChecked();
					break;
				default:
					inputArgs[Input::availability] = Nan::Undefined();
				}
			} else
				inputArgs[Input::availability] = Nan::Undefined();

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
			}
			else
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
			
			if(input->AreNamesDefault(&isDefault) == S_OK)
				inputArgs[Input::defaultNames] = Nan::True();
			else
				inputArgs[Input::defaultNames] = Nan::False();
			
			if(input->IsProgramTallied(&prgmTallied) == S_OK)
				inputArgs[Input::programTallied] = Nan::True();
			else
				inputArgs[Input::programTallied] = Nan::False();
			
			if(input->IsPreviewTallied(&prvwTallied) == S_OK)
				inputArgs[Input::previewTallied] = Nan::True();
			else
				inputArgs[Input::previewTallied] = Nan::False();

			/**
			 * TODO: APPLY DATA IN INPUT'S NEW METHOD
			 */

			//Create an instance of an Input
			inputObject = Nan::New(Input::constructor)->NewInstance(context, Input::inputArgc, inputArgs).ToLocalChecked();
			
			//Add to return array
			inputArray->Set(context, inputArray->Length(), inputObject);
		}


		//End code
		info.GetReturnValue().Set(inputArray);
		//inputObject = Nan::NewInstance(Nan::New(Input::constructor)).ToLocalChecked();
		//info.GetReturnValue().Set(inputObject);
	}

	NODE_MODULE(ATEMSwitcherNode, Init);
}