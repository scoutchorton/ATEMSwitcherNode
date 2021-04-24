console.log("Initalizing...");
const ATEMSwitcher = require('../build/Release/ATEMSwitcherNode.node');
//const ATEMSwitcher = require('../build/Debug/ATEMSwitcherNode.node');

console.log("Connecting Switcher...");
let switcher = new ATEMSwitcher.Switcher();
switcher.address = "192.168.100.155";
switcher.connect();

console.log("Getting inputs...");
let inputs = switcher.getInputs();
let switchableInputs = inputs.filter((i) => {
    return i.availability.indexOf("MixEffectBlock0") != -1;
});
let normalInputs = inputs.filter((i) => {
    return i.type == "External";
});

console.log("Switching inputs...");
console.log(switcher.setPreview(switcher.inputs[0]));

//Disco!!
let inputIndex = 0;
setInterval(() => {
    switcher.setPreview(normalInputs[inputIndex]);
    switcher.setProgram(normalInputs[normalInputs.length - 1 - inputIndex]);
    inputIndex++;
    if(inputIndex >= normalInputs.length)
        inputIndex = 0;
}, 100);