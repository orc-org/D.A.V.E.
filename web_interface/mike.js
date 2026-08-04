const mike = document.getElementById("mike");
//const mike_laugh = document.getElementById("mike-laugh");
const foxy = document.getElementById("foxy");
const foxy_scream = document.getElementById("foxy-scream")
//Mike Myers Stim List
const mikeSounds = [
    document.getElementById("mike-laugh"),
    document.getElementById("accident"),
    document.getElementById("babysit"),
    document.getElementById("excited"),
    document.getElementById("oh_yeah"),
    document.getElementById("ruined"),
    document.getElementById("sonuva"),
    document.getElementById("stupid"),
    //document.getElementById("swamp"),
    document.getElementById("that_couldve_gone_better"),
    document.getElementById("thats_disgusting"),
    document.getElementById("ugly_mom"),
    document.getElementById("wheeze"),
    //document.getElementById("fun_song")
];
mikeSounds.forEach((sound, index) => {
    console.log(index, sound);
});

let x = Math.random() * window.innerWidth;
let y = Math.random() * window.innerHeight;

let dx = (Math.random() < 0.5 ? -1 : 1);
let dy = (Math.random() < 0.5 ? -1 : 1);
let jumpscareCooldown = false;

function animateMike() {

    x += dx;
    y += dy;

    if (x <= 0 || x + mike.offsetWidth >= window.innerWidth) {
        dx *= -1;
    }

    if (y <= 0 || y + mike.offsetHeight >= window.innerHeight) {
        dy *= -1;
    }

    mike.style.left = x + "px";
    mike.style.top = y + "px";

    const margin = 5;

    const hitLeft = x <= margin;
    const hitRight = x + mike.offsetWidth >= window.innerWidth - margin;
    const hitTop = y <= margin;
    const hitBottom = y + mike.offsetHeight >= window.innerHeight - margin;

    if ((hitLeft || hitRight) && (hitTop || hitBottom) && !jumpscareCooldown) {
        console.log("CORNER HIT!");
        triggerJumpscare();
    }

    requestAnimationFrame(animateMike);
}

animateMike();

function triggerJumpscare() {

    if (jumpscareCooldown) return;

    jumpscareCooldown = true;

    const foxy = document.getElementById("foxy");

    // restart gif
    foxy.src = "images/foxy_jumpscare_visual.gif?" + Date.now();

    foxy.style.display = "block";

    foxy_scream.currentTime = 0;
    foxy_scream.play();

    setTimeout(() => {
        foxy.style.display = "none";
        jumpscareCooldown = false;
    }, 800);
}

function playMikeSound() {

    let random = Math.random();

    let sound;

    if (random < 0.01) {
        sound = document.getElementById("fun_song");
    } 
    else if (random < 0.04) {
        sound = document.getElementById("swamp");
    }
    else {
        sound = mikeSounds[Math.floor(Math.random() * mikeSounds.length)];
    }

    sound.currentTime = 0;
    sound.play();
}

mike.onclick = playMikeSound;

window.playMikeSound = playMikeSound;


function spawnMikeAtCorner() {

    // Put Mike near the top-right corner
    x = window.innerWidth - mike.offsetWidth - 2;
    y = 2;

    // Make him travel into the corner
    dx = 1;
    dy = -1;

    console.log("Mike spawned near corner");
}

document.getElementById("test-corner").onclick = spawnMikeAtCorner;

document.getElementById("test-jumpscare").onclick = triggerJumpscare;
