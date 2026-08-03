const controller = (() => {

    let mode = "drive"; //I dont think this will be needed after I add the button to swap, but chat put it here so Ill keep it for now.]
    let sensitivity = "fine";
    let last_message_time = Date.now(); //Needed so ui updates if controller gets unplugged (but the lights on the controller would also shut off)
    let previous_buttons = [];
    let lights = false;
    let handPosition = 0.5;
    let lastHandUpdate = 0;

    function update(joyMsg) {
        if (checkStop()) return
        last_message_time = Date.now();
        
        const axes = joyMsg.axes;
        const buttons = joyMsg.buttons;
        
        // ===========================
        // Controller mappings
        // ===========================

        
        const leftX = axes[0];
        const leftY = axes[1];
        const rightX = axes[2];
        const rightY = axes[3];

        const cross = buttons[0];
        const circle = buttons[1];
        const square = buttons[2]; //I think I have square and triangle swapped on the documentation. Oops
        const triangle = buttons[3];
        const select = buttons[4];
        const ps_button = buttons[5]; //Dont hold this button
        const start = buttons[6];
        const L3 = buttons[7];
        const R3 = buttons[8];
        const LB = buttons[9];
        const RB = buttons[10];
        const d_up = buttons[11];
        const d_down = buttons[12];
        const d_left = buttons[13];
        const d_right = buttons[14];
        const touchpad = buttons[15]; //Dont touch this button
        const mic = buttons[16];

        //Uncomment if need to see if html reads controller
        //console.log(joyMsg);
        //console.log("BUTTONS:", buttons);

        // Calls the setMode function by pressign triangle
        if (triangle === 1 && previous_buttons[3] !== 1) {

            if (mode === "drive") {
                setMode("arm");
            } else {
                setMode("drive");
            }

        }

        // Toggle sensitivity with square button
        if (square === 1 && previous_buttons[2] !== 1) {

            if (sensitivity === "fine") {
                setSensitivity("coarse");
            } 
            else if (sensitivity === "coarse") {
                setSensitivity("ultra");
            }
            else {
                setSensitivity("fine");
            }

        }

        // Toggle lights with Select button
        if (select === 1 && previous_buttons[4] !== 1) {

            if (lights) {
                setLights(false);
            } else {
                setLights(true);
            }

        }

        // holding bumpers should slowly adjust hand position slider (visually)
        const now = Date.now();

        if (now - lastHandUpdate > 50) {   // update every 50ms

            if (RB === 1) {
                handPosition += 0.01;
                handPosition = Math.min(handPosition, 1.0);
                setHandPosition(handPosition);
                lastHandUpdate = now;
            }


            if (LB === 1) {
                handPosition -= 0.01;
                handPosition = Math.max(handPosition, 0.0);
                setHandPosition(handPosition);
                lastHandUpdate = now;
            }
        }

        //Only sends the command after yuo release the bumper
        if (previous_buttons[10] === 1 && RB === 0) {
            sendHandPosition();
        }

        if (previous_buttons[9] === 1 && LB === 0) {
            sendHandPosition();
        }
        
        // D-pad controls UI focus
        if (d_right === 1 && previous_buttons[14] !== 1) {
            pressTab();
        }
        
        if (d_down === 1 && previous_buttons[12] !== 1) {
            pressTab();
        }

        if (d_left === 1 && previous_buttons[13] !== 1) {
            pressShiftTab();
        }

        if (d_up === 1 && previous_buttons[11] !== 1) {
            pressShiftTab();
        }

        previous_buttons = [...buttons];

    }

    //Need to look over this section still, but should toggle the mode with button press
    function setMode(newMode) {
        mode = newMode;
        const driveButton = document.getElementById("set-mode-drive");
        const armButton = document.getElementById("set-mode-arm");

        //should call the window function in script.js
        if (window.setModeButton) {

            if (mode === "drive") {
                window.setModeButton(driveButton);
            } 
            else {
                window.setModeButton(armButton);
            }
        } else {
            console.log("setModeButton does not exist"); //was used for debuging
        }
    }

    function pressTab() {
        document.dispatchEvent(new KeyboardEvent("keydown", {
            key: "Tab",
            code: "Tab",
            keyCode: 9,
            which: 9,
            bubbles: true
        }));

        document.dispatchEvent(new KeyboardEvent("keyup", {
            key: "Tab",
            code: "Tab",
            keyCode: 9,
            which: 9,
            bubbles: true
        }));
    }

    function pressShiftTab() {
        document.dispatchEvent(new KeyboardEvent("keydown", {
            key: "Shift",
            code: "ShiftLeft",
            keyCode: 16,
            which: 16,
            bubbles: true
        }));

        document.dispatchEvent(new KeyboardEvent("keydown", {
            key: "Tab",
            code: "Tab",
            keyCode: 9,
            which: 9,
            bubbles: true
        }));

        document.dispatchEvent(new KeyboardEvent("keyup", {
            key: "Tab",
            code: "Tab",
            keyCode: 9,
            which: 9,
            bubbles: true
        }));

        document.dispatchEvent(new KeyboardEvent("keyup", {
            key: "Shift",
            code: "ShiftLeft",
            keyCode: 16,
            which: 16,
            bubbles: true
        }));
    }

    function setSensitivity(newSensitivity) {

        sensitivity = newSensitivity;

        const sensitivityButton = document.getElementById(
            "sensitivity-" + sensitivity
        );

        if (window.setSensitivityButton) {
            window.setSensitivityButton(sensitivityButton);
        }

    }

    function setLights(state) {

        lights = state;

        const onButton = document.getElementById("light-on");
        const offButton = document.getElementById("light-off");

        if (window.setLightsButton) {

            if (lights) {
                window.setLightsButton(onButton);
            } else {
                window.setLightsButton(offButton);
            }

        }

    }
    
    //This is so the slider changes visually while bumper is held
    function setHandPosition(value) {

        handPosition = value;

        const slider = document.getElementById("hand-pos-slider");

        if (slider) {
            slider.value = value * 100;
        }

        const valueDisplay = document.getElementById("hand-pos-value");

        if (valueDisplay) {
            valueDisplay.textContent = Math.round(value * 100) + "%";
        }
    }

    //This actually changes it
    function sendHandPosition() {

        if (window.publishHandPosition) {
            window.publishHandPosition(handPosition);
        }

    }

    //This part makes it so the ui knows the controller is disconnected (Joy node just stops publishing if controller is unplugged, so it says "connected" still without this)
    setInterval(() => {
        if (Date.now() - last_message_time > 1000) {
            disconnected();
        }
    }, 500);

    //Changes ui to show if controller is connected/disconnected
    function connected() {
        document.getElementById("controller-status-text").textContent =
            "Controller Connected";
        document.getElementById("controller-status-indicator")
            .style.backgroundColor = "var(--accent-active)";
    }

    function disconnected() {
        document.getElementById("controller-status-text").textContent =
            "No Controller Found";
        document.getElementById("controller-status-indicator")
            .style.backgroundColor = "var(--accent-inactive)";
    }

    return {
        update,
        connected,
        disconnected,
        setMode,
        setLights
    };

})();
