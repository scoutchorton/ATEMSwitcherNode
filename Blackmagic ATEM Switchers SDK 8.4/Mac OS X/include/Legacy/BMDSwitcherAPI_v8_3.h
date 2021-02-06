/* -LICENSE-START-
 ** Copyright (c) 2019 Blackmagic Design
 **
 ** Permission is hereby granted, free of charge, to any person or organization
 ** obtaining a copy of the software and accompanying documentation covered by
 ** this license (the "Software") to use, reproduce, display, distribute,
 ** execute, and transmit the Software, and to prepare derivative works of the
 ** Software, and to permit third-parties to whom the Software is furnished to
 ** do so, all subject to the following:
 **
 ** The copyright notices in the Software and this entire statement, including
 ** the above license grant, this restriction and the following disclaimer,
 ** must be included in all copies of the Software, in whole or in part, and
 ** all derivative works of the Software, unless such copies or derivative
 ** works are solely in the form of machine-executable object code generated by
 ** a source language processor.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 ** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 ** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 ** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 ** DEALINGS IN THE SOFTWARE.
 ** -LICENSE-END-
 */

#ifndef BMD_BMDSWITCHERAPI_V8_3_H
#define BMD_BMDSWITCHERAPI_V8_3_H


#ifndef BMD_CONST
	#if defined(_MSC_VER)
		#define BMD_CONST __declspec(selectany) static const
	#else
		#define BMD_CONST static const
	#endif
#endif

#include "BMDSwitcherAPI.h"

BMD_CONST REFIID IID_IBMDSwitcherRecordAV_v8_3                         = /* EDABB9AE-E070-4CD8-A73C-424AFC886483 */ { 0xED,0xAB,0xB9,0xAE,0xE0,0x70,0x4C,0xD8,0xA7,0x3C,0x42,0x4A,0xFC,0x88,0x64,0x83 };

BMD_CONST REFIID IID_IBMDSwitcherRecordDiskIterator_v8_3               = /* 17AAAD91-A792-4786-BC46-AF02F87194DA */ { 0x17,0xAA,0xAD,0x91,0xA7,0x92,0x47,0x86,0xBC,0x46,0xAF,0x02,0xF8,0x71,0x94,0xDA };

BMD_CONST REFIID IID_IBMDSwitcherRecordDisk_v8_3                       = /* CE5A81E6-9418-4DB5-99BF-2832ECE9ECBD */ { 0xCE,0x5A,0x81,0xE6,0x94,0x18,0x4D,0xB5,0x99,0xBF,0x28,0x32,0xEC,0xE9,0xEC,0xBD };

BMD_CONST REFIID IID_IBMDSwitcherStreamRTMP_v8_3                       = /* AF7170E6-CBEB-4682-8511-8D2C6B2A2241 */ { 0xAF,0x71,0x70,0xE6,0xCB,0xEB,0x46,0x82,0x85,0x11,0x8D,0x2C,0x6B,0x2A,0x22,0x41 };

#if defined(__cplusplus)

// Forward Declarations

class IBMDSwitcherRecordAV_v8_3;
class IBMDSwitcherRecordDiskIterator_v8_3;
class IBMDSwitcherRecordDisk_v8_3;
class IBMDSwitcherStreamRTMP_v8_3;

/* Interface IBMDSwitcherRecordAV_v8_3  - Switcher RecordAV Object */

class BMD_PUBLIC IBMDSwitcherRecordAV_v8_3 : public IUnknown
{
public:
    virtual HRESULT StartRecording (void) = 0;
    virtual HRESULT StopRecording (void) = 0;
    virtual HRESULT SwitchDisk (void) = 0;
    virtual HRESULT IsRecording (/* out */ bool* recording) = 0;
    virtual HRESULT GetStatus (/* out */ BMDSwitcherRecordAVState* state, /* out */ BMDSwitcherRecordAVError* error) = 0;
    virtual HRESULT SetFilename (/* in */ CFStringRef filename) = 0;
    virtual HRESULT GetFilename (/* out */ CFStringRef* filename) = 0;
    virtual HRESULT SetRecordInAllCameras (/* in */ bool recordInAllCameras) = 0;
    virtual HRESULT GetRecordInAllCameras (/* out */ bool* recordInAllCameras) = 0;
    virtual HRESULT GetWorkingSetLimit (/* out */ uint32_t* workingSetLimit) = 0;
    virtual HRESULT SetWorkingSetDisk (/* in */ uint32_t workingSetIndex, /* in */ BMDSwitcherRecordDiskId diskId) = 0;
    virtual HRESULT GetWorkingSetDisk (/* in */ uint32_t workingSetIndex, /* in */ BMDSwitcherRecordDiskId* diskId) = 0;
    virtual HRESULT GetActiveDiskIndex (/* out */ uint32_t* workingSetIndex) = 0;
    virtual HRESULT RequestDuration (void) = 0;
    virtual HRESULT GetDuration (/* out */ uint8_t* hours, /* out */ uint8_t* minutes, /* out */ uint8_t* seconds, /* out */ uint8_t* frames, /* out */ bool* dropFrame) = 0;
    virtual HRESULT GetTotalRecordingTimeAvailable (/* out */ uint32_t* totalRecordingTimeAvailable) = 0;
    virtual HRESULT CreateIterator (/* in */ REFIID iid, /* out */ LPVOID* ppv) = 0;
    virtual HRESULT AddCallback (/* in */ IBMDSwitcherRecordAVCallback* callback) = 0;
    virtual HRESULT RemoveCallback (/* in */ IBMDSwitcherRecordAVCallback* callback) = 0;

protected:
    virtual ~IBMDSwitcherRecordAV_v8_3 () {} // call Release method to drop reference count
};

