// ROS 2 Bridge Connection State
let ros = null;
let connected = false;

// ROS 2 Topics
let cmdVelPub = null;
let armTargetPub = null;
let autoModePub = null;
let armManualVelPub = null;
let gripperManualVelPub = null;
let gripperTargetPub = null;
let gripperActionClient = null;
let lightsPub = null;
let servoCommandsPub = null;

// Subscribers
let jointStateSub = null;
let armGlobalPosSub = null;
let gripperStateSub = null;
let tfSub = null;
let processStatusSub = null;
let flMotorSub = null;
let frMotorSub = null;
let rlMotorSub = null;
let rrMotorSub = null;
let cmdVelSub = null;
let gpsFixSub = null;
let gpsNmeaSub = null;
let gpsTargetPub = null;

// IMU Subscribers & State
let imuEulerSub = null;
let imuFilteredEulerSub = null;
let imuRawSub = null;
let imuFilteredSub = null;
let imuOdomSub = null;
let imuStrSub = null;

let imuEulerData = { roll: 0.0, pitch: 0.0, yaw: 0.0, lastUpdate: null };
let imuFilteredEulerData = { roll: 0.0, pitch: 0.0, yaw: 0.0, lastUpdate: null };
let imuMotionData = {
    accel: { x: 0.0, y: 0.0, z: 9.81 },
    gyro: { x: 0.0, y: 0.0, z: 0.0 }
};
let imuOdomData = {
    pos: { x: 0.0, y: 0.0, z: 0.0 },
    vel: { x: 0.0, y: 0.0, z: 0.0 }
};
let imuPublishingEnabled = true;

// Rover Path Trajectory History
let roverPathHistory = []; // Array of { lat, lon, x, y, timestamp }
let leafletPathPolyline = null;

// 3d viewer state (Disabled - 3D Robot View removed)
let showing3D = false;
let viewer3D = null;
let tfClient = null;
let urdfClient = null;
let targetMarker = null;
let robotStatePublisherRunning = false;

// Control state variables
let autoMode = false; // Default: MANUAL
let targetX = 25.0;
let targetY = 0.0;
let targetZ = 2.0;
let currentTooltipX = 25.0;
let currentTooltipY = 0.0;
let currentTooltipZ = 2.0;
let targetSpeed = 1.0; // speed of coordinate shift
let linearSens = 0.8;
let angularSens = 1.0;
let currentGripperTarget = 0.0;

// Driving state variables
let linearSpeed = 0.0;
let angularSpeed = 0.0;
let armManualVel = 0.0;
let gripperManualVel = 0.0;
let armUpBtnActive = false;
let armDownBtnActive = false;
let gripperOpenBtnActive = false;
let gripperCloseBtnActive = false;
let gamepadConnected = false;
let lastGamepadOpenPressed = false;
let lastGamepadClosePressed = false;
let driveIntervalId = null;

// Keyboard input state tracking
const keysPressed = {};

// DOM Elements
const connectionDot = document.getElementById('connection-status-dot');
const connectionText = document.getElementById('connection-status-text');
const ipInput = document.getElementById('rosbridge-url');
const connectBtn = document.getElementById('btn-connect');

const gamepadDot = document.getElementById('gamepad-status-dot');
const gamepadText = document.getElementById('gamepad-status-text');
const gamepadInputsDisplay = document.getElementById('gamepad-inputs-display');
const gamepadValL = document.getElementById('gamepad-val-l');
const gamepadValR = document.getElementById('gamepad-val-r');
const gamepadValBtns = document.getElementById('gamepad-val-btns');
const cameraViewModeSelect = document.getElementById('camera-view-mode');
let currentCameraMode = 'free'; // 'free', 'orbit', 'chase'

const autoBtn = document.getElementById('btn-mode-auto');
const manualBtn = document.getElementById('btn-mode-manual');
const modeDesc = document.getElementById('mode-desc-text');

const autoContainer = document.getElementById('auto-controls-container');
const manualContainer = document.getElementById('manual-controls-container');
const controlPanelTitle = document.getElementById('control-panel-title');
const controlPanelTag = document.getElementById('control-panel-tag');

const sliderX = document.getElementById('slider-target-x');
const sliderY = document.getElementById('slider-target-y');
const sliderZ = document.getElementById('slider-target-z');
const sliderSpeed = document.getElementById('slider-target-speed');
const sliderLinearSens = document.getElementById('slider-linear-sens');
const sliderAngularSens = document.getElementById('slider-angular-sens');

const lblX = document.getElementById('lbl-target-x');
const lblY = document.getElementById('lbl-target-y');
const lblZ = document.getElementById('lbl-target-z');
const lblSpeed = document.getElementById('lbl-target-speed');
const lblLinearSens = document.getElementById('lbl-linear-sens');
const lblAngularSens = document.getElementById('lbl-angular-sens');

const btnToggleSensitivity = document.getElementById('btn-toggle-sensitivity');
const sensitivityDrawer = document.getElementById('sensitivity-drawer');
const sensitivityCollapseIcon = document.getElementById('sensitivity-collapse-icon');

const btnToggleModules = document.getElementById('btn-toggle-modules');
const modulesDrawer = document.getElementById('modules-drawer');
const modulesCollapseIcon = document.getElementById('modules-collapse-icon');

const btnToggleView = document.getElementById('btn-toggle-view');
const viewportContainer = document.getElementById('viewport-container');
const urdfViewerContainer = document.getElementById('urdf-viewer-container');

const valTooltipPos = document.getElementById('val-tooltip-pos');
const valBasePos = document.getElementById('val-base-pos');
const valBaseYaw = document.getElementById('val-base-yaw');
const valArmJoint = document.getElementById('val-arm-joint');
const valGripper = document.getElementById('val-gripper-spacing');
const valWheelSpeed = document.getElementById('val-wheel-speed');

// HUD indicators
const hudMode = document.getElementById('hud-mode');
const hudTargetCoords = document.getElementById('hud-target-coords');
const hudLatency = document.getElementById('hud-latency');
const hudPitch = document.getElementById('hud-pitch');
const hudRoll = document.getElementById('hud-roll');

// Virtual Joystick elements
const joystickPad = document.getElementById('joystick-pad');
const joystickHandle = document.getElementById('joystick-handle');
let joystickActive = false;
let joystickStartX = 0;
let joystickStartY = 0;
const joystickMaxRadius = 50; // max travel distance in pixels

// ----------------------------------------------------
// 1. ROS BRIDGE CONNECTION LOGIC
// ----------------------------------------------------
function connectROS() {
    if (connected) {
        // Disconnect
        ros.close();
        return;
    }

    const url = ipInput.value;
    connectionText.innerText = "CONNECTING...";
    connectBtn.innerText = "WAITING...";

    ros = new ROSLIB.Ros({
        url: url
    });

    ros.on('connection', () => {
        connected = true;
        connectionDot.className = 'status-indicator connected';
        connectionText.innerText = "CONNECTED";
        connectBtn.innerText = "DISCONNECT";
        connectBtn.className = "neon-btn-red";
        console.log('Successfully connected to Rosbridge WebSocket Server.');
        hudLatency.innerText = "2 ms";
        
        // Advertise and Subscribe topics
        setupROSInterfaces();
    });

    ros.on('error', (error) => {
        connected = false;
        connectionDot.className = 'status-indicator disconnected';
        connectionText.innerText = "ERROR";
        connectBtn.innerText = "RETRY";
        connectBtn.className = "neon-btn-blue";
        console.error('Error connecting to Rosbridge: ', error);
        hudLatency.innerText = "-- ms";
    });

    ros.on('close', () => {
        connected = false;
        connectionDot.className = 'status-indicator disconnected';
        connectionText.innerText = "DISCONNECTED";
        connectBtn.innerText = "CONNECT";
        connectBtn.className = "neon-btn-blue";
        console.log('Connection to Rosbridge closed.');
        hudLatency.innerText = "-- ms";
        cleanupROSInterfaces();
    });
}

function setupROSInterfaces() {
    // Publishers
    cmdVelPub = new ROSLIB.Topic({
        ros: ros,
        name: '/cmd_vel',
        messageType: 'geometry_msgs/msg/Twist'
    });

    armTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_target',
        messageType: 'geometry_msgs/msg/Point'
    });

    autoModePub = new ROSLIB.Topic({
        ros: ros,
        name: '/auto_mode',
        messageType: 'std_msgs/msg/Bool'
    });

    armManualVelPub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_manual_vel',
        messageType: 'std_msgs/msg/Float32'
    });

    gripperManualVelPub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_manual_vel',
        messageType: 'std_msgs/msg/Float32'
    });

    gripperTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_target',
        messageType: 'std_msgs/msg/Float32'
    });

    lightsPub = new ROSLIB.Topic({
        ros: ros,
        name: '/lights',
        messageType: 'std_msgs/msg/Bool'
    });

    servoCommandsPub = new ROSLIB.Topic({
        ros: ros,
        name: '/servo_commands',
        messageType: 'std_msgs/msg/String'
    });

    // Action Client for gripper
    gripperActionClient = new ROSLIB.ActionClient({
        ros: ros,
        serverName: '/gripper_command',
        actionName: 'arm_interfaces/GripperCommand'
    });

    // Subscribers
    gripperStateSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_state',
        messageType: 'std_msgs/msg/Float32'
    });
    gripperStateSub.subscribe((msg) => {
        const progress = msg.data;
        if (valGripper) {
            valGripper.innerText = `${((1.0 - progress) * 0.2).toFixed(3)} m (${(progress * 100).toFixed(0)}% CLOSED)`;
        }
        
        const slider = document.getElementById('slider-gripper');
        const lbl = document.getElementById('lbl-gripper-val');
        const isSliderActive = slider && document.activeElement === slider;
        
        if (!isSliderActive) {
            currentGripperTarget = progress;
            if (slider) {
                slider.value = progress;
            }
            if (lbl) {
                lbl.innerText = progress.toFixed(2);
            }
        }
    });

    armGlobalPosSub = new ROSLIB.Topic({
        ros: ros,
        name: '/arm_global_pos',
        messageType: 'geometry_msgs/msg/Point'
    });
    armGlobalPosSub.subscribe((msg) => {
        currentTooltipX = msg.x;
        currentTooltipY = msg.y;
        currentTooltipZ = msg.z;
        valTooltipPos.innerText = `X: ${msg.x.toFixed(2)}, Y: ${msg.y.toFixed(2)}, Z: ${msg.z.toFixed(2)}`;
    });

    jointStateSub = new ROSLIB.Topic({
        ros: ros,
        name: '/joint_states',
        messageType: 'sensor_msgs/msg/JointState'
    });
    jointStateSub.subscribe((msg) => {
        // Find arm_joint
        let armIdx = msg.name.indexOf('arm_joint');
        if (armIdx !== -1) {
            valArmJoint.innerText = `${msg.position[armIdx].toFixed(3)} rad`;
        }

        // Find wheel speeds for telemetry UI
        let flIdx = msg.name.indexOf('front_left_wheel_joint');
        let frIdx = msg.name.indexOf('front_right_wheel_joint');
        if (flIdx !== -1 && frIdx !== -1) {
            // Convert simulated delta step angle into pseudo power levels for visual show
            // js base updates fl_vel and fr_vel
            let flVal = Math.sin(Date.now() / 200) * 10; // dummy display speed or map directly
            valWheelSpeed.innerText = `FL: ${(msg.position[flIdx] !== undefined) ? 'ACTIVE' : 'IDLE'}, FR: ${(msg.position[frIdx] !== undefined) ? 'ACTIVE' : 'IDLE'}`;
        }
    });

    tfSub = new ROSLIB.Topic({
        ros: ros,
        name: '/tf',
        messageType: 'tf2_msgs/msg/TFMessage'
    });
    tfSub.subscribe((msg) => {
        for (let i = 0; i < msg.transforms.length; i++) {
            let t = msg.transforms[i];
            // Accept TF transforms from authoritative odom or map frames to avoid collisions
            if (t.child_frame_id === 'base_link' && (t.header.frame_id === 'odom' || t.header.frame_id === 'map')) {
                let x = t.transform.translation.x;
                let y = t.transform.translation.y;
                if (valBasePos) valBasePos.innerText = `X: ${x.toFixed(2)}, Y: ${y.toFixed(2)}`;

                // Extract Heading (Yaw) from 3D Quaternion
                let qx = t.transform.rotation.x || 0.0;
                let qy = t.transform.rotation.y || 0.0;
                let qz = t.transform.rotation.z || 0.0;
                let qw = t.transform.rotation.w || 1.0;
                let siny_cosp = 2.0 * (qw * qz + qx * qy);
                let cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz);
                let yaw = Math.atan2(siny_cosp, cosy_cosp);
                let deg = yaw * (180.0 / Math.PI);
                if (valBaseYaw) valBaseYaw.innerText = `${yaw.toFixed(2)} rad (${deg.toFixed(1)}°)`;
            }
        }
    });

    processStatusSub = new ROSLIB.Topic({
        ros: ros,
        name: '/process_manager/status',
        messageType: 'std_msgs/msg/String'
    });
    processStatusSub.subscribe((msg) => {
        try {
            const status = JSON.parse(msg.data);
            updateModuleStatusUI(status);

            const isStatePubActive = status["state_publisher"] && status["state_publisher"].running;
            if (isStatePubActive && !robotStatePublisherRunning) {
                console.log("Robot State Publisher started running. Loading URDF...");
                robotStatePublisherRunning = true;
                if (showing3D && viewer3D) {
                    loadURDFClient();
                }
            } else if (!isStatePubActive && robotStatePublisherRunning) {
                console.log("Robot State Publisher stopped. Cleaning up URDF...");
                robotStatePublisherRunning = false;
                cleanupURDFClient();
            }
        } catch (e) {
            console.error("Failed to parse process status JSON:", e);
        }
    });

    const batteryStatusSub = new ROSLIB.Topic({
        ros: ros,
        name: '/battery/status',
        messageType: 'std_msgs/msg/String'
    });
    batteryStatusSub.subscribe((msg) => {
        try {
            const batt = JSON.parse(msg.data);
            document.getElementById('batt-val-soc').innerText = `${batt.SOC}%`;
            document.getElementById('batt-val-volts').innerText = `${batt.voltage}V`;
            document.getElementById('batt-val-amps').innerText = `${batt.current}A`;
            document.getElementById('batt-val-status').innerText = batt.battery_status ? batt.battery_status.toUpperCase() : "OK";
        } catch (e) {
            console.error("Failed to parse battery JSON:", e);
        }
    });

    // Wheel motor and velocity subscribers
    flMotorSub = new ROSLIB.Topic({
        ros: ros,
        name: '/motor/front_left',
        messageType: 'std_msgs/msg/Float32'
    });
    flMotorSub.subscribe((msg) => {
        updateWheelPowerUI('fl', msg.data);
    });

    frMotorSub = new ROSLIB.Topic({
        ros: ros,
        name: '/motor/front_right',
        messageType: 'std_msgs/msg/Float32'
    });
    frMotorSub.subscribe((msg) => {
        updateWheelPowerUI('fr', msg.data);
    });

    rlMotorSub = new ROSLIB.Topic({
        ros: ros,
        name: '/motor/rear_left',
        messageType: 'std_msgs/msg/Float32'
    });
    rlMotorSub.subscribe((msg) => {
        updateWheelPowerUI('rl', msg.data);
    });

    rrMotorSub = new ROSLIB.Topic({
        ros: ros,
        name: '/motor/rear_right',
        messageType: 'std_msgs/msg/Float32'
    });
    rrMotorSub.subscribe((msg) => {
        updateWheelPowerUI('rr', msg.data);
    });

    cmdVelSub = new ROSLIB.Topic({
        ros: ros,
        name: '/cmd_vel',
        messageType: 'geometry_msgs/msg/Twist'
    });
    cmdVelSub.subscribe((msg) => {
        const linEl = document.getElementById('wheel-mon-linear');
        const angEl = document.getElementById('wheel-mon-angular');
        if (linEl) linEl.innerText = `${msg.linear.x.toFixed(2)} m/s`;
        if (angEl) angEl.innerText = `${msg.angular.z.toFixed(2)} rad/s`;
    });

    // Send initial auto mode status
    publishMode();

    setupGPSROSInterfaces();
    setupIMUROSInterfaces();
    setupMorseSubscribers();

    if (showing3D && !viewer3D) {
        init3DViewer();
    }
}

