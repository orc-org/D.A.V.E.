const controller = (() => {

    let mode = "drive"; //I dont think this will be needed after I add the button to swap, but chat put it here so Ill keep it for now.
    let last_message_time = Date.now(); //Needed so ui updates if controller gets unplugged (but the lights on the controller would also shut off)

    function update(joyMsg) {
        last_message_time = Date.now();
        //just commented this out for now, but it declares the controller buttons
        /*
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
        const triangle = buttons[2];
        const square = buttons[3];
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

        */
       
        //Just for now to make sure the ui is reading the controller
        console.log(joyMsg);

        // ===========================
        // Update wheel display
        // ===========================

        //In progress

        // ===========================
        // Toggle drive/arm mode
        // ===========================

        // In Progress, might make its own function

    }

    //Need to look over this section still, but should toggle the mode with button press
    function setMode(newMode) {

        mode = newMode;

        const text = document.querySelector("#arm-toggle .data-input");
        text.textContent = mode;

        const driveButton = document.getElementById("set-mode-drive");
        const armButton = document.getElementById("set-mode-arm");

        driveButton.classList.remove("active-toggle");
        armButton.classList.remove("active-toggle");

        if (mode === "drive") {
            driveButton.classList.add("active-toggle");
        } else {
            armButton.classList.add("active-toggle");
        }
    }


    //This part makes it so the ui knows the controller is disconnected (Joy node just stops publishing if controller is unplugged, so it says connected still without this)
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
        setMode
    };

})();