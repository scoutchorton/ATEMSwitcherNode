console.log("Initalizing...");
const ATEMSwitcher = require('../build/Release/ATEMSwitcherNode.node');
//const ATEMSwitcher = require('../build/Debug/ATEMSwitcherNode.node');

console.log("Connecting Switcher...");
let switcher = new ATEMSwitcher.Switcher();
switcher.address = "192.168.100.155";
switcher.connect();

console.log("Getting inputs...");
let inputs = switcher.getInputs();