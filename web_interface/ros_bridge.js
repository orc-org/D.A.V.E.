var connected = false;
var stopAll = false;

var ros = new ROSLIB.Ros({'url': 'ws://localhost:9090'});

// ROS Topics

// ROS Subscribers
let armGlobalPosSub = null;



document.addEventListener("DOMContentLoaded", () => {
    // Trigger emergency stop on button press
    function emergencyStop() {
        stopAll = true;
        console.log("Emergency Stop")
    }
    document.getElementById("emergency-stop").onclick = emergencyStop

    function checkStop() {
        if (stopAll) {
            console.log("Emergency Stop");
            return true;
        }
        return false;
    }


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

});


function setupROS() {
    // Arm Position Monitoring
    armGlobalPosSub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_global_pos',
        messageType: 'geometry_msgs/msg/Point'
    });
    armGlobalPosSub.subscribe((msg) => {
        message = msg.z;
        height = message.toFixed(2);
        const div = document.getElementById('arm-height');
        const dataInput = div.querySelector('.data-input');
        dataInput.textContent = height + "m";
    })

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
        dataInput.textContent = message.toFixed(2) + "m";
    });
}
