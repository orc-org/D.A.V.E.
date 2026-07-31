var connected = false;
var stopAll = false;

var ros = new ROSLIB.Ros({
    'url': 'ws://localhost:9090'
});

// ROS Topics
let handActionClient = null;
let handTargetPub = null;
let armActionClient = null;
let armTargetPub = null;


// ROS Subscribers
let armGlobalPosSub = null;



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
        const currentHeight = Number(msg.z);
        const height = currentHeight.toFixed(2);
        const div = document.getElementById('arm-height');
        const dataInput = div?.querySelector('.data-input');
        if (dataInput) {
            dataInput.textContent = height + "m";
        }

        const slider = document.getElementById('arm-pos-slider');
        const valueDisplay = document.getElementById('arm-pos-value');
        const maxHeight = 4.0;
        const percentage = Math.max(0, Math.min(100, (currentHeight / maxHeight) * 100));

        if (slider) {
            slider.value = percentage;
        }
        if (valueDisplay) {
            valueDisplay.textContent = `${Math.round(percentage)}%`;
        }
    })

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
        dataInput.textContent = ((1 - message) * 0.2).toFixed(2)  + "m";

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

function publishArmPosition() {
    if (checkStop()) return;
    const msg = new ROSLIB.Message({
        x: targetX,
        y: targetY,
        z: targetZ
    });

    armTargetPub.publish(msg);
}

function publishHandPosition(value) {
    console.log("Publishing hand position: " + value);

    if (checkStop()) return;

    const clampedValue = Math.max(0, Math.min(1, Number(value)));

    if (handTargetPub) {
        handTargetPub.publish(new ROSLIB.Message({
            data: clampedValue
        }));
    }

    if (!handActionClient) {
        return;
    }

    const handGoal = new ROSLIB.Goal({
        actionClient: handActionClient,
        goalMessage: {
            position: clampedValue
        }
    });
    
    handGoal.send();
}

