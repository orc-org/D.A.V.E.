var connected = false;
var stopAll = false;
let ros; //Added this so "ros" can be used outside the connectROS function it was originally in

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


        console.log("A")

        ros = new ROSLIB.Ros({
            url: url
        });

        ros.on('connection', function () {
            dataInput.textContent = "Connected"
            indicator.style.backgroundColor = 'var(--accent-active)';
            
            //This subscribes to Joy Node so we can use the controller with the ui.
            const joySubscriber = new ROSLIB.Topic({
                ros: ros,
                name: "/joy",
                messageType: "sensor_msgs/msg/Joy"
            });

            joySubscriber.subscribe(function(msg) {
                controller.connected();
                controller.update(msg);
                
            });
            
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