/* Interface IBMDSwitcherRecordDiskIterator_v8_3 - RecordDisk Iterator */

class BMD_PUBLIC IBMDSwitcherRecordDiskIterator_v8_3 : public IUnknown
{
public:
    virtual HRESULT Next (/* out */ IBMDSwitcherRecordDisk_v8_3** recordDisk) = 0;
    virtual HRESULT GetById (/* in */ BMDSwitcherRecordDiskId diskId, /* out */ IBMDSwitcherRecordDisk_v8_3** disk) = 0;

protected:
    virtual ~IBMDSwitcherRecordDiskIterator_v8_3 () {} // call Release method to drop reference count
};

/* Interface IBMDSwitcherRecordDisk_v8_3 - Switcher RecordDisk Object */

class BMD_PUBLIC IBMDSwitcherRecordDisk_v8_3 : public IUnknown
{
public:
    virtual HRESULT GetId (/* out */ BMDSwitcherRecordDiskId* diskId) = 0;
    virtual HRESULT GetVolumeName (/* out */ CFStringRef* volumeName) = 0;
    virtual HRESULT GetRecordingTimeAvailable (/* out */ uint32_t* recordingTimeAvailable) = 0;
    virtual HRESULT GetStatus (/* out */ BMDSwitcherRecordDiskStatus* diskStatus) = 0;
    virtual HRESULT RequestRecordingTimeAvailable (void) = 0;
    virtual HRESULT AddCallback (/* in */ IBMDSwitcherRecordDiskCallback* callback) = 0;
    virtual HRESULT RemoveCallback (/* in */ IBMDSwitcherRecordDiskCallback* callback) = 0;

protected:
    virtual ~IBMDSwitcherRecordDisk_v8_3 () {} // call Release method to drop reference count
};

/* Interface IBMDSwitcherStreamRTMP_v8_3 - Switcher Stream RTMP Object */

class BMD_PUBLIC IBMDSwitcherStreamRTMP_v8_3 : public IUnknown
{
public:
    virtual HRESULT StartStreaming (void) = 0;
    virtual HRESULT StopStreaming (void) = 0;
    virtual HRESULT IsStreaming (/* out */ bool* streaming) = 0;
    virtual HRESULT GetStatus (/* out */ BMDSwitcherStreamRTMPState* state, /* out */ BMDSwitcherStreamRTMPError* error) = 0;
    virtual HRESULT SetServiceName (/* in */ CFStringRef serviceName) = 0;
    virtual HRESULT GetServiceName (/* out */ CFStringRef* serviceName) = 0;
    virtual HRESULT SetUrl (/* in */ CFStringRef url) = 0;
    virtual HRESULT GetUrl (/* out */ CFStringRef* url) = 0;
    virtual HRESULT SetKey (/* in */ CFStringRef url) = 0;
    virtual HRESULT GetKey (/* out */ CFStringRef* key) = 0;
    virtual HRESULT SetBitrates (/* in */ uint32_t lowBitrate, /* in */ uint32_t highBitrate) = 0;
    virtual HRESULT GetBitrates (/* out */ uint32_t* lowBitRate, /* out */ uint32_t* highBitRate) = 0;
    virtual HRESULT RequestDuration (void) = 0;
    virtual HRESULT GetDuration (/* out */ uint8_t* hours, /* out */ uint8_t* minutes, /* out */ uint8_t* seconds, /* out */ uint8_t* frames, /* out */ bool* dropFrame) = 0;
    virtual HRESULT GetEncodingBitrate (/* out */ uint32_t* encodingBitrate) = 0;
    virtual HRESULT GetCacheUsed (/* out */ double* cacheUsed) = 0;
    virtual HRESULT AddCallback (/* in */ IBMDSwitcherStreamRTMPCallback* callback) = 0;
    virtual HRESULT RemoveCallback (/* in */ IBMDSwitcherStreamRTMPCallback* callback) = 0;

protected:
    virtual ~IBMDSwitcherStreamRTMP_v8_3 () {} // call Release method to drop reference count
};


#endif      // defined(__cplusplus)
#endif /* defined(BMD_BMDSWITCHERAPI_V8_3_H) */
