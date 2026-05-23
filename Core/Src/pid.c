

#include "pid.h"

#ifndef PID_INIT
#define PID_INIT
void pid_init(PID_Controller *pid,
              float kp, float ki, float kd,
              float ilim, float olim)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->prev_error[0] = 0.0f;
    pid->integral_limit = ilim;
    pid->output_limit = olim;
}
#endif /*PID_INIT*/
void buck_pid_init(BuckCascadedPID *buck,
                   float v_kp, float v_ki, float v_kd,
                   float i_kp, float i_ki, float i_kd)
{
    pid_init(&buck->outer, v_kp, v_ki, v_kd, 5.0f, DEFAULT_IREF_MAX);
    pid_init(&buck->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    buck->vref = DEFAULT_VREF;
    buck->duty = 0.0f;
    buck->iref_limit = DEFAULT_IREF_MAX;
}

float pid_step(PID_Controller *pid, float error)
{
    float p, d;

    pid->integral += error;
    if (pid->integral > pid->integral_limit)
        pid->integral = pid->integral_limit;
    if (pid->integral < -pid->integral_limit)
        pid->integral = -pid->integral_limit;

    p = pid->kp * error;
    d = pid->kd * (error - pid->prev_error[0]);

    float out = p + pid->ki * pid->integral + d;

    if (out > pid->output_limit)
        out = pid->output_limit;
    else if (out < -pid->output_limit)
        out = -pid->output_limit;

    pid->prev_error[0] = error;
    pid->output_cache = out; /* store output for potential debugging */
    return out;
}

#define BUCK_OUTER_UPDATE_RATE 10 /* update outer loop every 10 calls */
static int buckOuterCounter = 10;

float buck_pid_update(BuckCascadedPID *buck,
                      float v_ref,
                      float vsense_raw, /* ADC result [V] */
                      float isense_raw)
{
    float v_in, i_in;
    float v_err, iref, i_err;

    /* scale ADC readings back to physical units */
    v_in = vsense_raw / VOLTAGE_GAIN;
    i_in = isense_raw / CURRENT_GAIN;

    /* ---- outer voltage loop ---- */
    v_err = v_ref - v_in;
    if (buckOuterCounter >= 10)
    {
        iref = pid_step(&buck->outer, v_err);
        buckOuterCounter = 0;
    }
    else
    {
        iref = *(&buck->outer.output_cache);
        buckOuterCounter++;
    }

    /* clamp current reference */
    if (iref > buck->iref_limit)
        iref = buck->iref_limit;
    if (iref < -buck->iref_limit)
        iref = -buck->iref_limit;

    /* ---- inner current loop ---- */
    i_err = iref - i_in;
    buck->duty = pid_step(&buck->inner, i_err);

    /* clamp duty */
    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;
    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}


