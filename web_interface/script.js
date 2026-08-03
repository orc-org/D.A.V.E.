// Set Sensitivity
document.addEventListener('DOMContentLoaded', () => {
    const sensitivityDiv = document.getElementById('sensitivity');
    const dataInput = sensitivityDiv.querySelector('.data-input');
    const buttons = sensitivityDiv.querySelectorAll('.toggle-three');

    function updateSensitivity(button){

        dataInput.textContent = button.textContent;

        buttons.forEach(btn => {
            btn.classList.remove('active-toggle');
        });

        button.classList.add('active-toggle');
    }

    window.setSensitivityButton = function(button){

        updateSensitivity(button);

        if (window.publishSensitivity) {
            window.publishSensitivity(button.textContent);
        }
    };

    buttons.forEach(button => {
        button.addEventListener('click', () => {
            window.setSensitivityButton(button);
        });
    });
    
});

// Set Drive Mode
document.addEventListener('DOMContentLoaded', () => {
    const sensitivityDiv = document.getElementById('arm-toggle');
    
    if (!sensitivityDiv) return;

    const dataInput = sensitivityDiv.querySelector('.data-input');
    const buttons = sensitivityDiv.querySelectorAll('.toggle-two');

    function updateStates(clickedButton) {
        buttons.forEach(button => {
            if (button === clickedButton) {
                button.classList.add('active-toggle');
                button.style.backgroundColor = 'var(--accent-active)';
            } else {
                button.classList.remove('active-toggle');
                button.style.backgroundColor = 'var(--accent-inactive)';
            }
        });
    }

    //Should make a function that both this js and controller.js can access which sends mode to ros
    window.setModeButton = function(button){

        dataInput.textContent = button.textContent;
        updateStates(button);

        // Send mode change to ROS
        if (window.publishMode) {
            window.publishMode(button.textContent);
        }
    };

    //Mouse Click changes button
    buttons.forEach(button => {
        button.addEventListener('click', () => {
            //dataInput.textContent = button.textContent;
            setModeButton(button);
        });
    
    });
});

//Set Lights
document.addEventListener('DOMContentLoaded', () => {
    const lightDiv = document.getElementById('light-toggle');
    const dataInput = lightDiv.querySelector('.data-input');
    const buttons = lightDiv.querySelectorAll('.toggle-two');

    buttons.forEach(button => {
        button.addEventListener('click', () => {
            dataInput.textContent = button.textContent;
            buttons.forEach(btn => btn.classList.remove('active-toggle'));
            button.classList.add('active-toggle');
        });
    });
});

// Button Push
document.addEventListener('DOMContentLoaded', () => {
    const button = document.getElementById('arm-push');

    button.addEventListener('click', () => {
        button.classList.toggle('active');
    });
});

// Servo Position
document.addEventListener('DOMContentLoaded', () => {
    const positionButtons = document.querySelectorAll('button.toggle-three[position]');

    positionButtons.forEach(button => {
        button.addEventListener('click', (event) => {
            const clickedButton = event.currentTarget;

            const parentContainer = clickedButton.closest('.data');

            if (parentContainer) {
                const dataInput = parentContainer.querySelector('.data-input');
                
                if (dataInput) {
                    dataInput.textContent = clickedButton.textContent.trim();
                }
            }
        });
    });
});