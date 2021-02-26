{
  "targets": [
    {
      "target_name": "ATEMSwitcherNode",
      "sources": [
		"C:/Program Files (x86)/Blackmagic Design/Blackmagic ATEM Switchers/Developer SDK/Windows/include/BMDSwitcherAPI.idl",
		"build/BMDSwitcherAPI_i.c",
		"build/BMDSwitcherAPI_h.h",
		"src/Switcher.cpp",
		"src/Switcher.h",
		"src/test.js"
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