function updateWheelPowerUI(wheel, value) {
    const percent = Math.min(100, Math.max(0, Math.round(Math.abs(value) * 100)));
    const fillEl = document.getElementById(`wheel-fill-${wheel}`);
    const valEl = document.getElementById(`wheel-val-${wheel}`);
    
    if (fillEl) {
        fillEl.style.height = `${percent}%`;
        if (value < 0) {
            fillEl.classList.add('reverse');
        } else {
            fillEl.classList.remove('reverse');
        }
    }
    
    if (valEl) {
        const sign = value > 0 ? '+' : (value < 0 ? '-' : '');
        valEl.innerText = `${sign}${percent}%`;
        if (value > 0) {
            valEl.style.color = 'var(--accent-green)';
        } else if (value < 0) {
            valEl.style.color = 'var(--accent-red)';
        } else {
            valEl.style.color = 'var(--text-main)';
        }
    }
}

function cleanupROSInterfaces() {
    if (jointStateSub) jointStateSub.unsubscribe();
    if (armGlobalPosSub) armGlobalPosSub.unsubscribe();
    if (gripperStateSub) { gripperStateSub.unsubscribe(); gripperStateSub = null; }
    if (tfSub) tfSub.unsubscribe();
    if (processStatusSub) {
        processStatusSub.unsubscribe();
        processStatusSub = null;
    }
    
    if (flMotorSub) { flMotorSub.unsubscribe(); flMotorSub = null; }
    if (frMotorSub) { frMotorSub.unsubscribe(); frMotorSub = null; }
    if (rlMotorSub) { rlMotorSub.unsubscribe(); rlMotorSub = null; }
    if (rrMotorSub) { rrMotorSub.unsubscribe(); rrMotorSub = null; }
    if (cmdVelSub) { cmdVelSub.unsubscribe(); cmdVelSub = null; }

    cleanupGPSROSInterfaces();
    cleanupIMUROSInterfaces();
    cleanupMorseSubscribers();

    resetModuleStatusUI();
    robotStatePublisherRunning = false;
    
    cmdVelPub = null;
    armTargetPub = null;
    autoModePub = null;
    armManualVelPub = null;
    gripperManualVelPub = null;
    gripperTargetPub = null;
    gripperActionClient = null;

    cleanup3DViewer();
}

// ----------------------------------------------------
// 2. MODE HANDLING CONTROLS
// ----------------------------------------------------
function setControlMode(isAuto) {
    autoMode = isAuto;
    
    if (autoMode) {
        autoBtn.classList.add('active');
        manualBtn.classList.remove('active');
        modeDesc.innerText = "Auto Mode: Adjust sliders or press keys to translate the 3D target point. The rover base and arm move automatically using inverse kinematics.";
        hudMode.innerText = "AUTO (IK)";
        hudMode.className = "neon-text-amber";
        
        autoContainer.classList.remove('hidden');
        manualContainer.classList.add('hidden');
        controlPanelTitle.innerText = "CONTROLS";
        controlPanelTag.innerText = "COORD_INPUT";
        
        // Match target coordinates to physical tooltip position on switch
        targetX = currentTooltipX;
        targetY = currentTooltipY;
        targetZ = currentTooltipZ;
        updateCoordinateSliders();
        publishArmTarget();
        
        stopRoverMovement();
    } else {
        manualBtn.classList.add('active');
        autoBtn.classList.remove('active');
        modeDesc.innerText = "Manual Mode: Directly steer the rover using keyboard (WASD / Arrows) or the virtual joystick. The motion coordinator solver is disabled.";
        hudMode.innerText = "MANUAL";
        hudMode.className = "neon-text-green";
        
        manualContainer.classList.remove('hidden');
        autoContainer.classList.add('hidden');
        controlPanelTitle.innerText = "CONTROLS";
        controlPanelTag.innerText = "DRIVE_ACTUATE";
        
        stopRoverMovement();
    }
    
    publishMode();
}

function publishMode() {
    if (connected && autoModePub) {
        const msg = new ROSLIB.Message({
            data: autoMode
        });
        autoModePub.publish(msg);
    }
}

// ----------------------------------------------------
// 3. AUTO TARGET COORDINATES MANAGER
// ----------------------------------------------------
function publishArmTarget() {
    if (connected && armTargetPub) {
        const msg = new ROSLIB.Message({
            x: targetX,
            y: targetY,
            z: targetZ
        });
        armTargetPub.publish(msg);
    }
    
    // Update local HUD
    hudTargetCoords.innerText = `X: ${targetX.toFixed(1)}, Y: ${targetY.toFixed(1)}, Z: ${targetZ.toFixed(1)}`;
}

function getCameraRelativeDirections() {
    const forward = new THREE.Vector3(1, 0, 0);
    const right = new THREE.Vector3(0, -1, 0);
    
    if (typeof THREE !== 'undefined' && viewer3D && viewer3D.camera) {
        const cam = viewer3D.camera;
        const matrix = cam.matrixWorld;
        
        const camForward = new THREE.Vector3();
        cam.getWorldDirection(camForward);
        
        if (Math.abs(camForward.z) > 0.99) {
            // Looking straight down or up: use camera's local up vector projected to XY
            forward.set(matrix.elements[4], matrix.elements[5], 0);
        } else {
            forward.set(camForward.x, camForward.y, 0);
        }
        forward.normalize();
        
        // Compute right vector orthogonal to forward and world up (0,0,1)
        right.crossVectors(forward, new THREE.Vector3(0, 0, 1));
        right.normalize();
    }
    
    return { forward, right };
}

function updateCoordinateSliders() {
    sliderX.value = targetX;
    sliderY.value = targetY;
    sliderZ.value = targetZ;
    lblX.innerText = targetX.toFixed(1);
    lblY.innerText = targetY.toFixed(1);
    lblZ.innerText = targetZ.toFixed(1);
}

// Slider event listeners
sliderX.addEventListener('input', (e) => {
    targetX = parseFloat(e.target.value);
    lblX.innerText = targetX.toFixed(1);
    publishArmTarget();
});
sliderY.addEventListener('input', (e) => {
    targetY = parseFloat(e.target.value);
    lblY.innerText = targetY.toFixed(1);
    publishArmTarget();
});
sliderZ.addEventListener('input', (e) => {
    targetZ = parseFloat(e.target.value);
    lblZ.innerText = targetZ.toFixed(1);
    publishArmTarget();
});
sliderSpeed.addEventListener('input', (e) => {
    targetSpeed = parseFloat(e.target.value);
    lblSpeed.innerText = targetSpeed.toFixed(1);
});

sliderLinearSens.addEventListener('input', (e) => {
    linearSens = parseFloat(e.target.value);
    lblLinearSens.innerText = linearSens.toFixed(1);
});

sliderAngularSens.addEventListener('input', (e) => {
    angularSens = parseFloat(e.target.value);
    lblAngularSens.innerText = angularSens.toFixed(1);
});

btnToggleSensitivity.addEventListener('click', () => {
    sensitivityDrawer.classList.toggle('collapsed');
    sensitivityCollapseIcon.classList.toggle('collapsed');
});

// ----------------------------------------------------
// 4. MANUAL DRIVE TELEOP (WASD & Joystick)
// ----------------------------------------------------
function getActiveGamepad() {
    if (!navigator.getGamepads) return null;
    const gamepads = navigator.getGamepads();
    for (let i = 0; i < gamepads.length; i++) {
        if (gamepads[i] && gamepads[i].connected) {
            return gamepads[i];
        }
    }
    return null;
}

let gamepadUIIntervalId = null;

function startGamepadUIInterval() {
    if (gamepadUIIntervalId) return;
    gamepadUIIntervalId = setInterval(() => {
        const gp = getActiveGamepad();
        if (gp) {
            let gpSteering = gp.axes[0] !== undefined ? gp.axes[0] : 0.0;
            let gpThrottle = gp.axes[1] !== undefined ? gp.axes[1] : 0.0;
            let gpArm = gp.axes[3] !== undefined ? gp.axes[3] : 0.0;
            
            if (gamepadValL) {
                gamepadValL.innerText = `X: ${gpSteering.toFixed(2)} Y: ${gpThrottle.toFixed(2)}`;
            }
            if (gamepadValR) {
                gamepadValR.innerText = gpArm.toFixed(2);
            }
            
            let activeBtnText = "-";
            let openPressed = false;
            if ((gp.buttons[4] && gp.buttons[4].pressed) || (gp.buttons[2] && gp.buttons[2].pressed)) {
                openPressed = true;
            }

            let closePressed = false;
            if ((gp.buttons[5] && gp.buttons[5].pressed) || (gp.buttons[0] && gp.buttons[0].pressed)) {
                closePressed = true;
            }

            if (openPressed && closePressed) {
                activeBtnText = "OPEN & CLOSE";
            } else if (openPressed) {
                activeBtnText = "OPEN";
            } else if (closePressed) {
                activeBtnText = "CLOSE";
            }
            
            if (gamepadValBtns) {
                gamepadValBtns.innerText = activeBtnText;
            }
        }
    }, 100);
}

function stopGamepadUIInterval() {
    if (gamepadUIIntervalId) {
        clearInterval(gamepadUIIntervalId);
        gamepadUIIntervalId = null;
    }
}

function showGamepadStatus(connected, id) {
    if (connected) {
        if (gamepadDot) gamepadDot.className = 'status-indicator connected';
        if (gamepadText) gamepadText.innerText = id ? id.substring(0, 15).toUpperCase() : "GAMEPAD ACTIVE";
        if (gamepadInputsDisplay) gamepadInputsDisplay.classList.remove('hidden');
        startGamepadUIInterval();
    } else {
        if (gamepadDot) gamepadDot.className = 'status-indicator disconnected';
        if (gamepadText) gamepadText.innerText = "NO GAMEPAD";
        if (gamepadInputsDisplay) gamepadInputsDisplay.classList.add('hidden');
        stopGamepadUIInterval();
    }
}

function startControlLoopTimer() {
    if (driveIntervalId) return;
    
    driveIntervalId = setInterval(() => {
        let gpDrivingActive = false;
        let gpArmActive = false;
        const gp = getActiveGamepad();
        
        if (gp) {
            if (!gamepadConnected) {
                gamepadConnected = true;
                showGamepadStatus(true, gp.id);
            }
            
            // Read stick states
            let gpSteering = gp.axes[0] !== undefined ? gp.axes[0] : 0.0;
            let gpThrottle = gp.axes[1] !== undefined ? gp.axes[1] : 0.0;
            let gpArm = gp.axes[3] !== undefined ? gp.axes[3] : 0.0;
            
            // Deadzone
            const deadzone = 0.1;
            const hasSteering = Math.abs(gpSteering) >= deadzone;
            const hasThrottle = Math.abs(gpThrottle) >= deadzone;
            const hasArm = Math.abs(gpArm) >= deadzone;
            
            if (autoMode) {
                // AUTO MODE: Gamepad controls targets
                let coordsChanged = false;
                const dirs = getCameraRelativeDirections();
                const step = targetSpeed * 0.2;
                
                // Left Stick Y (gpThrottle) controls camera-relative Forward/Backward
                if (hasThrottle) {
                    targetX += -gpThrottle * dirs.forward.x * step;
                    targetY += -gpThrottle * dirs.forward.y * step;
                    coordsChanged = true;
                }
                // Left Stick X (gpSteering) controls camera-relative Left/Right
                if (hasSteering) {
                    targetX += gpSteering * dirs.right.x * step;
                    targetY += gpSteering * dirs.right.y * step;
                    coordsChanged = true;
                }
                // Right Stick Y (gpArm) controls Target Z
                if (hasArm) {
                    targetZ += -gpArm * step;
                    coordsChanged = true;
                }
                
                if (coordsChanged) {
                    updateCoordinateSliders();
                    publishArmTarget();
                }
            } else {
                // MANUAL MODE: Gamepad controls driving & arm velocities
                if (hasSteering || hasThrottle) {
                    linearSpeed = -gpThrottle * 0.8 * linearSens;
                    angularSpeed = -gpSteering * 1.0 * angularSens;
                    gpDrivingActive = true;
                }
                
                if (hasArm) {
                    armManualVel = gpArm * 0.3; // Inverted sign so joystick up (negative axis) moves arm up
                    gpArmActive = true;
                }
            }
            
            // Process gripper buttons with edge-detection (works in both modes)
            let openPressed = false;
            if ((gp.buttons[4] && gp.buttons[4].pressed) || (gp.buttons[2] && gp.buttons[2].pressed)) {
                openPressed = true;
            }

            let closePressed = false;
            if ((gp.buttons[5] && gp.buttons[5].pressed) || (gp.buttons[0] && gp.buttons[0].pressed)) {
                closePressed = true;
            }

            if (openPressed) {
                const oldVal = currentGripperTarget;
                currentGripperTarget = Math.max(0.0, currentGripperTarget - 0.05);
                if (Math.abs(currentGripperTarget - oldVal) > 0.001) {
                    publishGripperTarget(currentGripperTarget);
                }
            }
            if (closePressed) {
                const oldVal = currentGripperTarget;
                currentGripperTarget = Math.min(1.0, currentGripperTarget + 0.05);
                if (Math.abs(currentGripperTarget - oldVal) > 0.001) {
                    publishGripperTarget(currentGripperTarget);
                }
            }
        } else {
            if (gamepadConnected) {
                gamepadConnected = false;
                showGamepadStatus(false, "");
            }
        }
        
        // UI Button & Keyboard (Z / C) Gripper control (runs in both modes, whether gamepad is connected or not)
        if (gripperOpenBtnActive || keysPressed['z']) {
            const oldVal = currentGripperTarget;
            currentGripperTarget = Math.max(0.0, currentGripperTarget - 0.05);
            if (Math.abs(currentGripperTarget - oldVal) > 0.001) {
                publishGripperTarget(currentGripperTarget);
            }
        }
        if (gripperCloseBtnActive || keysPressed['c']) {
            const oldVal = currentGripperTarget;
            currentGripperTarget = Math.min(1.0, currentGripperTarget + 0.05);
            if (Math.abs(currentGripperTarget - oldVal) > 0.001) {
                publishGripperTarget(currentGripperTarget);
            }
        }
        
        // Manual controls fallbacks & publishers (only active in manual mode)
        if (!autoMode) {
            // If gamepad did not override drive speed, fall back to keyboard/joystick
            if (!gpDrivingActive) {
                if (!joystickActive) {
                    calculateKeyboardVelocities();
                }
            }
            
            // If gamepad did not override arm speed, fall back to keyboard/buttons
            if (!gpArmActive) {
                calculateManualArmVelocities();
            }
            
            calculateManualGripperVelocities();
            
            publishDriveCommand();
            publishArmManualCommand();
            publishGripperManualCommand();
        }
    }, 100); // 10Hz
}

function stopRoverMovement() {
    // Publish stop command directly without checking autoMode
    linearSpeed = 0.0;
    angularSpeed = 0.0;
    if (connected && cmdVelPub) {
        const msg = new ROSLIB.Message({
            linear: { x: 0.0, y: 0.0, z: 0.0 },
            angular: { x: 0.0, y: 0.0, z: 0.0 }
        });
        cmdVelPub.publish(msg);
    }

    armManualVel = 0.0;
    gripperManualVel = 0.0;
    if (connected && armManualVelPub) {
        const msg = new ROSLIB.Message({
            data: 0.0
        });
        armManualVelPub.publish(msg);
    }
    if (connected && gripperManualVelPub) {
        const msg = new ROSLIB.Message({
            data: 0.0
        });
        gripperManualVelPub.publish(msg);
    }
}

