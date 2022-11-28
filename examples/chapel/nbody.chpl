use Time;
use IO;

/* Use ./nbody --n=20000 --iterations=5 to run with different arguments. */
config const n: int = 10000;
config const iterations: int = 5;

proc accelerate(pos1: [1..3] real, pos2: [1..3] real, mass: real)
{
    var norm: real = (+ reduce ((pos2 - pos1)**2))**1.5;

    if (norm == 0) {
        return 0;
    } else {
        return + reduce ((pos2 - pos1) * mass / norm);
    }
}

proc accelerateAll(positions: [1..n, 1..3] real, masses: [1..n] real)
{
    var accel: [1..n, 1..3] real = 0;

    forall (i, j) in {1..n, 1..n} do
        accel[i, ..] += accelerate(positions[i, ..], positions[j, ..], masses[j]);

    return accel;
}

/* Is Chapel call by reference or value for arrays? If the latter, need to return the
 * velocities some way as well for a real application. */
proc advance(positions: [1..n, 1..3] real, velocities: [1..n, 1..3] real, masses: [1..n] real,
    dt: real, iterations: int)
{
    var accel: [1..n, 1..3] real;

    for t in 1..iterations do
        accel = accelerateAll(positions, masses);
        velocities += accel;
        positions = velocities * dt;

    return positions;
}

var masses: [1..n] real = 1;
var positions: [1..n, 1..3] real = 1;
var velocities: [1..n, 1..3] real = 1;

var watch: Timer;
watch.start();

positions = advance(positions, velocities, masses, 0.1, iterations);

watch.stop();
stderr.writeln('Anti-optimisation number: ', + reduce velocities, '\n');

stdout.writeln(3.0 * 16 * n * n / 1000000000.0 / watch.elapsed(), '\n');
