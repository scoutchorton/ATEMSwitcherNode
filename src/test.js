const ATEMSwitcher = require('../build/Release/ATEMSwitcherNode.node');
//const ATEMSwitcher = require('../build/Debug/ATEMSwitcherNode.node');

//console.dir(process);

let switcher = new ATEMSwitcher.Switcher();
let x;

switcher.address = "192.168.100.155";
switcher.connect();

x = 0;
//switcher.cut();
/*
let autoInterval = setInterval(() => {
    switcher.auto();
    if(x++ >= 4) {
        clearInterval(autoInterval);
        x = 0;
        let cutInterval = setInterval(() => {
            switcher.cut();
            if(x++ >= 4) {
                clearInterval(cutInterval);
                x = 0;
                let fadeToBlackInterval = setInterval(() => {
                    switcher.fadeToBlack();
                    if(x++ >= 4)
                        clearInterval(fadeToBlackInterval);
                }, 1500);
            }
        }, 1500);
    }
}, 1500);
*/

//console.log(JSON.stringify(switcher.getInputs()).replace('[', '[\n').replace(']', '\n]').split('},{').join('},\n  {'));
debugger;
let inputs = switcher.getInputs();
//console.log(inputs);
let data = { 'prgm': '', 'prvw': '' }
/*
for(let input of inputs) {
    if(input.programTallied)
        data.prgm = input.longName;
    else if(input.previewTallied)
        data.prvw = input.longName;
}

console.log(`Program: ${data.prgm}`);
console.log(`Preview: ${data.prvw}`);
*/
console.log(switcher.getInputs());
//switcher.getInputs();