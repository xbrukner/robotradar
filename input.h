#ifndef INPUT
#define INPUT

struct Input {
    /// Angle in degrees - <0, 360>
    unsigned angle;

    /// Relative distance in float - <0, 1>
    float distance;

    Input(unsigned angle, float distance) :
        angle(angle), distance(distance) {
        if (angle >= 360) angle = 0;
        if (distance < 0.0) distance = 0.0;
        if (distance > 1.0) distance = 1.0;
    }
};

#endif // INPUT

