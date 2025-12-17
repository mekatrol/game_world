#pragma once

struct FpsCounter
{
    double last_time = 0.0;
    int frames = 0;
    int fps = 0;

    void tick(double now)
    {
        frames++;
        if (now - last_time >= 1.0)
        {
            fps = frames;
            frames = 0;
            last_time = now;
        }
    }
};