function calculateManualArmVelocities() {
    let speed = 0.0;
    // Q/PageUp moves arm UP, E/PageDown moves arm DOWN
    if (keysPressed['q'] || keysPressed['pageup'] || armUpBtnActive) {
        speed += 0.3; // Arm UP (Positive)
    }
    if (keysPressed['e'] || keysPressed['pagedown'] || armDownBtnActive) {
        speed -= 0.3; // Arm DOWN (Negative)
    }
    armManualVel = speed;
}

function publishArmManualCommand() {
    if (connected && armManualVelPub && !autoMode) {
        const msg = new ROSLIB.Message({
            data: armManualVel
        });
        armManualVelPub.publish(msg);
    }
}

function calculateManualGripperVelocities() {
    let speed = 0.0;
    // Z / Button Open moves gripper OPEN, C / Button Close moves gripper CLOSE
    if (gripperOpenBtnActive || keysPressed['z']) {
        speed += 0.5; // Gripper OPEN (Positive)
    }
    if (gripperCloseBtnActive || keysPressed['c']) {
        speed -= 0.5; // Gripper CLOSE (Negative)
    }
    gripperManualVel = speed;
}

function publishGripperManualCommand() {
    if (connected && gripperManualVelPub && !autoMode) {
        const msg = new ROSLIB.Message({
            data: gripperManualVel
        });
        gripperManualVelPub.publish(msg);
    }
}

function calculateKeyboardVelocities() {
    // Only calculate keyboard velocity if joystick is inactive
    if (joystickActive) return;

    let forward = 0.0;
    let turn = 0.0;

    if (keysPressed['w'] || keysPressed['arrowup']) {
        forward += 1.0;
        document.getElementById('key-up').classList.add('active');
    } else {
        document.getElementById('key-up').classList.remove('active');
    }

    if (keysPressed['s'] || keysPressed['arrowdown']) {
        forward -= 1.0;
        document.getElementById('key-down').classList.add('active');
    } else {
        document.getElementById('key-down').classList.remove('active');
    }

    if (keysPressed['a'] || keysPressed['arrowleft']) {
        turn += 1.0;
        document.getElementById('key-left').classList.add('active');
    } else {
        document.getElementById('key-left').classList.remove('active');
    }

    if (keysPressed['d'] || keysPressed['arrowright']) {
        turn -= 1.0;
        document.getElementById('key-right').classList.add('active');
    } else {
        document.getElementById('key-right').classList.remove('active');
    }

    linearSpeed = forward * 0.8 * linearSens;  // max linear speed scaling
    angularSpeed = turn * 1.0 * angularSens;   // max angular speed scaling
}

function publishDriveCommand() {
    if (connected && cmdVelPub && !autoMode) {
        const msg = new ROSLIB.Message({
            linear: { x: linearSpeed, y: 0.0, z: 0.0 },
            angular: { x: 0.0, y: 0.0, z: angularSpeed }
        });
        cmdVelPub.publish(msg);
    }
}

// Virtual Joystick Drag Handling
function handleJoystickStart(e) {
    if (autoMode) return;
    joystickActive = true;
    
    const rect = joystickPad.getBoundingClientRect();
    joystickStartX = rect.left + rect.width / 2;
    joystickStartY = rect.top + rect.height / 2;
    
    handleJoystickMove(e);
}

function handleJoystickMove(e) {
    if (!joystickActive) return;
    
    let clientX = e.clientX || (e.touches && e.touches[0].clientX);
    let clientY = e.clientY || (e.touches && e.touches[0].clientY);
    
    if (!clientX || !clientY) return;

    let deltaX = clientX - joystickStartX;
    let deltaY = clientY - joystickStartY;
    
    let distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
    
    if (distance > joystickMaxRadius) {
        deltaX = (deltaX / distance) * joystickMaxRadius;
        deltaY = (deltaY / distance) * joystickMaxRadius;
        distance = joystickMaxRadius;
    }
    
    // Update visual handle position
    joystickHandle.style.transform = `translate(calc(-50% + ${deltaX}px), calc(-50% + ${deltaY}px))`;
    
    // Map coordinates to ROS Twist speeds:
    // deltaY pushes forward/backward (inverted because up is negative Y in browser screen)
    // deltaX steers left/right
    linearSpeed = -(deltaY / joystickMaxRadius) * 0.8 * linearSens;
    angularSpeed = -(deltaX / joystickMaxRadius) * 1.0 * angularSens;
}

function handleJoystickEnd() {
    if (!joystickActive) return;
    joystickActive = false;
    
    // Reset visual handle
    joystickHandle.style.transform = 'translate(-50%, -50%)';
    
    // Stop the rover
    linearSpeed = 0.0;
    angularSpeed = 0.0;
}

// Joystick Event Listeners
joystickPad.addEventListener('mousedown', handleJoystickStart);
window.addEventListener('mousemove', handleJoystickMove);
window.addEventListener('mouseup', handleJoystickEnd);

// Touch support for mobile layouts
joystickPad.addEventListener('touchstart', handleJoystickStart);
window.addEventListener('touchmove', handleJoystickMove);
window.addEventListener('touchend', handleJoystickEnd);

// Gamepad Connection Listeners
window.addEventListener("gamepadconnected", (e) => {
    console.log("Gamepad connected:", e.gamepad.id);
    gamepadConnected = true;
    showGamepadStatus(true, e.gamepad.id);
});

window.addEventListener("gamepaddisconnected", (e) => {
    console.log("Gamepad disconnected:", e.gamepad.id);
    const gp = getActiveGamepad();
    if (gp) {
        showGamepadStatus(true, gp.id);
    } else {
        gamepadConnected = false;
        showGamepadStatus(false, "");
    }
});

// ----------------------------------------------------
// 5. KEYBOARD LISTENERS FOR SHIFTING TARGETS / TELEOP
// ----------------------------------------------------
window.addEventListener('keydown', (e) => {
    const key = e.key.toLowerCase();
    
    // Prevent default scroll behavior for PageUp/PageDown keys
    if (key === 'pageup' || key === 'pagedown') {
        e.preventDefault();
    }
    
    keysPressed[key] = true;
    
    if (autoMode) {
        let coordsChanged = false;
        const dirs = getCameraRelativeDirections();
        const step = targetSpeed * 0.5;
        
        // Target shifting controls
        if (key === 'w') {
            targetX += dirs.forward.x * step;
            targetY += dirs.forward.y * step;
            coordsChanged = true;
        } else if (key === 's') {
            targetX -= dirs.forward.x * step;
            targetY -= dirs.forward.y * step;
            coordsChanged = true;
        }
        
        if (key === 'a') {
            // Left moves target opposite to camera's right vector
            targetX -= dirs.right.x * step;
            targetY -= dirs.right.y * step;
            coordsChanged = true;
        } else if (key === 'd') {
            // Right moves target along camera's right vector
            targetX += dirs.right.x * step;
            targetY += dirs.right.y * step;
            coordsChanged = true;
        }
        
        if (key === 'q') {
            targetZ += step;
            coordsChanged = true;
        } else if (key === 'e') {
            targetZ -= step;
            coordsChanged = true;
        }
        
        if (coordsChanged) {
            updateCoordinateSliders();
            publishArmTarget();
        }
    }
});

window.addEventListener('keyup', (e) => {
    const key = e.key.toLowerCase();
    keysPressed[key] = false;
    
    if (!autoMode) {
        // Remove active visual class on key releases
        if (key === 'w' || key === 'arrowup') document.getElementById('key-up').classList.remove('active');
        if (key === 's' || key === 'arrowdown') document.getElementById('key-down').classList.remove('active');
        if (key === 'a' || key === 'arrowleft') document.getElementById('key-left').classList.remove('active');
        if (key === 'd' || key === 'arrowright') document.getElementById('key-right').classList.remove('active');
    }
});

// ----------------------------------------------------
// 6. GRIPPER COMMAND ACTION AND TOPIC CALLS
// ----------------------------------------------------
function publishGripperTarget(val) {
    if (connected && gripperTargetPub) {
        const msg = new ROSLIB.Message({
            data: val
        });
        gripperTargetPub.publish(msg);
    }
}

function sendGripperCommand(position) {
    if (!connected || !gripperActionClient) {
        console.warn('Cannot send gripper goal: ROS Disconnected.');
        return;
    }

    console.log(`Sending Gripper command goal to server: position = ${position}`);
    
    const goal = new ROSLIB.Goal({
        actionClient: gripperActionClient,
        goalMessage: {
            position: position
        }
    });

    goal.on('feedback', (feedback) => {
        valGripper.innerText = `${((1.0 - feedback.progress) * 0.2).toFixed(3)} m (${(feedback.progress * 100).toFixed(0)}% CLOSED)`;
    });

    goal.on('result', (result) => {
        if (result.success) {
            console.log('Gripper command succeeded.');
            valGripper.innerText = position === 1.0 ? "0.000 m (CLOSED)" : "0.200 m (OPEN)";
        }
    });

    goal.send();
}

const gripperOpenBtn = document.getElementById('btn-gripper-open');
const gripperCloseBtn = document.getElementById('btn-gripper-close');

const lightOnBtn = document.getElementById('btn-light-on');
const lightOffBtn = document.getElementById('btn-light-off');

if (lightOnBtn) {
    lightOnBtn.addEventListener('click', () => {
        if (!lightsPub) return;
        const msg = new ROSLIB.Message({ data: true });
        lightsPub.publish(msg);
        console.log("Published Lights ON");
    });
}
if (lightOffBtn) {
    lightOffBtn.addEventListener('click', () => {
        if (!lightsPub) return;
        const msg = new ROSLIB.Message({ data: false });
        lightsPub.publish(msg);
        console.log("Published Lights OFF");
    });
}

const servo1LeftBtn = document.getElementById('btn-servo1-left');
const servo1RightBtn = document.getElementById('btn-servo1-right');
const servo2LeftBtn = document.getElementById('btn-servo2-left');
const servo2RightBtn = document.getElementById('btn-servo2-right');

function publishServoCommand(cmd) {
    if (!servoCommandsPub) return;
    const msg = new ROSLIB.Message({ data: cmd });
    servoCommandsPub.publish(msg);
    console.log("Published Servo Command: " + cmd);
}

function bindServoBtn(btn, moveCmd, stopCmd) {
    if (!btn) return;
    btn.addEventListener('mousedown', () => publishServoCommand(moveCmd));
    btn.addEventListener('touchstart', (e) => { e.preventDefault(); publishServoCommand(moveCmd); });
    btn.addEventListener('mouseup', () => publishServoCommand(stopCmd));
    btn.addEventListener('mouseleave', () => publishServoCommand(stopCmd));
    btn.addEventListener('touchend', () => publishServoCommand(stopCmd));
}

bindServoBtn(servo1LeftBtn, 'servo1_left', 'servo1_stop');
bindServoBtn(servo1RightBtn, 'servo1_right', 'servo1_stop');
bindServoBtn(servo2LeftBtn, 'servo2_left', 'servo2_stop');
bindServoBtn(servo2RightBtn, 'servo2_right', 'servo2_stop');

if (gripperOpenBtn) {
    gripperOpenBtn.addEventListener('mousedown', () => { gripperOpenBtnActive = true; });
    gripperOpenBtn.addEventListener('touchstart', (e) => { e.preventDefault(); gripperOpenBtnActive = true; });
    gripperOpenBtn.addEventListener('mouseup', () => { gripperOpenBtnActive = false; });
    gripperOpenBtn.addEventListener('mouseleave', () => { gripperOpenBtnActive = false; });
    gripperOpenBtn.addEventListener('touchend', () => { gripperOpenBtnActive = false; });
}

if (gripperCloseBtn) {
    gripperCloseBtn.addEventListener('mousedown', () => { gripperCloseBtnActive = true; });
    gripperCloseBtn.addEventListener('touchstart', (e) => { e.preventDefault(); gripperCloseBtnActive = true; });
    gripperCloseBtn.addEventListener('mouseup', () => { gripperCloseBtnActive = false; });
    gripperCloseBtn.addEventListener('mouseleave', () => { gripperCloseBtnActive = false; });
    gripperCloseBtn.addEventListener('touchend', () => { gripperCloseBtnActive = false; });
}

// Gripper manual slider handler
const gripperSlider = document.getElementById('slider-gripper');
const gripperSliderLbl = document.getElementById('lbl-gripper-val');
if (gripperSlider) {
    gripperSlider.addEventListener('input', (e) => {
        const val = parseFloat(e.target.value);
        currentGripperTarget = val;
        if (gripperSliderLbl) {
            gripperSliderLbl.innerText = val.toFixed(2);
        }
        publishGripperTarget(val);
    });
}

// Manual Arm up/down buttons handlers
const armUpBtn = document.getElementById('btn-arm-up');
const armDownBtn = document.getElementById('btn-arm-down');

if (armUpBtn) {
    armUpBtn.addEventListener('mousedown', () => { armUpBtnActive = true; });
    armUpBtn.addEventListener('touchstart', (e) => { e.preventDefault(); armUpBtnActive = true; });
    armUpBtn.addEventListener('mouseup', () => { armUpBtnActive = false; });
    armUpBtn.addEventListener('mouseleave', () => { armUpBtnActive = false; });
    armUpBtn.addEventListener('touchend', () => { armUpBtnActive = false; });
}

if (armDownBtn) {
    armDownBtn.addEventListener('mousedown', () => { armDownBtnActive = true; });
    armDownBtn.addEventListener('touchstart', (e) => { e.preventDefault(); armDownBtnActive = true; });
    armDownBtn.addEventListener('mouseup', () => { armDownBtnActive = false; });
    armDownBtn.addEventListener('mouseleave', () => { armDownBtnActive = false; });
    armDownBtn.addEventListener('touchend', () => { armDownBtnActive = false; });
}

// ----------------------------------------------------
// 7. HUD telemetry setup
// ----------------------------------------------------
if (hudPitch) hudPitch.innerText = "0.0°";
if (hudRoll) hudRoll.innerText = "0.0°";

// ----------------------------------------------------
// 8. INITIALIZATION
// ----------------------------------------------------
connectBtn.addEventListener('click', connectROS);
autoBtn.addEventListener('click', () => setControlMode(true));
manualBtn.addEventListener('click', () => setControlMode(false));

// Load Initial Sliders Values
setControlMode(false);
lblSpeed.innerText = targetSpeed.toFixed(1);
lblLinearSens.innerText = linearSens.toFixed(1);
lblAngularSens.innerText = angularSens.toFixed(1);

// Start the gamepad and control update loop permanently
startControlLoopTimer();

// Check for gamepad connection on startup
setTimeout(() => {
    const gp = getActiveGamepad();
    if (gp) {
        gamepadConnected = true;
        showGamepadStatus(true, gp.id);
    }
}, 500);

// Toggle view callback
btnToggleView.addEventListener('click', () => {
    showing3D = !showing3D;
    if (showing3D) {
        btnToggleView.innerText = "SHOW CAM FEED";
        viewportContainer.classList.add('hidden');
        urdfViewerContainer.classList.remove('hidden');
        if (cameraViewModeSelect) cameraViewModeSelect.classList.remove('hidden');
        
        // Defer initialization to allow DOM layout to calculate dimensions
        setTimeout(() => {
            if (showing3D && connected) {
                if (!viewer3D) {
                    init3DViewer();
                } else {
                    onViewerResize();
                }
            }
        }, 150);
    } else {
        btnToggleView.innerText = "SHOW 3D VIEW";
        urdfViewerContainer.classList.add('hidden');
        viewportContainer.classList.remove('hidden');
        if (cameraViewModeSelect) cameraViewModeSelect.classList.add('hidden');
    }
});

