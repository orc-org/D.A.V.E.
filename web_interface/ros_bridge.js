var connected = false;
var stopAll = false;

document.addEventListener("DOMContentLoaded", () => {
    // Trigger emergency stop on button press
    function emergencyStop() {
        stopAll = true;
        console.log("Emergency Stop")
    }
    document.getElementById("emergency-stop").onclick = emergencyStop

    // Check for emergency stop
    if (!stopAll) {
        // Connect to ROS
        function connectROS() {
            const url = document.getElementById('connection-port').value
            const dataInput = document.getElementById("connection-status-text")
            const indicator = document.getElementById("connection-status-indicator")

            console.log("A")

            var ros = new ROSLIB.Ros({
                url: url
            });

            ros.on('connection', function () {
                dataInput.textContent = "Connected"
                indicator.style.backgroundColor = 'var(--accent-active)';
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
    }
});