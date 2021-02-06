/* -LICENSE-START-
** Copyright (c) 2018 Blackmagic Design
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
/* BMDSwitcherAPIDispatch_v7_5_2.cpp */

#include "BMDSwitcherAPI_v7_5_2.h"
#include <pthread.h>

#define kBMDSwitcherAPI_BundlePath "/Library/Application Support/Blackmagic Design/Switchers/BMDSwitcherAPI.bundle"

typedef IBMDSwitcherDiscovery_v7_5_2* (*CreateDiscoveryFunc)(void);

static pthread_once_t						gBMDSwitcherOnceControl		= PTHREAD_ONCE_INIT;
static CFBundleRef							gBundleRef					= NULL;
static CreateDiscoveryFunc					gCreateDiscoveryFunc		= NULL;

static void	InitBMDSwitcherAPI (void)
{
	CFURLRef		bundleURL;
	
	bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR(kBMDSwitcherAPI_BundlePath), kCFURLPOSIXPathStyle, true);
	if (bundleURL != NULL)
	{
		gBundleRef = CFBundleCreate(kCFAllocatorDefault, bundleURL);
		if (gBundleRef != NULL)
		{
			gCreateDiscoveryFunc = (CreateDiscoveryFunc)CFBundleGetFunctionPointerForName(gBundleRef, CFSTR("GetBMDSwitcherDiscoveryInstance_0003"));
		}
		CFRelease(bundleURL);
	}
}

IBMDSwitcherDiscovery_v7_5_2*			CreateBMDSwitcherDiscoveryInstance_v7_5_2 (void)
{
	pthread_once(&gBMDSwitcherOnceControl, InitBMDSwitcherAPI);
	
	if (gCreateDiscoveryFunc == NULL)
		return NULL;

	return gCreateDiscoveryFunc();
}