// Camera View Mode change listener
if (cameraViewModeSelect) {
    cameraViewModeSelect.addEventListener('change', (e) => {
        currentCameraMode = e.target.value;
        console.log("Camera view mode changed to:", currentCameraMode);
        
        const controls = viewer3D ? viewer3D.cameraControls : null;
        if (controls) {
            try {
                if (currentCameraMode === 'free' || currentCameraMode === 'orbit') {
                    controls.enabled = true;
                } else if (currentCameraMode === 'chase') {
                    controls.enabled = false;
                }
                controls.update();
                console.log("Camera Controls updated. Enabled status:", controls.enabled);
            } catch (err) {
                console.error("Failed to update camera controls on mode change:", err);
            }
        } else {
            console.warn("Camera controls not initialized yet.");
        }
    });
}

let chaseTargetVector = null;

// requestAnimationFrame loop to follow rover in 3D scene
function updateCameraFollow() {
    try {
        // Update Target Marker position and visibility in 3D scene
        if (showing3D && viewer3D && targetMarker) {
            targetMarker.visible = autoMode;
            if (autoMode) {
                targetMarker.position.set(targetX, targetY, targetZ);
                // Rotate outer wireframe sphere for visual effect
                const outerWire = targetMarker.children[1];
                if (outerWire) {
                    outerWire.rotation.x += 0.01;
                    outerWire.rotation.y += 0.01;
                }
                // Pulsate scale slightly
                const time = Date.now() * 0.003;
                const scale = 1.0 + Math.sin(time) * 0.15;
                targetMarker.scale.set(scale, scale, scale);
            }
        }

        if (showing3D && viewer3D && tfClient && currentCameraMode !== 'free') {
            const tf = tfClient.getTransform('base_link');
            if (tf) {
                const rx = tf.translation.x;
                const ry = tf.translation.y;
                const rz = tf.translation.z;
                
                // Extract Heading (Yaw) from Quaternion
                const qz = tf.rotation.z !== undefined ? tf.rotation.z : 0.0;
                const qw = tf.rotation.w !== undefined ? tf.rotation.w : 1.0;
                const yaw = 2.0 * Math.atan2(qz, qw);

                const controls = viewer3D.cameraControls;
                const camera = viewer3D.camera;
                
                if (currentCameraMode === 'orbit') {
                    // Orbit Lock: Target locks onto rover center, allowing manual orbital controls
                    if (controls) {
                        if (controls.center) {
                            controls.center.set(rx, ry, rz);
                        } else if (controls.target) {
                            controls.target.set(rx, ry, rz);
                        }
                        if (controls.update) {
                            controls.update();
                        }
                    }
                } else if (currentCameraMode === 'chase') {
                    // Chase Cam: Camera locks onto back of rover (follows behind and rotates with it)
                    const distance = 25;
                    const height = 15;
                    
                    const camX = rx - Math.cos(yaw) * distance;
                    const camY = ry - Math.sin(yaw) * distance;
                    const camZ = rz + height;
                    
                    if (controls) {
                        if (controls.center) {
                            controls.center.set(rx, ry, rz);
                        } else if (controls.target) {
                            controls.target.set(rx, ry, rz);
                        }
                    }
                    
                    if (camera) {
                        camera.position.set(camX, camY, camZ);
                        if (typeof THREE !== 'undefined') {
                            if (!chaseTargetVector) {
                                chaseTargetVector = new THREE.Vector3();
                            }
                            chaseTargetVector.set(rx, ry, rz);
                            camera.lookAt(chaseTargetVector);
                        }
                    }
                }
            } else {
                // Throttle warning log to diagnostic console once in a while
                if (Math.random() < 0.005) {
                    console.warn("updateCameraFollow: No TF transform path from 'base_link' to 'world' yet. Make sure ROS is connected and publishers are active.");
                }
            }
        }
    } catch (err) {
        console.error("Error in updateCameraFollow loop: " + err.message + "\nStack: " + err.stack);
    }
    requestAnimationFrame(updateCameraFollow);
}

// Start camera follow animation loop
requestAnimationFrame(updateCameraFollow);

// Custom Local TF Client for ROS 2 (subscribes directly to /tf and /tf_static and does local math)
class LocalTFClient {
    constructor(options) {
        this.ros = options.ros;
        this.fixedFrame = options.fixedFrame || 'base_link';
        this.callbacks = {};
        this.frames = {};
        this.missingPaths = new Set();

        // Subscribe to /tf and /tf_static topics
        let tfCount = 0;
        this.tfSub = new ROSLIB.Topic({
            ros: this.ros,
            name: '/tf',
            messageType: 'tf2_msgs/msg/TFMessage'
        });
        this.tfSub.subscribe((msg) => {
            tfCount++;
            if (tfCount % 100 === 1) {
                console.log("LocalTFClient: received /tf message #" + tfCount + " with " + (msg.transforms ? msg.transforms.length : 0) + " transforms");
            }
            this.processTF(msg);
        });

        let staticCount = 0;
        this.tfStaticSub = new ROSLIB.Topic({
            ros: this.ros,
            name: '/tf_static',
            messageType: 'tf2_msgs/msg/TFMessage',
            latch: true,
            qos: {
                durability: 'transient_local',
                reliability: 'reliable',
                depth: 10
            }
        });
        this.tfStaticSub.subscribe((msg) => {
            staticCount++;
            console.log("LocalTFClient: received /tf_static message #" + staticCount + " with " + (msg.transforms ? msg.transforms.length : 0) + " transforms");
            this.processTF(msg);
        });
    }

