// Ref: https://github.com/ZachSaucier/Disintegrate

var particles = [];

var ExplodingParticle = function() {
    // Set how long we want our particle to animate for
    this.animationDuration = 1000; // in ms

    // Set the speed for our particle
    this.speed = {
        x: -5 + Math.random() * 10,
        y: -5 + Math.random() * 10
    };

    // Size our particle
    this.radius = 2 + Math.random() * 2;

    // Set a max time to live for our particle
    this.life = 30 + Math.random() * 10;
    this.remainingLife = this.life;

    // This function will be called by our animation logic later on
    this.draw = ctx => {
        let p = this;

        if (this.remainingLife > 0 && this.radius > 0) {
            // Draw a circle at the current location
            ctx.beginPath();
            ctx.arc(p.startX, p.startY, p.radius, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(' + this.rgbArray[0] + ',' + this.rgbArray[1] + ',' + this.rgbArray[2] + ',' + this.rgbArray[3] + ')';
            ctx.fill();

            // Update the particle's location and life
            p.remainingLife--;
            p.radius -= 0.25;
            p.startX += p.speed.x;
            p.startY += p.speed.y;
            p.speed.y += 1;
        }
    }
}

function createParticleAtPoint(x, y, colorData) {
    let particle = new ExplodingParticle();
    particle.rgbArray = colorData;
    particle.startX = x;
    particle.startY = y;
    particle.startTime = Date.now();
    particle.endTime = particle.startTime + particle.animationDuration;

    particles.push(particle);
}

function update_particles(ctx) {
    // Draw all of our particles in their new location
    for (let i = 0; i < particles.length; i++) {
        particles[i].draw(ctx);
        if (Date.now() > particles[i].endTime) {
            particles.splice(i, 1);
        }
    }
}

