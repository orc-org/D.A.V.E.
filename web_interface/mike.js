const mike = document.getElementById("mike");
const mike_laugh = document.getElementById("mike-laugh");
const foxy = document.getElementById("foxy");
const foxy_scream = document.getElementById("foxy-scream")
//console.log(foxy_scream);

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

    const margin = 10;

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

mike.onclick = function() {

    console.log("Mike Myers");

    mike_laugh.currentTime = 0;
    mike_laugh.play();

};
/*
function spawnMikeAtCorner() {

    // Put Mike near the top-right corner
    x = window.innerWidth - mike.offsetWidth - 2;
    y = 2;

    // Make him travel into the corner
    dx = 1;
    dy = -1;

    console.log("Mike spawned near corner");
}
*/
//document.getElementById("test-corner").onclick = spawnMikeAtCorner;

//document.getElementById("test-jumpscare").onclick = triggerJumpscare;