    processTF(msg) {
        if (!msg || !msg.transforms) return;
        
        let changed = false;
        for (let i = 0; i < msg.transforms.length; i++) {
            const t = msg.transforms[i];
            const parent = t.header.frame_id;
            const child = t.child_frame_id;
            
            // Normalize names by stripping leading slash
            const parentNorm = parent.replace(/^\//, '');
            const childNorm = child.replace(/^\//, '');

            this.frames[childNorm] = {
                parent: parentNorm,
                transform: t.transform
            };
            changed = true;
        }

        if (changed) {
            this.updateAll();
        }
    }

    subscribe(frameId, callback) {
        const frameNorm = frameId.replace(/^\//, '');
        if (!this.callbacks[frameNorm]) {
            this.callbacks[frameNorm] = [];
        }
        this.callbacks[frameNorm].push(callback);

        // Instantly invoke callback if we already have the transform
        const tf = this.getTransform(frameNorm);
        if (tf) {
            callback(tf);
        }
    }

    unsubscribe(frameId, callback) {
        const frameNorm = frameId.replace(/^\//, '');
        if (this.callbacks[frameNorm]) {
            const index = this.callbacks[frameNorm].indexOf(callback);
            if (index !== -1) {
                this.callbacks[frameNorm].splice(index, 1);
            }
        }
    }

    updateAll() {
        for (const frameId in this.callbacks) {
            const tf = this.getTransform(frameId);
            if (tf) {
                const list = this.callbacks[frameId];
                for (let i = 0; i < list.length; i++) {
                    list[i](tf);
                }
            }
        }
    }

    getTransform(frameId) {
        const fixedNorm = this.fixedFrame.replace(/^\//, '');
        if (frameId === fixedNorm) {
            return {
                translation: { x: 0, y: 0, z: 0 },
                rotation: { x: 0, y: 0, z: 0, w: 1 }
            };
        }

        const translation = new THREE.Vector3(0, 0, 0);
        const rotation = new THREE.Quaternion(0, 0, 0, 1);
        
        let currentFrame = frameId;
        const visited = new Set();
        
        while (currentFrame && currentFrame !== fixedNorm) {
            if (visited.has(currentFrame)) {
                console.error("Circular TF dependency detected at: " + currentFrame);
                return null;
            }
            visited.add(currentFrame);

            const frameData = this.frames[currentFrame];
            if (!frameData) {
                return null; // Transform not yet loaded
            }
            
            const parentTranslation = new THREE.Vector3(
                frameData.transform.translation.x,
                frameData.transform.translation.y,
                frameData.transform.translation.z
            );
            const parentRotation = new THREE.Quaternion(
                frameData.transform.rotation.x,
                frameData.transform.rotation.y,
                frameData.transform.rotation.z,
                frameData.transform.rotation.w
            );
            
            translation.applyQuaternion(parentRotation);
            translation.add(parentTranslation);
            rotation.premultiply(parentRotation);
            
            currentFrame = frameData.parent;
        }
        
        if (currentFrame !== fixedNorm) {
            const pathKey = frameId + "->" + fixedNorm;
            if (!this.missingPaths.has(pathKey)) {
                this.missingPaths.add(pathKey);
                console.warn("getTransform: No transform path from " + frameId + " to " + fixedNorm + ". (Unresolved parent: " + currentFrame + ")");
            }
            return null; // Frame has no path back to fixedFrame
        }
        
        return {
            translation: { x: translation.x, y: translation.y, z: translation.z },
            rotation: { x: rotation.x, y: rotation.y, z: rotation.z, w: rotation.w }
        };
    }

    dispose() {
        if (this.tfSub) this.tfSub.unsubscribe();
        if (this.tfStaticSub) this.tfStaticSub.unsubscribe();
        this.callbacks = {};
        this.frames = {};
    }
}

// 3D Viewer initialization and destruction functions
function init3DViewer() {
    if (!showing3D) return;
    if (!connected || !ros) {
        console.warn('init3DViewer: Not connected to ROS.');
        return;
    }
    
    try {
        const viewerDiv = document.getElementById('urdf-viewer');
        const width = viewerDiv.clientWidth || 500;
        const height = viewerDiv.clientHeight || 350;

        console.log("init3DViewer: clientWidth =", width, "clientHeight =", height);

        viewer3D = new ROS3D.Viewer({
            divID: 'urdf-viewer',
            width: width,
            height: height,
            antialias: true,
            background: '#0a0e17'
        });

        // Add a grid
        viewer3D.addObject(new ROS3D.Grid({
            color: '#00e5ff',
            cellSize: 10.0,
            numCells: 200
        }));

        // Add holographic target marker for IK Mode
        if (typeof THREE !== 'undefined') {
            targetMarker = new THREE.Group();
            
            // Core mint-green sphere
            const coreGeom = new THREE.SphereGeometry(0.3, 16, 16);
            const coreMat = new THREE.MeshBasicMaterial({
                color: 0x00ff88,
                transparent: true,
                opacity: 0.9
            });
            const coreMesh = new THREE.Mesh(coreGeom, coreMat);
            targetMarker.add(coreMesh);
            
            // Outer sci-fi wireframe sphere
            const wireGeom = new THREE.SphereGeometry(0.6, 8, 8);
            const wireMat = new THREE.MeshBasicMaterial({
                color: 0x00ff88,
                wireframe: true,
                transparent: true,
                opacity: 0.4
            });
            const wireMesh = new THREE.Mesh(wireGeom, wireMat);
            targetMarker.add(wireMesh);
            
            targetMarker.visible = autoMode;
            viewer3D.scene.add(targetMarker);
        }

        // Setup Local TF Client to track links relative to world
        tfClient = new LocalTFClient({
            ros: ros,
            fixedFrame: 'world'
        });

        // Setup URDF Client dynamically if publisher is already active
        if (robotStatePublisherRunning) {
            loadURDFClient();
        }

        console.log("Direct param fetch test starting...");
        const robotDescParam = new ROSLIB.Param({
            ros: ros,
            name: '/robot_state_publisher:robot_description'
        });
        robotDescParam.get((val) => {
            console.log("Direct param fetched. Type:", typeof val, "Length:", val ? val.length : 0);
            if (val) {
                console.log("URDF snippet:", val.substring(0, 150));
            } else {
                console.warn("Direct param fetched value is empty or null!");
            }
        });

        console.log('3D WebGL Viewer successfully initialized.');

        // Resize callback
        window.addEventListener('resize', onViewerResize);
    } catch (err) {
        console.error('Failed to initialize 3D Viewer: ' + err.message + '\nStack: ' + err.stack);
        // Print error directly to screen for diagnosis
        const container = document.getElementById('urdf-viewer') || document.body;
        const errDiv = document.createElement('div');
        errDiv.style.cssText = 'color: #ffaa00; font-family: monospace; font-size: 0.85rem; padding: 15px; background: rgba(15,0,0,0.95); border-left: 3px solid #ffaa00; margin: 10px; z-index: 9999; position: relative; width: calc(100% - 20px); text-align: left;';
        errDiv.innerText = '⚠️ 3D VIEWER ERROR: ' + err.message + '\n' + err.stack;
        container.appendChild(errDiv);
    }
}

function onViewerResize() {
    if (viewer3D && viewer3D.resize) {
        const viewerDiv = document.getElementById('urdf-viewer');
        const w = viewerDiv.clientWidth;
        const h = viewerDiv.clientHeight;
        console.log("onViewerResize: clientWidth =", w, "clientHeight =", h);
        viewer3D.resize(w, h);
    }
}

function loadURDFClient() {
    if (!connected || !ros || !viewer3D || !tfClient) return;
    if (urdfClient) return; // already loaded

    console.log("loadURDFClient: Creating ROS3D.UrdfClient...");
    urdfClient = new ROS3D.UrdfClient({
        ros: ros,
        tfClient: tfClient,
        rootObject: viewer3D.scene,
        param: 'robot_description'
    });
}

function cleanupURDFClient() {
    if (urdfClient) {
        console.log("cleanupURDFClient: Clearing URDF model...");
        if (viewer3D && viewer3D.scene) {
            // Remove URDF children (meshes) from scene
            const toRemove = [];
            viewer3D.scene.children.forEach(child => {
                // Keep grid and lights
                if (child.type !== "Grid" && child.type !== "AmbientLight" && child.type !== "DirectionalLight") {
                    toRemove.push(child);
                }
            });
            toRemove.forEach(child => viewer3D.scene.remove(child));
        }
        urdfClient = null;
    }
}

function cleanup3DViewer() {
    window.removeEventListener('resize', onViewerResize);
    cleanupURDFClient();
    if (viewer3D) {
        if (tfClient && tfClient.dispose) {
            tfClient.dispose();
        }
        viewer3D = null;
        tfClient = null;
        targetMarker = null;
        document.getElementById('urdf-viewer').innerHTML = '';
        console.log('3D WebGL Viewer cleaned up.');
    }
}

// ----------------------------------------------------
// 8. PROCESS MANAGER HANDLERS
// ----------------------------------------------------
let modulesInitialized = false;

function updateModuleStatusUI(status) {
    const drawer = document.getElementById('modules-drawer');
    if (!drawer) return;

    const currentKeys = Object.keys(status).join(',');
    if (!modulesInitialized || drawer.getAttribute('data-keys') !== currentKeys) {
        drawer.innerHTML = '';
        for (const key in status) {
            const info = status[key];
            const row = document.createElement('div');
            row.className = 'module-row';
            row.innerHTML = `
                <div class="module-info">
                    <span class="module-status-dot stopped" id="status-dot-${key}"></span>
                    <span class="module-name">${info.name}</span>
                </div>
                <button class="module-toggle-btn neon-btn-blue" id="btn-toggle-${key}" data-module="${key}">START</button>
            `;
            drawer.appendChild(row);
        }
        drawer.setAttribute('data-keys', currentKeys);
        modulesInitialized = true;
    }

    // update statuses and buttons
    for (const key in status) {
        const info = status[key];
        const isRunning = info.running;
        const dot = document.getElementById(`status-dot-${key}`);
        const btn = document.getElementById(`btn-toggle-${key}`);
        if (dot) {
            if (isRunning) {
                dot.className = "module-status-dot running";
            } else {
                dot.className = "module-status-dot stopped";
            }
        }
        if (btn) {
            if (isRunning) {
                btn.innerText = "STOP";
                btn.className = "module-toggle-btn neon-btn-red";
            } else {
                btn.innerText = "START";
                btn.className = "module-toggle-btn neon-btn-blue";
            }
        }

        // update corresponding camera card UI widgets if these are the stream modules
        if (key === 'stream_cam_0') {
            const cam0Dot = document.getElementById('status-cam-0-dot');
            const cam0Text = document.getElementById('status-cam-0-text');
            const cam0Btn = document.getElementById('btn-toggle-cam-0');
            if (cam0Dot) cam0Dot.className = isRunning ? "status-indicator connected" : "status-indicator disconnected";
            if (cam0Text) cam0Text.innerText = isRunning ? "ACTIVE" : "OFFLINE";
            if (cam0Btn) {
                cam0Btn.innerText = isRunning ? "STOP STREAM" : "START STREAM";
                cam0Btn.className = isRunning ? "neon-btn-red" : "neon-btn-blue";
            }
        }
        if (key === 'stream_cam_1') {
            const cam1Dot = document.getElementById('status-cam-1-dot');
            const cam1Text = document.getElementById('status-cam-1-text');
            const cam1Btn = document.getElementById('btn-toggle-cam-1');
            if (cam1Dot) cam1Dot.className = isRunning ? "status-indicator connected" : "status-indicator disconnected";
            if (cam1Text) cam1Text.innerText = isRunning ? "ACTIVE" : "OFFLINE";
            if (cam1Btn) {
                cam1Btn.innerText = isRunning ? "STOP STREAM" : "START STREAM";
                cam1Btn.className = isRunning ? "neon-btn-red" : "neon-btn-blue";
            }
        }
        if (key === 'stream_cam_2') {
            const cam2Dot = document.getElementById('status-cam-2-dot');
            const cam2Text = document.getElementById('status-cam-2-text');
            const cam2Btn = document.getElementById('btn-toggle-cam-2');
            if (cam2Dot) cam2Dot.className = isRunning ? "status-indicator connected" : "status-indicator disconnected";
            if (cam2Text) cam2Text.innerText = isRunning ? "ACTIVE" : "OFFLINE";
            if (cam2Btn) {
                cam2Btn.innerText = isRunning ? "STOP STREAM" : "START STREAM";
                cam2Btn.className = isRunning ? "neon-btn-red" : "neon-btn-blue";
            }
        }
    }
}

function resetModuleStatusUI() {
    modulesInitialized = false;
    const drawer = document.getElementById('modules-drawer');
    if (drawer) {
        const dots = drawer.querySelectorAll('.module-status-dot');
        const btns = drawer.querySelectorAll('.module-toggle-btn');
        dots.forEach(dot => { dot.className = "module-status-dot stopped"; });
        btns.forEach(btn => {
            btn.innerText = "START";
            btn.className = "module-toggle-btn neon-btn-blue";
            btn.disabled = false;
        });
    }
    // reset camera card indicators too
    const cam0Dot = document.getElementById('status-cam-0-dot');
    const cam0Text = document.getElementById('status-cam-0-text');
    const cam0Btn = document.getElementById('btn-toggle-cam-0');
    if (cam0Dot) cam0Dot.className = "status-indicator disconnected";
    if (cam0Text) cam0Text.innerText = "OFFLINE";
    if (cam0Btn) {
        cam0Btn.innerText = "START STREAM";
        cam0Btn.className = "neon-btn-blue";
        cam0Btn.disabled = false;
    }
    const cam1Dot = document.getElementById('status-cam-1-dot');
    const cam1Text = document.getElementById('status-cam-1-text');
    const cam1Btn = document.getElementById('btn-toggle-cam-1');
    if (cam1Dot) cam1Dot.className = "status-indicator disconnected";
    if (cam1Text) cam1Text.innerText = "OFFLINE";
    if (cam1Btn) {
        cam1Btn.innerText = "START STREAM";
        cam1Btn.className = "neon-btn-blue";
        cam1Btn.disabled = false;
    }
}

function toggleModule(key) {
    if (!connected || !ros) {
        console.warn("Cannot toggle module: Not connected to ROS.");
        return;
    }
    
    // support toggling both process manager list buttons and specific viewport buttons
    const btn = document.getElementById(`btn-toggle-${key}`);
    const camBtn = (key === 'stream_cam_0') ? document.getElementById('btn-toggle-cam-0') : 
                   (key === 'stream_cam_1') ? document.getElementById('btn-toggle-cam-1') : 
                   (key === 'stream_cam_2') ? document.getElementById('btn-toggle-cam-2') : null;

    const currentText = btn ? btn.innerText : (camBtn ? camBtn.innerText : "");
    const shouldStart = currentText.includes("START");

    if (btn) {
        btn.innerText = shouldStart ? "STARTING..." : "STOPPING...";
        btn.disabled = true;
    }
    if (camBtn) {
        camBtn.innerText = shouldStart ? "STARTING..." : "STOPPING...";
        camBtn.disabled = true;
    }
    
    const service = new ROSLIB.Service({
        ros: ros,
        name: `/process_manager/toggle_${key}`,
        serviceType: 'std_srvs/srv/SetBool'
    });
    
    const request = new ROSLIB.ServiceRequest({
        data: shouldStart
    });
    
    service.callService(request, (result) => {
        if (btn) btn.disabled = false;
        if (camBtn) camBtn.disabled = false;
        if (result && result.success) {
            console.log(`Successfully toggled ${key}: ${result.message}`);
        } else {
            console.error(`Failed to toggle ${key}: ${result ? result.message : 'Unknown error'}`);
            if (btn) btn.innerText = shouldStart ? "START" : "STOP";
            if (camBtn) camBtn.innerText = shouldStart ? "START STREAM" : "STOP STREAM";
        }
    }, (error) => {
        if (btn) btn.disabled = false;
        if (camBtn) camBtn.disabled = false;
        console.error(`Service call error for ${key}:`, error);
        if (btn) btn.innerText = shouldStart ? "START" : "STOP";
        if (camBtn) camBtn.innerText = shouldStart ? "START STREAM" : "STOP STREAM";
    });
}

// collapsible panel event listener
btnToggleModules.addEventListener('click', () => {
    modulesDrawer.classList.toggle('collapsed');
    modulesCollapseIcon.classList.toggle('collapsed');
});

// click delegation for toggle buttons
document.body.addEventListener('click', (e) => {
    const btn = e.target.closest('.module-toggle-btn');
    if (btn) {
        const key = btn.getAttribute('data-module');
        if (key) {
            toggleModule(key);
        }
    }
});

// click listeners for main panel camera toggle buttons
const cam0Toggle = document.getElementById('btn-toggle-cam-0');
if (cam0Toggle) {
    cam0Toggle.addEventListener('click', () => {
        toggleModule('stream_cam_0');
    });
}
const cam1Toggle = document.getElementById('btn-toggle-cam-1');
if (cam1Toggle) {
    cam1Toggle.addEventListener('click', () => {
        toggleModule('stream_cam_1');
    });
}
const cam2Toggle = document.getElementById('btn-toggle-cam-2');
if (cam2Toggle) {
    cam2Toggle.addEventListener('click', () => {
        toggleModule('stream_cam_2');
    });
}

// XLR / Serial UI interactions
const btnSerialConverse = document.getElementById('btn-serial-converse');
const btnSerialTransmit = document.getElementById('btn-serial-transmit');
const inputSerialOutgoing = document.getElementById('serial-outgoing-input');
const preSerialConsole = document.getElementById('serial-console-log');

function callSerialService(serviceName, serviceType, requestData, callback, errCallback) {
    if (!connected || !ros) {
        if (errCallback) errCallback("Not connected to ROS.");
        else if (preSerialConsole) preSerialConsole.innerText = "Error: Not connected to ROS.";
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: serviceName,
        serviceType: serviceType
    });
    const request = new ROSLIB.ServiceRequest(requestData);
    service.callService(request, callback, (error) => {
        if (errCallback) errCallback(error);
        else if (preSerialConsole) preSerialConsole.innerText = "Service Error: " + error;
    });
}

if (btnSerialConverse && inputSerialOutgoing && preSerialConsole) {
    btnSerialConverse.addEventListener('click', () => {
        const outStr = inputSerialOutgoing.value;
        preSerialConsole.innerText = `[Sending Converse]: ${outStr}\n[Awaiting reply...]`;
        callSerialService('/Converse', 'serial_interfaces/srv/Converse', { outgoing: outStr }, (result) => {
            preSerialConsole.innerText = `[Received]: ${result.incoming}`;
        });
    });
}

if (btnSerialTransmit && inputSerialOutgoing && preSerialConsole) {
    btnSerialTransmit.addEventListener('click', () => {
        const outStr = inputSerialOutgoing.value;
        preSerialConsole.innerText = `[Transmitting]: ${outStr}`;
        callSerialService('/Transmit', 'serial_interfaces/srv/Transmit', { outgoing: outStr }, (result) => {
            preSerialConsole.innerText += `\n[Transmit OK]`;
        });
    });
}


// click listener to collapse/expand diagnostics log panel
const diagConsole = document.getElementById('diagnostic-console');
const diagHeader = document.getElementById('diag-header');
const diagCollapseBtn = document.getElementById('diag-collapse-btn');

if (diagHeader && diagConsole && diagCollapseBtn) {
    diagHeader.addEventListener('click', () => {
        const isCollapsed = diagConsole.style.height === '24px';
        if (isCollapsed) {
            diagConsole.style.height = '120px';
            diagCollapseBtn.innerText = '[ HIDE ]';
        } else {
            diagConsole.style.height = '24px';
            diagCollapseBtn.innerText = '[ SHOW ]';
        }
    });
}

// ====================================================
// 9. GPS & LOCATION NAVIGATION SYSTEM
// ====================================================

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

let gpsMapMode = 'radar';
let leafletMap = null;
let leafletRoverMarker = null;
let leafletTargetMarker = null;
let leafletPolyline = null;

let nmeaAutoScroll = true;

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
    if (lat === null || lon === null) return { latitude: 0.0, longitude: 0.0 };
    const R = 6371000;
    const newLat = lat + (dyMeters / R) * (180 / Math.PI);
    const newLon = lon + (dxMeters / (R * Math.cos(lat * Math.PI / 180))) * (180 / Math.PI);
    return { latitude: newLat, longitude: newLon };
}

// 1. Setup GPS ROS Topics
function setupGPSROSInterfaces() {
    if (!ros) return;

    gpsFixSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gps/fix',
        messageType: 'sensor_msgs/msg/NavSatFix'
    });
    gpsFixSub.subscribe((msg) => {
        if (msg && msg.latitude !== undefined && msg.longitude !== undefined) {
            gpsCurrentFix.latitude = msg.latitude;
            gpsCurrentFix.longitude = msg.longitude;
            gpsCurrentFix.altitude = msg.altitude || 0.0;
            gpsCurrentFix.status = (msg.status && msg.status.status !== undefined) ? msg.status.status : 0;
            gpsCurrentFix.lastUpdate = Date.now();

            if (!gpsHomeOrigin.isSet && gpsCurrentFix.status >= 0) {
                gpsHomeOrigin.latitude = msg.latitude;
                gpsHomeOrigin.longitude = msg.longitude;
                gpsHomeOrigin.altitude = msg.altitude || 0.0;
                gpsHomeOrigin.isSet = true;
            }

            if (gpsCurrentFix.status >= 0) {
                recordRoverPathPoint(msg.latitude, msg.longitude, null, null);
            }

            updateGPSUI();
        }
    });

    gpsNmeaSub = new ROSLIB.Topic({
        ros: ros,
        name: '/gps/nmea_raw',
        messageType: 'std_msgs/msg/String'
    });
    gpsNmeaSub.subscribe((msg) => {
        if (msg && msg.data) {
            appendNmeaLog(msg.data);
        }
    });

    gpsTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/gps/set_target',
        messageType: 'geometry_msgs/msg/Point'
    });
}

function cleanupGPSROSInterfaces() {
    if (gpsFixSub) { gpsFixSub.unsubscribe(); gpsFixSub = null; }
    if (gpsNmeaSub) { gpsNmeaSub.unsubscribe(); gpsNmeaSub = null; }
    gpsTargetPub = null;
}

// 2. UI Update Function
function updateGPSUI() {
    const fixDot = document.getElementById('gps-fix-status-dot');
    const fixText = document.getElementById('gps-fix-status-text');
    const isFixed = gpsCurrentFix.status >= 0 && gpsCurrentFix.latitude !== null;

    if (fixDot) {
        fixDot.className = isFixed ? "status-indicator connected" : "status-indicator disconnected";
    }
    if (fixText) {
        fixText.innerText = isFixed ? "3D FIX (GNSS)" : "NO FIX";
        fixText.style.color = isFixed ? "var(--accent-green)" : "var(--accent-red)";
    }

    // Telemetry Cards (in SYSTEM TELEMETRY)
    const teleCoords = document.getElementById('val-gps-telemetry-coords');
    const teleFix = document.getElementById('val-gps-telemetry-fix');
    if (teleCoords) {
        teleCoords.innerText = isFixed ? `${gpsCurrentFix.latitude.toFixed(6)}°, ${gpsCurrentFix.longitude.toFixed(6)}°` : "Lat: --, Lon: --";
    }
    if (teleFix) {
        teleFix.innerText = isFixed ? `FIX 3D (${gpsCurrentFix.altitude.toFixed(1)} m)` : "NO FIX (-- m)";
    }

    // Detailed Location Cards
    const latDet = document.getElementById('val-gps-lat-det');
    const lonDet = document.getElementById('val-gps-lon-det');
    const altDet = document.getElementById('val-gps-alt-det');
    const qualityDet = document.getElementById('val-gps-quality');
    const distDet = document.getElementById('val-gps-dist');
    const bearingDet = document.getElementById('val-gps-bearing');
    const roverCoordsShort = document.getElementById('map-rover-coords-short');

    if (latDet) latDet.innerText = isFixed ? `${gpsCurrentFix.latitude.toFixed(6)}°` : "--";
    if (lonDet) lonDet.innerText = isFixed ? `${gpsCurrentFix.longitude.toFixed(6)}°` : "--";
    if (altDet) altDet.innerText = isFixed ? `${gpsCurrentFix.altitude.toFixed(1)} m` : "-- m";
    if (roverCoordsShort) roverCoordsShort.innerText = isFixed ? `${gpsCurrentFix.latitude.toFixed(5)}, ${gpsCurrentFix.longitude.toFixed(5)}` : "NO FIX";

    if (qualityDet) {
        if (isFixed) {
            qualityDet.innerText = "3D FIX (GOOD)";
            qualityDet.style.color = "var(--accent-green)";
        } else {
            qualityDet.innerText = "SEARCHING...";
            qualityDet.style.color = "var(--accent-amber)";
        }
    }

    // Distance and bearing calculations if target set
    const mapTargetCoords = document.getElementById('map-target-coords');
    const mapTargetDistBearing = document.getElementById('map-target-dist-bearing');

    if (isFixed && gpsTargetWaypoint.isSet && gpsTargetWaypoint.latitude !== null && gpsTargetWaypoint.longitude !== null) {
        const dist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const bearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const cardDir = getCompassDirection(bearing);

        if (distDet) distDet.innerText = `${dist.toFixed(1)} m`;
        if (bearingDet) bearingDet.innerText = `${bearing.toFixed(1)}° (${cardDir})`;
        if (mapTargetCoords) mapTargetCoords.innerText = `Lat: ${gpsTargetWaypoint.latitude.toFixed(6)} | Lon: ${gpsTargetWaypoint.longitude.toFixed(6)}`;
        if (mapTargetDistBearing) mapTargetDistBearing.innerText = `Dist: ${dist.toFixed(1)} m | Bear: ${bearing.toFixed(1)}° (${cardDir})`;
    } else {
        if (distDet) distDet.innerText = "-- m";
        if (bearingDet) bearingDet.innerText = "--°";
        if (mapTargetCoords) mapTargetCoords.innerText = "Lat: -- | Lon: --";
        if (mapTargetDistBearing) mapTargetDistBearing.innerText = "Dist: -- m | Bear: --°";
    }

    // Render maps
    if (gpsMapMode === 'radar') {
        drawTacticalRadar();
    } else if (gpsMapMode === 'map' && leafletMap) {
        updateLeafletMap();
    }
}

