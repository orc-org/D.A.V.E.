// ROS 2 Bridge Connection State
let ros = null;
let connected = false;

// ROS 2 Topics
let cmdVelPub = null;
let armTargetPub = null;
let autoModePub = null;
let armManualVelPub = null;
let gripperTargetPub = null;
let gripperActionClient = null;

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

// 3D Viewer State
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

    gripperTargetPub = new ROSLIB.Topic({
        ros: ros,
        name: '/gripper_target',
        messageType: 'std_msgs/msg/Float32'
    });

    // Action Client for gripper
    gripperActionClient = new ROSLIB.ActionClient({
        ros: ros,
        serverName: '/gripper_command',
        actionName: 'arm_interfaces/action/GripperCommand'
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
            if (t.child_frame_id === 'base_link') {
                let x = t.transform.translation.x;
                let y = t.transform.translation.y;
                valBasePos.innerText = `X: ${x.toFixed(2)}, Y: ${y.toFixed(2)}`;

                // Extract Heading (Yaw) from Quaternion
                let qz = t.transform.rotation.z;
                let qw = t.transform.rotation.w;
                let yaw = 2.0 * Math.atan2(qz, qw);
                let deg = yaw * (180.0 / Math.PI);
                valBaseYaw.innerText = `${yaw.toFixed(2)} rad (${deg.toFixed(1)}°)`;
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

    resetModuleStatusUI();
    robotStatePublisherRunning = false;
    
    cmdVelPub = null;
    armTargetPub = null;
    autoModePub = null;
    armManualVelPub = null;
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
        
        // UI Button Gripper control (runs in both modes, whether gamepad is connected or not)
        if (gripperOpenBtnActive) {
            const oldVal = currentGripperTarget;
            currentGripperTarget = Math.max(0.0, currentGripperTarget - 0.05);
            if (Math.abs(currentGripperTarget - oldVal) > 0.001) {
                publishGripperTarget(currentGripperTarget);
            }
        }
        if (gripperCloseBtnActive) {
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
            
            publishDriveCommand();
            publishArmManualCommand();
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
    if (connected && armManualVelPub) {
        const msg = new ROSLIB.Message({
            data: 0.0
        });
        armManualVelPub.publish(msg);
    }
}

function calculateManualArmVelocities() {
    let speed = 0.0;
    // Q/PageUp moves arm UP, E/PageDown moves arm DOWN
    if (keysPressed['q'] || keysPressed['pageup'] || armUpBtnActive) {
        speed -= 0.3; // Inverted sign so arm goes up
    }
    if (keysPressed['e'] || keysPressed['pagedown'] || armDownBtnActive) {
        speed += 0.3; // Inverted sign so arm goes down
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
        param: '/robot_state_publisher:robot_description'
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

    // Build module list dynamically from received status if not initialized
    if (!modulesInitialized) {
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
        modulesInitialized = true;
    }

    // Update statuses and buttons
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
}

function toggleModule(key) {
    if (!connected || !ros) {
        console.warn("Cannot toggle module: Not connected to ROS.");
        return;
    }
    const btn = document.getElementById(`btn-toggle-${key}`);
    if (!btn) return;
    
    const shouldStart = btn.innerText === "START";
    
    // Set button to busy/loading state
    btn.innerText = shouldStart ? "STARTING..." : "STOPPING...";
    btn.disabled = true;
    
    const service = new ROSLIB.Service({
        ros: ros,
        name: `/process_manager/toggle_${key}`,
        serviceType: 'std_srvs/srv/SetBool'
    });
    
    const request = new ROSLIB.ServiceRequest({
        data: shouldStart
    });
    
    service.callService(request, (result) => {
        btn.disabled = false;
        if (result && result.success) {
            console.log(`Successfully toggled ${key}: ${result.message}`);
        } else {
            console.error(`Failed to toggle ${key}: ${result ? result.message : 'Unknown error'}`);
            btn.innerText = shouldStart ? "START" : "STOP";
        }
    }, (error) => {
        btn.disabled = false;
        console.error(`Service call error for ${key}:`, error);
        btn.innerText = shouldStart ? "START" : "STOP";
    });
}

// Collapsible Panel Event Listener
btnToggleModules.addEventListener('click', () => {
    modulesDrawer.classList.toggle('collapsed');
    modulesCollapseIcon.classList.toggle('collapsed');
});

// Click delegation for toggle buttons
modulesDrawer.addEventListener('click', (e) => {
    const btn = e.target.closest('.module-toggle-btn');
    if (btn) {
        const key = btn.getAttribute('data-module');
        if (key) {
            toggleModule(key);
        }
    }
});

