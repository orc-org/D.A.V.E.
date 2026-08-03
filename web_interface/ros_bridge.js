var connected = false;
let stopAll = false;

var ros = new ROSLIB.Ros({
    'url': 'ws://localhost:9090'
});

// ROS Topics
let handActionClient = null;
let handTargetPub = null;
let armActionClient = null;
let armTargetPub = null;
let modePub = null;
let sensitivityPub = null;
let lightsPub = null;


// ROS Subscribers
let armGlobalPosSub = null;
let wheelFLSub = null;
let wheelFRSub = null;
let wheelRLSub = null;
let wheelRRSub = null;
let currentArmX = 0.0;
let currentArmY = 0.0;



document.addEventListener("DOMContentLoaded", () => {
    // Trigger emergency stop on button press
    function emergencyStop() {
        stopAll = true;
        console.log("Emergency Stop")
    }
    document.getElementById("emergency-stop").onclick = emergencyStop

    // Connect to ROS

    function connectROS() {
        const url = document.getElementById('connection-port').value
        const dataInput = document.getElementById("connection-status-text")
        const indicator = document.getElementById("connection-status-indicator")

        if (checkStop()) return;

        ros = new ROSLIB.Ros({
            url: url
        });

        ros.on('connection', function () {
            dataInput.textContent = "Connected"
            indicator.style.backgroundColor = 'var(--accent-active)';
            setupROS();
        })

        ros.on('error', function () {
            dataInput.textContent = "Error"
            indicator.style.backgroundColor = 'var(--accent-mid)';
        })

        ros.on('close', function () {
            dataInput.textContent = "Disconnected"
            indicator.style.backgroundColor = 'var(--accent-inactive)';
        })
    }

    document.getElementById("connect-button").onclick = connectROS;

    const handSlider = document.getElementById("hand-pos-slider");
    if (handSlider) {
        handSlider.oninput = function () {
            publishHandPosition(this.value / 100);
        };
    }

    const armSlider = document.getElementById("arm-pos-slider");
    if (armSlider) {
        armSlider.oninput = function () {
            publishArmPosition(this.value);
        };
    }
});


function checkStop() {
    if (stopAll) {
        console.log("Emergency Stop");
        return true;
    }
    return false;
}