// 3. Raw NMEA Inspector logger
function appendNmeaLog(line) {
    const logBox = document.getElementById('nmea-stream-log');
    if (!logBox) return;

    if (logBox.children.length === 1 && logBox.children[0].innerText.includes('Waiting for Serial Input')) {
        logBox.innerHTML = '';
    }

    const timeStr = new Date().toLocaleTimeString();
    const entry = document.createElement('div');
    entry.style.borderBottom = '1px solid rgba(255,255,255,0.03)';
    entry.style.padding = '2px 0';

    if (line.includes('GGA')) {
        entry.style.color = 'var(--accent-blue)';
    } else if (line.includes('RMC')) {
        entry.style.color = 'var(--accent-green)';
    } else {
        entry.style.color = 'var(--text-muted)';
    }

    entry.innerText = `[${timeStr}] ${line}`;
    logBox.appendChild(entry);

    while (logBox.children.length > 50) {
        logBox.removeChild(logBox.firstChild);
    }

    if (nmeaAutoScroll) {
        logBox.scrollTop = logBox.scrollHeight;
    }
}

function recordRoverPathPoint(lat, lon, x, y) {
    if (lat === null || lon === null || isNaN(lat) || isNaN(lon)) return;
    const now = Date.now();
    if (roverPathHistory.length > 0) {
        const last = roverPathHistory[roverPathHistory.length - 1];
        const dist = calculateDistanceMeters(last.lat, last.lon, lat, lon);
        if (dist < 0.2) return; // Only add point if moved at least 0.2 meters
    }
    roverPathHistory.push({ lat: lat, lon: lon, x: x || 0, y: y || 0, timestamp: now });
    if (roverPathHistory.length > 500) {
        roverPathHistory.shift(); // Keep max 500 points
    }
}

// 4. Tactical Radar Canvas Renderer (Clean static grid with trajectory path line)
function drawTacticalRadar() {
    const canvas = document.getElementById('gps-radar-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const rect = canvas.getBoundingClientRect();
    if (canvas.width !== rect.width || canvas.height !== rect.height) {
        canvas.width = rect.width;
        canvas.height = rect.height;
    }

    const w = canvas.width;
    const h = canvas.height;
    const cx = w / 2;
    const cy = h / 2;
    const maxRadius = Math.min(w, h) / 2 - 25;

    ctx.fillStyle = '#050811';
    ctx.fillRect(0, 0, w, h);

    const rings = [0.25, 0.5, 0.75, 1.0];
    const ringDistances = [5, 10, 20, 50];
    ctx.strokeStyle = 'rgba(0, 229, 255, 0.15)';
    ctx.lineWidth = 1;

    rings.forEach((rRatio, idx) => {
        const r = maxRadius * rRatio;
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, 2 * Math.PI);
        ctx.stroke();

        ctx.fillStyle = 'rgba(0, 229, 255, 0.4)';
        ctx.font = '10px monospace';
        ctx.fillText(`${ringDistances[idx]}m`, cx + r + 4, cy - 4);
    });

    ctx.beginPath();
    ctx.moveTo(cx - maxRadius, cy);
    ctx.lineTo(cx + maxRadius, cy);
    ctx.moveTo(cx, cy - maxRadius);
    ctx.lineTo(cx, cy + maxRadius);
    ctx.stroke();

    ctx.fillStyle = '#00e5ff';
    ctx.font = 'bold 12px monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText('N', cx, cy - maxRadius - 12);
    ctx.fillText('S', cx, cy + maxRadius + 12);
    ctx.fillText('E', cx + maxRadius + 12, cy);
    ctx.fillText('W', cx - maxRadius - 12, cy);

    // Draw Rover Trajectory Path Line (if history exists)
    if (roverPathHistory.length > 1 && gpsCurrentFix.latitude !== null && gpsCurrentFix.longitude !== null) {
        ctx.save();
        ctx.strokeStyle = '#00e5ff';
        ctx.lineWidth = 2.5;
        ctx.shadowColor = '#00e5ff';
        ctx.shadowBlur = 8;
        ctx.beginPath();

        let firstPoint = true;
        for (let i = 0; i < roverPathHistory.length; i++) {
            const pt = roverPathHistory[i];
            const dist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, pt.lat, pt.lon);
            if (dist > 100) continue;

            const bearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, pt.lat, pt.lon);
            const rad = (bearing - 90) * Math.PI / 180;
            const px = cx + (dist / 50) * maxRadius * Math.cos(rad);
            const py = cy + (dist / 50) * maxRadius * Math.sin(rad);

            if (firstPoint) {
                ctx.moveTo(px, py);
                firstPoint = false;
            } else {
                ctx.lineTo(px, py);
            }
        }
        ctx.stroke();
        ctx.shadowBlur = 0;

        // Draw neon green breadcrumb dots along historical path
        for (let i = 0; i < roverPathHistory.length; i += 3) {
            const pt = roverPathHistory[i];
            const dist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, pt.lat, pt.lon);
            if (dist > 100) continue;
            const bearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, pt.lat, pt.lon);
            const rad = (bearing - 90) * Math.PI / 180;
            const px = cx + (dist / 50) * maxRadius * Math.cos(rad);
            const py = cy + (dist / 50) * maxRadius * Math.sin(rad);

            ctx.fillStyle = 'rgba(57, 255, 20, 0.75)';
            ctx.beginPath();
            ctx.arc(px, py, 2.5, 0, 2 * Math.PI);
            ctx.fill();
        }
        ctx.restore();
    }

    // Draw Home Origin (if set and fix available)
    if (gpsCurrentFix.latitude !== null && gpsHomeOrigin.isSet && gpsHomeOrigin.latitude !== null) {
        const homeDist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsHomeOrigin.latitude, gpsHomeOrigin.longitude);
        if (homeDist < 100) {
            const homeBearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsHomeOrigin.latitude, gpsHomeOrigin.longitude);
            const homeRad = (homeBearing - 90) * Math.PI / 180;
            const px = cx + (homeDist / 50) * maxRadius * Math.cos(homeRad);
            const py = cy + (homeDist / 50) * maxRadius * Math.sin(homeRad);

            ctx.fillStyle = '#ffaa00';
            ctx.beginPath();
            ctx.arc(px, py, 5, 0, 2 * Math.PI);
            ctx.fill();
            ctx.font = '10px monospace';
            ctx.fillText('HOME', px, py - 8);
        }
    }

    // Draw Target Waypoint (if set)
    if (gpsCurrentFix.latitude !== null && gpsTargetWaypoint.isSet && gpsTargetWaypoint.latitude !== null && gpsTargetWaypoint.longitude !== null) {
        const dist = calculateDistanceMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const bearing = calculateBearingDegrees(gpsCurrentFix.latitude, gpsCurrentFix.longitude, gpsTargetWaypoint.latitude, gpsTargetWaypoint.longitude);
        const rad = (bearing - 90) * Math.PI / 180;

        const clampedDist = Math.min(dist, 50);
        const tx = cx + (clampedDist / 50) * maxRadius * Math.cos(rad);
        const ty = cy + (clampedDist / 50) * maxRadius * Math.sin(rad);

        ctx.strokeStyle = '#ffaa00';
        ctx.setLineDash([4, 4]);
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.lineTo(tx, ty);
        ctx.stroke();
        ctx.setLineDash([]);

        ctx.fillStyle = '#ffaa00';
        ctx.beginPath();
        ctx.arc(tx, ty, 7, 0, 2 * Math.PI);
        ctx.fill();

        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 1.5;
        ctx.stroke();

        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 10px monospace';
        ctx.fillText(`TARGET (${dist.toFixed(1)}m)`, tx, ty - 12);
    }

    // Rover Icon (Center of Radar) with Heading Pointer (Priority: Raw /imu/euler -> Filtered /imu/filtered_euler -> TF Base Yaw)
    let headingRad = 0;
    const isImuRaw = imuEulerData.lastUpdate !== null && (Date.now() - imuEulerData.lastUpdate < 5000);
    const isImuFiltered = imuFilteredEulerData.lastUpdate !== null && (Date.now() - imuFilteredEulerData.lastUpdate < 5000);

    if (isImuRaw) {
        headingRad = -(imuEulerData.yaw * Math.PI / 180.0);
    } else if (isImuFiltered) {
        headingRad = -(imuFilteredEulerData.yaw * Math.PI / 180.0);
    } else if (typeof valBaseYaw !== 'undefined' && valBaseYaw) {
        const yawText = valBaseYaw.innerText || '0';
        const match = yawText.match(/(-?\d+\.?\d*)\s*rad/);
        if (match) {
            headingRad = -parseFloat(match[1]);
        }
    }

    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate(headingRad);
    ctx.fillStyle = '#39ff14';
    ctx.beginPath();
    ctx.moveTo(0, -12);
    ctx.lineTo(8, 10);
    ctx.lineTo(0, 6);
    ctx.lineTo(-8, 10);
    ctx.closePath();
    ctx.fill();
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 1;
    ctx.stroke();
    ctx.restore();
}

// 5. Leaflet Map
function initLeafletMap() {
    const mapDiv = document.getElementById('leaflet-map');
    if (!mapDiv || typeof L === 'undefined') return;

    if (leafletMap) {
        leafletMap.invalidateSize();
        return;
    }

    const initLat = gpsCurrentFix.latitude || 37.774929;
    const initLon = gpsCurrentFix.longitude || -122.419416;

    try {
        leafletMap = L.map('leaflet-map', {
            zoomControl: true,
            attributionControl: false
        }).setView([initLat, initLon], 18);

        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            maxZoom: 19
        }).addTo(leafletMap);

        const roverIcon = L.divIcon({
            className: 'custom-rover-marker',
            html: '<div class="radar-pulse-dot"></div>',
            iconSize: [20, 20],
            iconAnchor: [10, 10]
        });
        leafletRoverMarker = L.marker([initLat, initLon], { icon: roverIcon })
            .addTo(leafletMap)
            .bindPopup('<b>D.A.V.E. Rover</b><br>GNSS Location');

        leafletMap.on('click', (e) => {
            if (gpsCurrentFix.latitude !== null) {
                setTargetWaypoint(e.latlng.lat, e.latlng.lng);
            }
        });

    } catch (err) {
        console.error("Failed to initialize Leaflet Map:", err);
    }
}

function updateLeafletMap() {
    if (!leafletMap || typeof L === 'undefined') return;

    const lat = gpsCurrentFix.latitude;
    const lon = gpsCurrentFix.longitude;

    if (lat !== null && lon !== null && leafletRoverMarker) {
        leafletRoverMarker.setLatLng([lat, lon]);
    }

    if (gpsTargetWaypoint.isSet && gpsTargetWaypoint.latitude !== null && gpsTargetWaypoint.longitude !== null) {
        const targetLat = gpsTargetWaypoint.latitude;
        const targetLon = gpsTargetWaypoint.longitude;

        if (!leafletTargetMarker) {
            leafletTargetMarker = L.marker([targetLat, targetLon]).addTo(leafletMap)
                .bindPopup('<b>Target Waypoint</b>');
        } else {
            leafletTargetMarker.setLatLng([targetLat, targetLon]);
        }

        if (lat !== null && lon !== null) {
            if (!leafletPolyline) {
                leafletPolyline = L.polyline([[lat, lon], [targetLat, targetLon]], { color: '#ffaa00', weight: 3, dashArray: '6, 6' }).addTo(leafletMap);
            } else {
                leafletPolyline.setLatLngs([[lat, lon], [targetLat, targetLon]]);
            }
        }
    } else {
        if (leafletTargetMarker) {
            leafletMap.removeLayer(leafletTargetMarker);
            leafletTargetMarker = null;
        }
        if (leafletPolyline) {
            leafletMap.removeLayer(leafletPolyline);
            leafletPolyline = null;
        }
    }
}

// 6. Waypoint & Origin Actions
function setTargetWaypoint(lat, lon) {
    gpsTargetWaypoint.latitude = lat;
    gpsTargetWaypoint.longitude = lon;
    gpsTargetWaypoint.isSet = true;

    if (gpsHomeOrigin.isSet && gpsHomeOrigin.latitude !== null) {
        const dx = calculateDistanceMeters(gpsHomeOrigin.latitude, gpsHomeOrigin.longitude, gpsHomeOrigin.latitude, lon);
        const dy = calculateDistanceMeters(gpsHomeOrigin.latitude, gpsHomeOrigin.longitude, lat, gpsHomeOrigin.longitude);
        const signX = lon >= gpsHomeOrigin.longitude ? 1 : -1;
        const signY = lat >= gpsHomeOrigin.latitude ? 1 : -1;
        gpsTargetWaypoint.x = dx * signX;
        gpsTargetWaypoint.y = dy * signY;
    }

    if (connected && gpsTargetPub) {
        const msg = new ROSLIB.Message({
            x: gpsTargetWaypoint.x || 0.0,
            y: gpsTargetWaypoint.y || 0.0,
            z: 0.0
        });
        gpsTargetPub.publish(msg);
    }

    updateGPSUI();
}

function clearTargetWaypoint() {
    gpsTargetWaypoint.latitude = null;
    gpsTargetWaypoint.longitude = null;
    gpsTargetWaypoint.isSet = false;
    updateGPSUI();
}

function setHomeOrigin() {
    if (gpsCurrentFix.latitude !== null) {
        gpsHomeOrigin.latitude = gpsCurrentFix.latitude;
        gpsHomeOrigin.longitude = gpsCurrentFix.longitude;
        gpsHomeOrigin.altitude = gpsCurrentFix.altitude;
        gpsHomeOrigin.isSet = true;
        console.log(`GPS Home Origin set to ${gpsHomeOrigin.latitude}, ${gpsHomeOrigin.longitude}`);
        updateGPSUI();
    }
}

