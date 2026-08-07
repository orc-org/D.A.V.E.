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
let speedSub = null
let latSub = null
let lngSub = null
let bearingSub = null
let distanceSub = null
let nextLatSub = null
let nextLngSub = null
let currentRouteSub = null
let followingSub = null
let morseStrSub = null
let morsePixelSub = null

//GPS Stuff
let gpsCurrentFix = {
    latitude: null,
    longitude: null,
    altitude: null,
    status: -1, // STATUS_NO_FIX
    satellites: 0,
    hdop: 0,
    lastUpdate: null
};

let gpsHomeOrigin = {
    latitude: null,
    longitude: null,
    altitude: null,
    isSet: false
};

let gpsTargetWaypoint = {
    latitude: null,
    longitude: null,
    x: null,
    y: null,
    isSet: false
};



// Math utility helpers for GPS calculation
function calculateDistanceMeters(lat1, lon1, lat2, lon2) {
    if (lat1 === null || lon1 === null || lat2 === null || lon2 === null) return 0.0;
    const R = 6371000;
    const dLat = (lat2 - lat1) * Math.PI / 180;
    const dLon = (lon2 - lon1) * Math.PI / 180;
    const a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
        Math.sin(dLon / 2) * Math.sin(dLon / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    return R * c;
}

function calculateBearingDegrees(lat1, lon1, lat2, lon2) {
    if (lat1 === null || lon1 === null || lat2 === null || lon2 === null) return 0.0;
    const phi1 = lat1 * Math.PI / 180;
    const phi2 = lat2 * Math.PI / 180;
    const lam1 = lon1 * Math.PI / 180;
    const lam2 = lon2 * Math.PI / 180;
    const y = Math.sin(lam2 - lam1) * Math.cos(phi2);
    const x = Math.cos(phi1) * Math.sin(phi2) - Math.sin(phi1) * Math.cos(phi2) * Math.cos(lam2 - lam1);
    let bearing = Math.atan2(y, x) * 180 / Math.PI;
    return (bearing + 360) % 360;
}

function getCompassDirection(bearingDeg) {
    const dirs = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'];
    const idx = Math.round(bearingDeg / 45) % 8;
    return dirs[idx];
}

function offsetLatLonByMeters(lat, lon, dxMeters, dyMeters) {
    if (lat === null || lon === null) return {
        latitude: 0.0,
        longitude: 0.0
    };
    const R = 6371000;
    const newLat = lat + (dyMeters / R) * (180 / Math.PI);
    const newLon = lon + (dxMeters / (R * Math.cos(lat * Math.PI / 180))) * (180 / Math.PI);
    return {
        latitude: newLat,
        longitude: newLon
    };
}



document.addEventListener("DOMContentLoaded", () => {
    // Trigger emergency stop on button press
    function StopAll() {
        stopAll = true;
        console.log("Stop All")
    }
    document.getElementById("stop-all").onclick = StopAll

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
        console.log("Stop All");
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

    joySubscriber.subscribe(function (msg) {
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


    wheelFLSub.subscribe((msg) => {
        //console.log("Front left:", msg.data);
        updateWheelDisplay("wheel-fl-power", msg.data);
    });

    wheelFRSub.subscribe((msg) => {
        updateWheelDisplay("wheel-fr-power", msg.data);
    });

    wheelRLSub.subscribe((msg) => {
        updateWheelDisplay("wheel-bl-power", msg.data);
    });

    wheelRRSub.subscribe((msg) => {
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

    // GNSS Subscribers 
    const gpsFixSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gps/fix',
        messageType: 'sensor_msgs/msg/NavSatFix'
    });

    const gpsNmeaSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gps/nmea_raw',
        messageType: 'std_msgs/msg/String'
    });

    gpsFixSub.subscribe((msg) => {
        const lat = GpsCurrentFix.latitude;
        const lng = GpsCurrentFix.longitude;
        const wplat = gpsTargetWaypoint.latitude;
        const wplng = gpsTargetWaypoint.longitude;

        const latDiv = document.getElementById('latitude-value');
        const lngDiv = document.getElementById('longitude-value');
        const distDiv = document.getElementById('distance-value');
        const wplatDiv = document.getElementById('next-waypoint-lat');
        const wplngDiv = document.getElementById('next-waypoint-lng');

        latDiv.textContent = isNaN(lat) ? 'N/A' : lat.toFixed(5);
        lngDiv.textContent = isNaN(lng) ? 'N/A' : lng.toFixed(5);
        wplatDiv.textContent = isNaN(wplat) ? 'N/A' : wplat.toFixed(5);
        wplngDiv.textContent = isNaN(wplng) ? 'N/A' : wplng.toFixed(5);

        const dist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const bearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const cardDir = getCompassDirection(bearing);

        const north = document.getElementById('north-arrow');
        const south = document.getElementById('south-arrow');
        const arrow = document.getElementById('compass-arrow');

        north.style.transform = 'translate(-50%, -100%) rotate(0deg)';
        south.style.transform = 'translate(-50%, -100%) rotate(180deg)';
        arrow.style.transform = `translate(-50%, -100%) rotate(${bearing}deg)`;

        distDiv.textContent = isNaN(dist) ? 'N/A' : dist.toFixed(2) + 'm';

    });

    // Morse Node
    function setupMorse() {
        morseStrSub = new ROSLIB.Topic({
            ros: ros,
            name: '/morse_code_str',
            messageType: 'std_msgs/msg/String'
        });

        morseStrSub.subscribe((message) => {
            const rawMsg = (message && message.data !== undefined) ? message.data : '-. -..-. .-'
            updateMorseTeletype(message);
        });
    }

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
function updateWheelDisplay(id, power) {
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
<<<<<<< HEAD
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
    function StopAll() {
        stopAll = true;
        console.log("Stop All")
    }
    document.getElementById("stop-all").onclick = StopAll

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
        console.log("Stop All");
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
=======

// Morse node

function setMorseParamaters(name, val) {
    if (checkStop()) return;

    const paramService = new ROSLIB.Service({
        ros: ros,
        name: '/morse_recorder/set_parameters',
        serviceType: 'rcl_interfaces/srv/SetParameters'
    });

    const req = new ROSLIB.ServiceRequest({
        parameters: [{
            name: paramName,
            value: {
                type: 2, // INTEGER
                integer_value: parseInt(val)
            }
        }]
    });
}

function getPassword() {
    if (checkStop()) return;

    const getpw = new ROSLIB.Service({
        ros: ros,
        name: '/get_password',
        serviceType: 'enigma_machine_interfaces/srv/GetPassword'
    });

    const req = new ROSLIB.ServiceRequest({});

    getpw.callService(req, (result) => {
        const passwordDiv = document.getElementById('morse-string');
        passwordDiv.textContent = result.password;
    });
}

function transmitMorse() {
    if (checkStop()) return;

    const passwordDiv = document.getElementById('morse-string');
    const msg = passwordDiv.textContent.trim()

    const service = new ROSLIB.Service({
        ros: ros,
        name: 'set/message',
        serviceType: 'enigma_machine_interfaces/srv/SetMessage'
    });
    const request = new ROSLIB.ServiceRequest({
        message: msg
    });

    service.callService(request);

}

document.addEventListener('DOMContentLoaded', () => {
    const getter = document.getElementById('push-get')
    const tapper = document.getElementById("arm-push")
    getter.addEventListener('click', getPassword)
    tapper.addEventListener('click', transmitMorse)

    const sliderBright = document.getElementById('morse-brightness-slider')
    const sliderThresh = document.getElementById('morse-threshold-slider')

    const textBright = document.getElementById('morse-brightness')
    const textThresh = document.getElementById('morse-threshold')

    sliderBright.addEventListener('input', (e) => {
        setMorseParamaters('brightness_threshold', e.target.value)
        textBright.innerText = e.target.value
    })
    sliderBright.addEventListener('change', (e) => {
        setMorseParamaters('brightness_threshold', e.target.value)
    })

    sliderThresh.addEventListener('input', (e) => {
        setMorseParamaters('pixel_count_threshold', e.target.value)
        textThresh.innerText = e.target.value
    })
    sliderThresh.addEventListener('change', (e) => {
        setMorseParamaters('pixel_count_threshold', e.target.value)
    })
})