function setupROS() {
    // Arm Position Monitoring
    armGlobalPosSub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_global_pos',
        messageType: 'geometry_msgs/msg/Point'
    });
    armGlobalPosSub.subscribe((msg) => {
        currentArmX = Number(msg.x);
        currentArmY = Number(msg.y);
        const currentHeight = Number(msg.z);
        const height = currentHeight.toFixed(2);
        const div = document.getElementById('arm-height');
        const dataInput = div.querySelector('.data-input');
        dataInput.textContent = (height / 8.88).toFixed(2) + "m";

        const slider = document.getElementById('arm-pos-slider');
        const valueDisplay = document.getElementById('arm-pos-value');
        const maxHeight = 13.5;
        const percentage = Math.max(0, Math.min(100, (currentHeight / maxHeight) * 100));

        slider.value = percentage;
        valueDisplay.textContent = `${Math.round(percentage)}%`;
    })

    // Joy Subscriber (So we can use controller)
    const joySubscriber = new ROSLIB.Topic({
        ros: ros,
        name: "/joy",
        messageType: "sensor_msgs/msg/Joy"
    });

    joySubscriber.subscribe(function(msg) {
        controller.connected();
        controller.update(msg);
    });

    //Swap Drive and Arm Mode
    
    // Drive/Arm mode publisher
    modePub = new ROSLIB.Topic({
        ros: ros,
        name: "/control_mode",
        messageType: "std_msgs/msg/String"
    });
    
    //Subscribes to my wheel power topics
    wheelFLSub = new ROSLIB.Topic({
        ros: ros,
        name: "/motor/front_left",
        messageType: "std_msgs/msg/Float32"
    });

    wheelFRSub = new ROSLIB.Topic({
        ros: ros,
        name: "/motor/front_right",
        messageType: "std_msgs/msg/Float32"
    });

    wheelRLSub = new ROSLIB.Topic({
        ros: ros,
        name: "/motor/rear_left",
        messageType: "std_msgs/msg/Float32"
    });

    wheelRRSub = new ROSLIB.Topic({
        ros: ros,
        name: "/motor/rear_right",
        messageType: "std_msgs/msg/Float32"
    });


    wheelFLSub.subscribe((msg)=>{
        //console.log("Front left:", msg.data);
        updateWheelDisplay("wheel-fl-power", msg.data);
    });

    wheelFRSub.subscribe((msg)=>{
        updateWheelDisplay("wheel-fr-power", msg.data);
    });

    wheelRLSub.subscribe((msg)=>{
        updateWheelDisplay("wheel-bl-power", msg.data);
    });

    wheelRRSub.subscribe((msg)=>{
        updateWheelDisplay("wheel-br-power", msg.data);
    });

    //Sensitivity node publisher
    sensitivityPub = new ROSLIB.Topic({
        ros: ros,
        name: "/sensitivity",
        messageType: "std_msgs/msg/String"
    });

    //Lights node publisher
    lightsPub = new ROSLIB.Topic({
        ros: ros,
        name: "/lights",
        messageType: "std_msgs/msg/Bool"
    });

    // Arm Position Control
    armTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_target',
        messageType: 'geometry_msgs/msg/Point'
    });

    armActionClient = new ROSLIB.ActionClient({
        ros: ros,
        serverName: '/move_to_point',
        actionName: 'arm_interfaces/action/MoveToPoint'
    });

    // Hand position Monitoring
    handStateSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_state',
        messageType: 'std_msgs/msg/Float32'
    });
    

    handStateSub.subscribe((msg) => {
        message = msg.data;
        const div = document.getElementById('hand-pos');
        const dataInput = div.querySelector('.data-input');
        dataInput.textContent = ((1 - message) * 0.2).toFixed(2) + "m";

        const slider = document.getElementById('hand-pos-slider');
        slider.value = 100 * message;
        document.getElementById('hand-pos-value').textContent = slider.value + "%";
    });


    // Hand Position Control
    handTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_target',
        messageType: 'std_msgs/msg/Float32'
    });


    handActionClient = new ROSLIB.ActionClient({
        ros: ros,
        serverName: '/gripper_command',
        actionName: 'arm_interfaces/action/GripperCommand'
    });

}

function publishArmPosition(value) {
    if (checkStop()) return;

    const percentage = Number(value);
    const maxHeight = 13.5;
    const clampedPerc = Math.max(0, Math.min(100, percentage));
    const z = (clampedPerc / 100) * maxHeight;

    const msg = new ROSLIB.Message({
        x: currentArmX,
        y: currentArmY,
        z: z
    });

    armTargetPub.publish(msg);
}

function publishHandPosition(value) {
    if (checkStop()) return;

    if (handTargetPub) {
        handTargetPub.publish(new ROSLIB.Message({
            data: value
        }));
    }
    
    
    if (!handActionClient) {
        return;
    }

    const handGoal = new ROSLIB.Goal({
        actionClient: handActionClient,
        goalMessage: {
            position: value
        }
    });

    handGoal.send();
    

}

window.publishHandPosition = publishHandPosition;


//Used to change drive and arm mode
function publishMode(mode) {

    if (!modePub) {
        console.warn("Mode selector not initialized");
        return;
    }

    const msg = new ROSLIB.Message({
        data: mode
    });

    console.log("Selected mode:", mode);

    modePub.publish(msg);
}
window.publishMode = publishMode;

//Used for the wheel power
function updateWheelDisplay(id, power)
{
    const percentage = Math.round(power * 100);

    document.getElementById(id).textContent = percentage + "%";
}

//Adjusts the sensitivity
function publishSensitivity(sensitivity) {

    if (!sensitivityPub) {
        console.warn("Sensitivity publisher not initialized");
        return;
    }

    sensitivityPub.publish(new ROSLIB.Message({
        data: sensitivity
    }));

    //console.log("Selected sensitivity:", sensitivity);
}

window.publishSensitivity = publishSensitivity;

//Turns on/off lights
function publishLights(state) {

    if (!lightsPub) {
        console.warn("Lights publisher not initialized");
        return;
    }

    lightsPub.publish(new ROSLIB.Message({
        data: state
    }));

    //console.log("Lights:", state ? "ON" : "OFF");
}

window.publishLights = publishLights;