// 7. Event Listeners for Location Navigation Section
document.addEventListener('DOMContentLoaded', () => {
    const selectMapMode = document.getElementById('gps-map-mode');
    const btnSetHome = document.getElementById('btn-set-home-origin');
    const btnSetTarget = document.getElementById('btn-set-target-waypoint');
    const btnCenter = document.getElementById('btn-center-rover');
    const btnClearTarget = document.getElementById('btn-clear-waypoint');
    const btnAutoscroll = document.getElementById('btn-toggle-nmea-autoscroll');
    const btnClearNmea = document.getElementById('btn-clear-nmea');
    const radarCanvas = document.getElementById('gps-radar-canvas');

    if (selectMapMode) {
        selectMapMode.addEventListener('change', (e) => {
            gpsMapMode = e.target.value;
            const radarCanvas = document.getElementById('gps-radar-canvas');
            const leafletDiv = document.getElementById('leaflet-map');

            if (gpsMapMode === 'radar') {
                if (radarCanvas) radarCanvas.style.display = 'block';
                if (leafletDiv) leafletDiv.style.display = 'none';
                drawTacticalRadar();
            } else {
                if (radarCanvas) radarCanvas.style.display = 'none';
                if (leafletDiv) leafletDiv.style.display = 'block';
                initLeafletMap();
                updateLeafletMap();
            }
        });
    }

    if (btnSetHome) {
        btnSetHome.addEventListener('click', () => {
            setHomeOrigin();
        });
    }

    if (btnSetTarget) {
        btnSetTarget.addEventListener('click', () => {
            const defaultLat = gpsCurrentFix.latitude ? gpsCurrentFix.latitude.toFixed(6) : "37.774929";
            const defaultLon = gpsCurrentFix.longitude ? gpsCurrentFix.longitude.toFixed(6) : "-122.419416";
            const targetLatStr = prompt("Enter Target Latitude (Decimal Degrees):", defaultLat);
            const targetLonStr = prompt("Enter Target Longitude (Decimal Degrees):", defaultLon);
            if (targetLatStr && targetLonStr) {
                const lat = parseFloat(targetLatStr);
                const lon = parseFloat(targetLonStr);
                if (!isNaN(lat) && !isNaN(lon)) {
                    setTargetWaypoint(lat, lon);
                }
            }
        });
    }

    if (btnCenter) {
        btnCenter.addEventListener('click', () => {
            if (gpsMapMode === 'map' && leafletMap && gpsCurrentFix.latitude !== null) {
                leafletMap.setView([gpsCurrentFix.latitude, gpsCurrentFix.longitude], 18);
            } else {
                drawTacticalRadar();
            }
        });
    }

    if (btnClearTarget) {
        btnClearTarget.addEventListener('click', () => {
            clearTargetWaypoint();
        });
    }

    const btnClearPath = document.getElementById('btn-clear-path');
    if (btnClearPath) {
        btnClearPath.addEventListener('click', () => {
            roverPathHistory = [];
            if (leafletPathPolyline && leafletMap) {
                leafletMap.removeLayer(leafletPathPolyline);
                leafletPathPolyline = null;
            }
            drawTacticalRadar();
        });
    }

    if (btnAutoscroll) {
        btnAutoscroll.addEventListener('click', () => {
            nmeaAutoScroll = !nmeaAutoScroll;
            btnAutoscroll.innerText = nmeaAutoScroll ? "AUTOSCROLL ON" : "AUTOSCROLL OFF";
            btnAutoscroll.style.color = nmeaAutoScroll ? "var(--accent-blue)" : "var(--text-muted)";
        });
    }

    if (btnClearNmea) {
        btnClearNmea.addEventListener('click', () => {
            const logBox = document.getElementById('nmea-stream-log');
            if (logBox) logBox.innerHTML = '';
        });
    }

    if (radarCanvas) {
        radarCanvas.addEventListener('click', (e) => {
            if (gpsCurrentFix.latitude === null) return;
            const rect = radarCanvas.getBoundingClientRect();
            const clickX = e.clientX - rect.left;
            const clickY = e.clientY - rect.top;
            const cx = radarCanvas.width / 2;
            const cy = radarCanvas.height / 2;
            const maxRadius = Math.min(radarCanvas.width, radarCanvas.height) / 2 - 25;

            const dxPx = clickX - cx;
            const dyPx = clickY - cy;

            const dxMeters = (dxPx / maxRadius) * 50;
            const dyMeters = (-dyPx / maxRadius) * 50;

            const targetCoords = offsetLatLonByMeters(gpsCurrentFix.latitude, gpsCurrentFix.longitude, dxMeters, dyMeters);
            setTargetWaypoint(targetCoords.latitude, targetCoords.longitude);
        });
    }

    // IMU Command & Service Button Event Listeners
    const btnImuCalibrate = document.getElementById('btn-imu-calibrate');
    const btnImuTogglePub = document.getElementById('btn-imu-toggle-pub');
    const btnImuStatus = document.getElementById('btn-imu-status');
    const btnImuResetFilter = document.getElementById('btn-imu-reset-filter');
    const btnImuGetState = document.getElementById('btn-imu-get-state');
    const btnImuCalibrateAll = document.getElementById('btn-imu-calibrate-all');

    const btnCalibrateNorth = document.getElementById('btn-calibrate-north');
    const btnCalibrateAll = document.getElementById('btn-calibrate-all');
    if (btnCalibrateNorth) btnCalibrateNorth.addEventListener('click', callIMUCalibrate);
    if (btnCalibrateAll) btnCalibrateAll.addEventListener('click', callIMUCalibrateAll);
    if (btnImuCalibrate) btnImuCalibrate.addEventListener('click', callIMUCalibrate);
    if (btnImuCalibrateAll) btnImuCalibrateAll.addEventListener('click', callIMUCalibrateAll);
    if (btnImuTogglePub) btnImuTogglePub.addEventListener('click', callIMUTogglePublishing);
    if (btnImuStatus) btnImuStatus.addEventListener('click', callIMUGetStatus);
    if (btnImuResetFilter) btnImuResetFilter.addEventListener('click', callIMUResetFilter);
    if (btnImuGetState) btnImuGetState.addEventListener('click', callIMUGetState);

    drawTacticalRadar();
    updateGPSUI();
    updateIMUUI();
});

// ====================================================
// 10. IMU & ATTITUDE TELEMETRY & COMMAND SYSTEM
// ====================================================

function setupIMUROSInterfaces() {
    if (!ros) return;

    // 1. /imu/euler (Raw Euler Roll, Pitch, Yaw)
    imuEulerSub = new ROSLIB.Topic({
        ros: ros,
        name: '/imu/euler',
        messageType: 'geometry_msgs/msg/Vector3'
    });
    imuEulerSub.subscribe((msg) => {
        if (msg) {
            imuEulerData.roll = msg.x || 0.0;
            imuEulerData.pitch = msg.y || 0.0;
            imuEulerData.yaw = msg.z || 0.0;
            imuEulerData.lastUpdate = Date.now();
            updateIMUUI();
        }
    });

    // 2. /imu/filtered_euler (UKF Filtered Euler Roll, Pitch, Yaw)
    imuFilteredEulerSub = new ROSLIB.Topic({
        ros: ros,
        name: '/imu/filtered_euler',
        messageType: 'geometry_msgs/msg/Vector3'
    });
    imuFilteredEulerSub.subscribe((msg) => {
        if (msg) {
            imuFilteredEulerData.roll = msg.x || 0.0;
            imuFilteredEulerData.pitch = msg.y || 0.0;
            imuFilteredEulerData.yaw = msg.z || 0.0;
            imuFilteredEulerData.lastUpdate = Date.now();
            updateIMUUI();
        }
    });

    // 3. /imu/data_raw (Raw Accelerometer and Gyroscope Telemetry)
    imuRawSub = new ROSLIB.Topic({
        ros: ros,
        name: '/imu/data_raw',
        messageType: 'sensor_msgs/msg/Imu'
    });
    imuRawSub.subscribe((msg) => {
        if (msg) {
            if (msg.linear_acceleration) {
                imuMotionData.accel = {
                    x: msg.linear_acceleration.x || 0.0,
                    y: msg.linear_acceleration.y || 0.0,
                    z: msg.linear_acceleration.z || 0.0
                };
            }
            if (msg.angular_velocity) {
                imuMotionData.gyro = {
                    x: msg.angular_velocity.x || 0.0,
                    y: msg.angular_velocity.y || 0.0,
                    z: msg.angular_velocity.z || 0.0
                };
            }
            updateIMUUI();
        }
    });

    // 4. /imu/odometry (UKF 15D Filtered Odometry Pose and Twist)
    imuOdomSub = new ROSLIB.Topic({
        ros: ros,
        name: '/imu/odometry',
        messageType: 'nav_msgs/msg/Odometry'
    });
    imuOdomSub.subscribe((msg) => {
        if (msg && msg.pose && msg.pose.pose && msg.twist && msg.twist.twist) {
            imuOdomData.pos = {
                x: msg.pose.pose.position.x || 0.0,
                y: msg.pose.pose.position.y || 0.0,
                z: msg.pose.pose.position.z || 0.0
            };
            imuOdomData.vel = {
                x: msg.twist.twist.linear.x || 0.0,
                y: msg.twist.twist.linear.y || 0.0,
                z: msg.twist.twist.linear.z || 0.0
            };
            updateIMUUI();
        }
    });

    // 5. /ekf/odometry (Fused EKF Wheel+IMU+GPS Odometry Pose and Twist)
    ekfOdomSub = new ROSLIB.Topic({
        ros: ros,
        name: '/ekf/odometry',
        messageType: 'nav_msgs/msg/Odometry'
    });
    ekfOdomSub.subscribe((msg) => {
        if (msg && msg.pose && msg.pose.pose && msg.twist && msg.twist.twist) {
            imuOdomData.pos = {
                x: msg.pose.pose.position.x || 0.0,
                y: msg.pose.pose.position.y || 0.0,
                z: msg.pose.pose.position.z || 0.0
            };
            imuOdomData.vel = {
                x: msg.twist.twist.linear.x || 0.0,
                y: msg.twist.twist.linear.y || 0.0,
                z: msg.twist.twist.linear.z || 0.0
            };
            const ekfValEl = document.getElementById('val-ekf-telemetry-fusion');
            if (ekfValEl) {
                const x = imuOdomData.pos.x;
                const y = imuOdomData.pos.y;
                const vx = imuOdomData.vel.x;
                ekfValEl.innerText = `X: ${x.toFixed(2)}m | Y: ${y.toFixed(2)}m | Speed: ${vx.toFixed(2)} m/s`;
            }
            updateIMUUI();
        }
    });

    // 5. /imu/telemetry_str (Formatted Multi-line Telemetry String)
    imuStrSub = new ROSLIB.Topic({
        ros: ros,
        name: '/imu/telemetry_str',
        messageType: 'std_msgs/msg/String'
    });
    imuStrSub.subscribe((msg) => {
        if (msg && msg.data) {
            const streamLog = document.getElementById('imu-stream-log');
            if (streamLog) {
                streamLog.innerText = msg.data;
            }
        }
    });
}

function cleanupIMUROSInterfaces() {
    if (imuEulerSub) { imuEulerSub.unsubscribe(); imuEulerSub = null; }
    if (imuFilteredEulerSub) { imuFilteredEulerSub.unsubscribe(); imuFilteredEulerSub = null; }
    if (imuRawSub) { imuRawSub.unsubscribe(); imuRawSub = null; }
    if (imuFilteredSub) { imuFilteredSub.unsubscribe(); imuFilteredSub = null; }
    if (imuOdomSub) { imuOdomSub.unsubscribe(); imuOdomSub = null; }
    if (imuStrSub) { imuStrSub.unsubscribe(); imuStrSub = null; }
}

function logIMUServiceResponse(actionName, success, message) {
    const logBox = document.getElementById('imu-service-response-log');
    if (!logBox) return;
    const timeStr = new Date().toLocaleTimeString();
    const statusStr = success ? "SUCCESS" : "FAILED";
    const newEntry = `[${timeStr}] [${actionName}] -> ${statusStr}: ${message}\n`;
    logBox.innerText = newEntry + logBox.innerText;
}

