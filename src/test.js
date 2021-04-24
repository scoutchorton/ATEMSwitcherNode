console.log("Initalizing...");
const ATEMSwitcher = require('../build/Release/ATEMSwitcherNode.node');
//const ATEMSwitcher = require('../build/Debug/ATEMSwitcherNode.node');

console.log("Connecting Switcher...");
let switcher = new ATEMSwitcher.Switcher();
switcher.address = "192.168.100.155";
switcher.connect();

console.log("Getting inputs...");
let inputs
console.log(JSON.stringify(inputs), "\n", JSON.stringify(switcher.inputs));
inputs = switcher.getInputs();
console.log(JSON.stringify(inputs), "\n", JSON.stringify(switcher.inputs));