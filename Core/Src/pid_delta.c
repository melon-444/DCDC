
#include "pid_delta.h"


void pid_delta_init(PID_Controller *pid,
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


void buck_pid_delta_init(BuckCascadedPID *buck,
                         float v_kp, float v_ki, float v_kd,
                         float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&buck->outer, v_kp, v_ki, v_kd, 5.0f, DEFAULT_IREF_MAX);
    pid_delta_init(&buck->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    buck->vref = DEFAULT_VREF;
    buck->duty = 0.5f;
    buck->iref_limit = DEFAULT_IREF_MAX;
}

void buck_single_pid_delta_init(BuckCascadedPID *buck,
                                 float v_threshold,
                                 float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&buck->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    buck->outer.kp = v_threshold;
    buck->outer.ki = 0.0f;
    buck->outer.kd = 0.0f;
    buck->duty = 0.5f;
}

void boost_single_pid_delta_init(BuckCascadedPID *boost,
                                  float v_threshold,
                                  float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&boost->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    boost->outer.kp = v_threshold;
    boost->outer.ki = 0.0f;
    boost->outer.kd = 0.0f;
    boost->duty = 0.5f;
}


float pid_delta_step(PID_Controller *pid, float error)
{
    float p, d;

    // i
    pid->integral = pid->ki * error;
    // clamp
    if (pid->integral > pid->integral_limit)
        pid->integral = pid->integral_limit;
    if (pid->integral < -pid->integral_limit)
        pid->integral = -pid->integral_limit;
    // p
    p = pid->kp * (error - pid->prev_error[0]);
    // d

    d = pid->kd * (error -( 2*pid->prev_error[0]) +pid->prev_error[1]);

    float out = p + pid->integral + d;

    if (pid->output_limit > pid->output_limit)
        pid->output_limit = pid->output_limit;
    else if (pid->output_limit < -pid->output_limit)
        pid->output_limit = -pid->output_limit;

    pid->prev_error[1] = pid->prev_error[0];
    pid->prev_error[0] = error;
    
    pid->output_cache += out; /* store output for potential debugging */
    return pid->output_cache;
}

#define BUCK_OUTER_UPDATE_RATE 10 /* update outer loop every 10 calls */
static int buckOuterCounter = BUCK_OUTER_UPDATE_RATE;


float buck_pid_delta_update(BuckCascadedPID *buck,
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
    if (buckOuterCounter >= BUCK_OUTER_UPDATE_RATE)
    {
        iref = pid_delta_step(&buck->outer, v_err);
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
    buck->duty = pid_delta_step(&buck->inner, i_err);

    /* clamp duty */
    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;
    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}

float buck_single_pid_delta_update(BuckCascadedPID *buck,float i_ref
                                 , float vsense_raw, float isense_raw)
{
    float i_in, v_in, i_err;

    v_in = vsense_raw / VOLTAGE_GAIN;
    buck->outer.ki = v_in;

    if (v_in >= buck->outer.kp) {
        buck->outer.kd = 999.0f;
        buck->duty = 0.0f;
        return buck->duty;
    }
    //buck->outer.kd = 0.0f;

    i_in = isense_raw / CURRENT_GAIN;
    i_err = i_ref - i_in;
    buck->duty = pid_delta_step(&buck->inner, i_err);

    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;
    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}
                                  
float boost_single_pid_delta_update(BuckCascadedPID *boost,float i_ref
                                 , float vsense_raw, float isense_raw)
{
    float i_in, v_in, i_err;

    v_in = vsense_raw / VOLTAGE_GAIN;
    boost->outer.ki = v_in;

    if (v_in >= boost->outer.kp) {
        boost->outer.kd = 999.0f;
        boost->duty = 0.0f;
        return boost->duty;
    }
    //boost->outer.kd = 0.0f;

    i_in = isense_raw / CURRENT_GAIN;
    i_err = i_ref - i_in;
    boost->duty = pid_delta_step(&boost->inner, i_err);

    if (boost->duty > DEFAULT_DUTY_MAX)
        boost->duty = DEFAULT_DUTY_MAX;
    if (boost->duty < DEFAULT_DUTY_MIN)
        boost->duty = DEFAULT_DUTY_MIN;

    return boost->duty;
}