function callIMUCalibrate() {
    if (!ros) {
        logIMUServiceResponse("Calibrate", false, "ROS Bridge not connected.");
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_telemetry/calibrate',
        serviceType: 'std_srvs/srv/Trigger'
    });
    service.callService(new ROSLIB.ServiceRequest({}), (result) => {
        if (result) {
            logIMUServiceResponse("Calibrate", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Calibrate", false, `Service Error: ${error}`);
    });
}

function callIMUCalibrateAll() {
    if (!ros) {
        logIMUServiceResponse("Calibrate All", false, "ROS Bridge not connected.");
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_telemetry/calibrate_all',
        serviceType: 'std_srvs/srv/Trigger'
    });
    service.callService(new ROSLIB.ServiceRequest({}), (result) => {
        if (result) {
            logIMUServiceResponse("Calibrate All", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Calibrate All", false, `Service Error: ${error}`);
    });
}

function callIMUTogglePublishing() {
    if (!ros) {
        logIMUServiceResponse("Toggle Publishing", false, "ROS Bridge not connected.");
        return;
    }
    imuPublishingEnabled = !imuPublishingEnabled;
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_telemetry/enable_publishing',
        serviceType: 'std_srvs/srv/SetBool'
    });
    const request = new ROSLIB.ServiceRequest({ data: imuPublishingEnabled });
    service.callService(request, (result) => {
        if (result) {
            logIMUServiceResponse("Toggle Publishing", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Toggle Publishing", false, `Service Error: ${error}`);
    });
}

function callIMUGetStatus() {
    if (!ros) {
        logIMUServiceResponse("Get Status", false, "ROS Bridge not connected.");
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_telemetry/get_status',
        serviceType: 'std_srvs/srv/Trigger'
    });
    service.callService(new ROSLIB.ServiceRequest({}), (result) => {
        if (result) {
            logIMUServiceResponse("Get Status", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Get Status", false, `Service Error: ${error}`);
    });
}

function callIMUResetFilter() {
    if (!ros) {
        logIMUServiceResponse("Reset Filter", false, "ROS Bridge not connected.");
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_kalman_filter/reset_filter',
        serviceType: 'std_srvs/srv/Trigger'
    });
    service.callService(new ROSLIB.ServiceRequest({}), (result) => {
        if (result) {
            logIMUServiceResponse("Reset Filter", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Reset Filter", false, `Service Error: ${error}`);
    });
}

function callIMUGetState() {
    if (!ros) {
        logIMUServiceResponse("Get Filter State", false, "ROS Bridge not connected.");
        return;
    }
    const service = new ROSLIB.Service({
        ros: ros,
        name: '/imu_kalman_filter/get_state',
        serviceType: 'std_srvs/srv/Trigger'
    });
    service.callService(new ROSLIB.ServiceRequest({}), (result) => {
        if (result) {
            logIMUServiceResponse("Get Filter State", result.success, result.message);
        }
    }, (error) => {
        logIMUServiceResponse("Get Filter State", false, `Service Error: ${error}`);
    });
}

function updateIMUUI() {
    const isRawAvailable = imuEulerData.lastUpdate !== null && (Date.now() - imuEulerData.lastUpdate < 3000);
    const isFilteredAvailable = imuFilteredEulerData.lastUpdate !== null && (Date.now() - imuFilteredEulerData.lastUpdate < 3000);
    const hasData = isRawAvailable || isFilteredAvailable;

    const imuDot = document.getElementById('imu-status-dot');
    const imuStatusText = document.getElementById('imu-status-text');
    if (imuDot) {
        imuDot.className = hasData ? "status-indicator connected" : "status-indicator disconnected";
    }
    if (imuStatusText) {
        imuStatusText.innerText = hasData ? (isRawAvailable ? "IMU ONLINE (RAW)" : "IMU ONLINE (UKF)") : "NO IMU DATA";
        imuStatusText.style.color = hasData ? "var(--accent-green)" : "var(--text-muted)";
    }

    const badge = document.getElementById('imu-mode-badge');
    if (badge) {
        badge.innerText = isRawAvailable ? "RAW TELEMETRY (/imu/euler)" : (isFilteredAvailable ? "FILTERED (UKF 15D)" : "OFFLINE");
    }

    // Prioritize raw BNO085 hardware orientation (/imu/euler) over dead-reckoned kalman filter
    const euler = isRawAvailable ? imuEulerData : imuFilteredEulerData;

    // Update Roll, Pitch, Yaw values
    const rollEl = document.getElementById('val-imu-roll');
    const pitchEl = document.getElementById('val-imu-pitch');
    const yawEl = document.getElementById('val-imu-yaw');

    if (rollEl) rollEl.innerText = `${euler.roll.toFixed(1)}°`;
    if (pitchEl) pitchEl.innerText = `${euler.pitch.toFixed(1)}°`;
    if (yawEl) yawEl.innerText = `${euler.yaw.toFixed(1)}°`;

    const valBaseYawEl = document.getElementById('val-base-yaw');
    if (valBaseYawEl && euler) {
        const yawRad = (euler.yaw * Math.PI) / 180.0;
        valBaseYawEl.innerText = `${yawRad.toFixed(2)} rad (${euler.yaw.toFixed(1)}°)`;
    }

    // Update bar indicators
    const barRoll = document.getElementById('bar-imu-roll');
    const barPitch = document.getElementById('bar-imu-pitch');
    const barYaw = document.getElementById('bar-imu-yaw');

    if (barRoll) {
        const rollPct = Math.min(100, Math.max(0, ((euler.roll + 180) / 360) * 100));
        barRoll.style.width = `${rollPct}%`;
    }
    if (barPitch) {
        const pitchPct = Math.min(100, Math.max(0, ((euler.pitch + 180) / 360) * 100));
        barPitch.style.width = `${pitchPct}%`;
    }
    if (barYaw) {
        const yawNormalized = ((euler.yaw % 360) + 360) % 360;
        barYaw.style.width = `${(yawNormalized / 360) * 100}%`;
    }

    // Motion data (Accel & Gyro)
    const accelEl = document.getElementById('val-imu-accel');
    const gyroEl = document.getElementById('val-imu-gyro');
    if (accelEl) {
        accelEl.innerText = `X: ${imuMotionData.accel.x.toFixed(2)} | Y: ${imuMotionData.accel.y.toFixed(2)} | Z: ${imuMotionData.accel.z.toFixed(2)} m/s²`;
    }
    if (gyroEl) {
        gyroEl.innerText = `X: ${imuMotionData.gyro.x.toFixed(2)} | Y: ${imuMotionData.gyro.y.toFixed(2)} | Z: ${imuMotionData.gyro.z.toFixed(2)} rad/s`;
    }

    // Odometry
    const odomPosEl = document.getElementById('val-imu-odom-pos');
    const odomVelEl = document.getElementById('val-imu-odom-vel');
    if (odomPosEl) {
        odomPosEl.innerText = `X: ${imuOdomData.pos.x.toFixed(2)}m | Y: ${imuOdomData.pos.y.toFixed(2)}m | Z: ${imuOdomData.pos.z.toFixed(2)}m`;
    }
    if (odomVelEl) {
        odomVelEl.innerText = `Vx: ${imuOdomData.vel.x.toFixed(2)} | Vy: ${imuOdomData.vel.y.toFixed(2)} | Vz: ${imuOdomData.vel.z.toFixed(2)} m/s`;
    }

    // System Telemetry cards
    const sysEuler = document.getElementById('val-imu-telemetry-euler');
    const sysMotion = document.getElementById('val-imu-telemetry-motion');
    if (sysEuler) {
        sysEuler.innerText = hasData ? `R: ${euler.roll.toFixed(1)}° | P: ${euler.pitch.toFixed(1)}° | Y: ${euler.yaw.toFixed(1)}°` : "R: --, P: --, Y: --";
    }
    if (sysMotion) {
        sysMotion.innerText = hasData ? `A: ${imuMotionData.accel.z.toFixed(1)} m/s² | G: ${imuMotionData.gyro.z.toFixed(2)} rad/s` : "A: -- | G: --";
    }

    // Redraw tactical radar if open so heading arrow updates
    if (gpsMapMode === 'radar') {
        drawTacticalRadar();
    }
}

// Morse Code Dictionary for Live Dashboard Teletype Decoding
const MORSE_TO_ENGLISH_TABLE = {
    '*-': 'A', '-***': 'B', '-*-*': 'C', '-**': 'D', '*': 'E',
    '**-*': 'F', '--*': 'G', '****': 'H', '**': 'I', '*---': 'J',
    '-*-': 'K', '*-**': 'L', '--': 'M', '-*': 'N', '---': 'O',
    '*--*': 'P', '--*-': 'Q', '*-*': 'R', '***': 'S', '-': 'T',
    '**-': 'U', '***-': 'V', '*--': 'W', '-**-': 'X', '-*--': 'Y',
    '--**': 'Z', '-----': '0', '*----': '1', '**---': '2', '***--': '3',
    '****-': '4', '*****': '5', '-****': '6', '--***': '7', '---**': '8',
    '----*': '9', '*-*-*-': '.', '--**--': ',', '**--**': '?'
};

let morseAudioEnabled = false;
let audioCtx = null;

function decodeMorseStringToEnglish(morseStr) {
    if (!morseStr) return "";
    const words = morseStr.split('  '); // Double space between words
    return words.map(word => {
        const letters = word.split(' '); // Single space between letters
        return letters.map(code => MORSE_TO_ENGLISH_TABLE[code] || '?').join('');
    }).join(' ');
}

function playMorseCWBeep(durationMs) {
    if (!morseAudioEnabled) return;
    try {
        if (!audioCtx) {
            audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        if (audioCtx.state === 'suspended') {
            audioCtx.resume();
        }
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();

        osc.type = 'sine';
        osc.frequency.setValueAtTime(750, audioCtx.currentTime); // 750Hz CW tone

        gain.gain.setValueAtTime(0.15, audioCtx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + (durationMs / 1000.0));

        osc.connect(gain);
        gain.connect(audioCtx.destination);

        osc.start();
        osc.stop(audioCtx.currentTime + (durationMs / 1000.0));
    } catch(e) {
        console.warn("Audio Context Error:", e);
    }
}

function updateMorseTeletype(rawMsg) {
    const rawStreamEl = document.getElementById('morse-raw-stream');
    const teletypeEl = document.getElementById('morse-teletype-decoded');
    const lastCharBadge = document.getElementById('morse-last-char-badge');

    if (rawStreamEl) rawStreamEl.innerText = rawMsg || '---';

    if (rawMsg && rawMsg !== '---') {
        const decodedText = decodeMorseStringToEnglish(rawMsg);
        if (teletypeEl) {
            teletypeEl.innerText = decodedText ? decodedText : '[ DECODING... ]';
        }

        // Get last symbol
        const parts = rawMsg.trim().split(' ');
        const lastSymbol = parts[parts.length - 1];
        const lastChar = MORSE_TO_ENGLISH_TABLE[lastSymbol] || '--';

        if (lastCharBadge) {
            lastCharBadge.innerText = `SYMBOL: ${lastSymbol} (${lastChar})`;
        }

        // Play audio beep for last symbol
        if (lastSymbol.endsWith('-')) {
            playMorseCWBeep(220); // Daw beep
        } else if (lastSymbol.endsWith('*')) {
            playMorseCWBeep(80);  // Dit beep
        }
    }
}

function simulateMorseTransmission() {
    logMorseConsole("[Simulator] Injecting test Morse sequence...");
    const sampleSequences = [
        "*** - --- *--*",                        // STOP
        "*** - --- *--* **** .. *** - --- *--*", // STOP HI STOP
        "**** . .-.. .-.. ---",                  // HELLO
        "*- -* -*--"                             // ANY
    ];
    const seq = sampleSequences[Math.floor(Math.random() * sampleSequences.length)];

    if (connected && ros) {
        const pub = new ROSLIB.Topic({
            ros: ros,
            name: '/morse_code',
            messageType: 'enigma_machine_interfaces/msg/Morse'
        });
        pub.publish(new ROSLIB.Message({ message: seq }));
        logMorseConsole(`[Simulator] Published to /morse_code: "${seq}"`);
    } else {
        // Local UI simulation if offline
        updateMorseTeletype(seq);
        logMorseConsole(`[Offline Sim] Decoded: "${decodeMorseStringToEnglish(seq)}"`);
    }
}

let morseStrSub = null;
let morsePixelSub = null;

function setupMorseSubscribers() {
    if (!ros || !connected) return;

    if (morseStrSub) {
        try { morseStrSub.unsubscribe(); } catch(e){}
    }
    if (morsePixelSub) {
        try { morsePixelSub.unsubscribe(); } catch(e){}
    }

    morseStrSub = new ROSLIB.Topic({
        ros: ros,
        name: '/morse_code_str',
        messageType: 'std_msgs/msg/String'
    });

    morseStrSub.subscribe((message) => {
        const rawMsg = (message && message.data !== undefined) ? message.data : '---';
        updateMorseTeletype(rawMsg);
        logMorseConsole(`[ROS Topic]: Received '${rawMsg}'`);
        queryTerminalPassword();
    });

    morsePixelSub = new ROSLIB.Topic({
        ros: ros,
        name: '/morse_recorder/pixel_count',
        messageType: 'std_msgs/msg/Int32'
    });

    morsePixelSub.subscribe((message) => {
        const pixCount = (message && message.data !== undefined) ? message.data : 0;
        const telemetryEl = document.getElementById('morse-pixel-telemetry');
        if (telemetryEl) {
            telemetryEl.innerText = `WHITE PIXELS: ${pixCount} px`;
            telemetryEl.style.color = pixCount > 0 ? "var(--accent-amber)" : "var(--text-muted)";
        }
    });
}

function cleanupMorseSubscribers() {
    if (morseStrSub) {
        try { morseStrSub.unsubscribe(); } catch(e){}
        morseStrSub = null;
    }
    if (morsePixelSub) {
        try { morsePixelSub.unsubscribe(); } catch(e){}
        morsePixelSub = null;
    }
}

function setMorseRecorderParam(paramName, val) {
    if (!connected || !ros) return;

    const paramService = new ROSLIB.Service({
        ros: ros,
        name: '/morse_recorder/set_parameters',
        serviceType: 'rcl_interfaces/srv/SetParameters'
    });

    const req = new ROSLIB.ServiceRequest({
        parameters: [
            {
                name: paramName,
                value: {
                    type: 2, // INTEGER
                    integer_value: parseInt(val)
                }
            }
        ]
    });

    paramService.callService(req, (res) => {
        logMorseConsole(`[Param Set] ${paramName} -> ${val}`);
    }, (err) => {
        logMorseConsole(`[Param Error] Failed to update ${paramName}`);
    });
}

function queryTerminalPassword() {
    if (!connected || !ros) {
        logMorseConsole("[Error] Not connected to ROS 2 bridge.");
        return;
    }

    const service = new ROSLIB.Service({
        ros: ros,
        name: '/get_password',
        serviceType: 'enigma_machine_interfaces/srv/GetPassword'
    });

    const request = new ROSLIB.ServiceRequest({});

    logMorseConsole("Calling /get_password service...");
    service.callService(request, (result) => {
        if (result && result.password !== undefined) {
            const passEl = document.getElementById('morse-terminal-password');
            if (passEl) {
                passEl.innerText = result.password ? result.password.toUpperCase() : '[ UNLOCKED / WAITING ]';
            }
            logMorseConsole(`[GetPassword Success] Decoded Password: "${result.password}"`);
        } else {
            logMorseConsole("[GetPassword Error] Invalid service response.");
        }
    }, (error) => {
        logMorseConsole(`[GetPassword Error] ${error}`);
    });
}

function setAndEncodeMessage() {
    const inputEl = document.getElementById('input-morse-message');
    if (!inputEl || !inputEl.value.trim()) {
        logMorseConsole("[Warning] Please enter a message to encode.");
        return;
    }

    const msg = inputEl.value.trim();
    if (!connected || !ros) {
        logMorseConsole("[Error] Not connected to ROS 2 bridge.");
        return;
    }

    const service = new ROSLIB.Service({
        ros: ros,
        name: '/set_message',
        serviceType: 'enigma_machine_interfaces/srv/SetMessage'
    });

    const request = new ROSLIB.ServiceRequest({
        message: msg
    });

    logMorseConsole(`Calling /set_message with '${msg}'...`);
    service.callService(request, (result) => {
        logMorseConsole(`[SetMessage Success] Message '${msg}' set & encoded for appendage.`);
    }, (error) => {
        logMorseConsole(`[SetMessage Error] ${error}`);
    });
}

function transmitCachedMessage() {
    if (!connected || !ros) {
        logMorseConsole("[Error] Not connected to ROS 2 bridge.");
        return;
    }

    const service = new ROSLIB.Service({
        ros: ros,
        name: '/transmit_cached',
        serviceType: 'std_srvs/srv/Trigger'
    });

    const request = new ROSLIB.ServiceRequest({});

    logMorseConsole(`Fetching cached Morse message to send to Arduino...`);
    service.callService(request, (result) => {
        if(result.success) {
            const morseStr = result.message;
            logMorseConsole(`[Fetched] '${morseStr}'. Sending to Arduino...`);
            
            // Call the MorseBridge /morse_converse service to get Arduino feedback
            callSerialService('/morse_converse', 'serial_interfaces/srv/Converse', { outgoing: morseStr + '\n' }, (convResult) => {
                logMorseConsole(`[Arduino Feedback]: ${convResult.incoming}`);
            }, (err) => {
                logMorseConsole(`[Arduino Error]: ${err}`);
            });
        } else {
            logMorseConsole(`[Error] Failed to fetch.`);
        }
    }, (error) => {
        logMorseConsole(`[Service Error] ${error}`);
    });
}

function logMorseConsole(text) {
    const logEl = document.getElementById('morse-console-log');
    if (!logEl) return;
    const timeStr = new Date().toLocaleTimeString();
    logEl.textContent = `[${timeStr}] ${text}\n` + logEl.textContent;
}

function clearMorseTeletype() {
    const rawStreamEl = document.getElementById('morse-raw-stream');
    const teletypeEl = document.getElementById('morse-teletype-decoded');
    const lastCharBadge = document.getElementById('morse-last-char-badge');

    if (rawStreamEl) rawStreamEl.innerText = '---';
    if (teletypeEl) teletypeEl.innerText = '[ TELETYPE READY ]';
    if (lastCharBadge) lastCharBadge.innerText = 'SYMBOL: --';

    logMorseConsole("[UI] Cleared raw stream and teletype display.");

    // Call ROS 2 service to clear memory buffer on morse_recorder node
    if (connected && ros) {
        const service = new ROSLIB.Service({
            ros: ros,
            name: '/morse_recorder/reset',
            serviceType: 'std_srvs/srv/Trigger'
        });
        service.callService(new ROSLIB.ServiceRequest({}), (result) => {
            logMorseConsole("[Reset Success] Cleared Morse node buffer memory.");
        }, (error) => {
            // Silently ignore if node is currently stopped
        });
    }
}

// Attach event listeners for Morse Code Section
document.addEventListener('DOMContentLoaded', () => {
    const btnGetPass = document.getElementById('btn-morse-get-password');
    if (btnGetPass) {
        btnGetPass.addEventListener('click', queryTerminalPassword);
    }

    const btnClear = document.getElementById('btn-morse-clear');
    if (btnClear) {
        btnClear.addEventListener('click', clearMorseTeletype);
    }

    const btnSetMsg = document.getElementById('btn-morse-set-message');
    if (btnSetMsg) {
        btnSetMsg.addEventListener('click', setAndEncodeMessage);
    }

    const btnTransmitCached = document.getElementById('btn-morse-transmit-cached');
    if (btnTransmitCached) {
        btnTransmitCached.addEventListener('click', transmitCachedMessage);
    }

    const btnSimulate = document.getElementById('btn-morse-simulate');
    if (btnSimulate) {
        btnSimulate.addEventListener('click', simulateMorseTransmission);
    }

    const btnAudioToggle = document.getElementById('btn-morse-audio-toggle');
    if (btnAudioToggle) {
        btnAudioToggle.addEventListener('click', () => {
            morseAudioEnabled = !morseAudioEnabled;
            btnAudioToggle.innerText = morseAudioEnabled ? "🔊 CW BEEP: ON" : "🔊 CW BEEP: OFF";
            btnAudioToggle.className = morseAudioEnabled ? "neon-btn-green" : "neon-btn-blue";
            if (morseAudioEnabled) playMorseCWBeep(100);
        });
    }

    const inputMsg = document.getElementById('input-morse-message');
    if (inputMsg) {
        inputMsg.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                setAndEncodeMessage();
            }
        });
    }

    const sliderBright = document.getElementById('slider-brightness-thresh');
    const valBright = document.getElementById('val-brightness-thresh');
    if (sliderBright && valBright) {
        sliderBright.addEventListener('input', (e) => {
            valBright.innerText = e.target.value;
        });
        sliderBright.addEventListener('change', (e) => {
            setMorseRecorderParam('brightness_threshold', e.target.value);
        });
    }

    const sliderPix = document.getElementById('slider-pixel-thresh');
    const valPix = document.getElementById('val-pixel-thresh');
    if (sliderPix && valPix) {
        sliderPix.addEventListener('input', (e) => {
            valPix.innerText = e.target.value;
        });
        sliderPix.addEventListener('change', (e) => {
            setMorseRecorderParam('pixel_count_threshold', e.target.value);
        });
    }
});




