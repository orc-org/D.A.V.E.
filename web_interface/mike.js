const mike = document.getElementById("mike");
const mike_laugh = document.getElementById("mike-laugh");

let x = 100;
let y = 100;
let dx = 1;
let dy = 1;

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

    requestAnimationFrame(animateMike);
}

animateMike();


mike.onclick = function() {

    console.log("Mike Myers");

    mike_laugh.currentTime = 0;
    mike_laugh.play();

};


