{
  "targets": [
    {
      "target_name": "ATEMSwitcherNode",
      "sources": [
		"Blackmagic ATEM Switchers SDK 8.4\Windows\include\BMDSwitcherAPI.idl",
    "build/BMDSwitcherAPI_i.c",
    "build/BMDSwitcherAPI_h.h",
		"src/Switcher.cpp",
		"src/Switcher.h"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [
        "-lcomsuppw.lib"
      ]
    }
  ]
}