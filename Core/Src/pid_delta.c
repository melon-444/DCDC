
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

void buck_pid_delta_init(CascadedPID *buck,
                         float v_kp, float v_ki, float v_kd,
                         float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&buck->outer, v_kp, v_ki, v_kd, 5.0f, DEFAULT_IREF_MAX);
    pid_delta_init(&buck->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    buck->vref = DEFAULT_VREF;
    buck->duty = 0.5f;
    buck->iref_limit = DEFAULT_IREF_MAX;
    buck->state = PID_STATE_IDLE;
}

void buck_single_pid_delta_init(CascadedPID *buck,
                                float v_threshold,
                                float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&buck->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    buck->outer.kp = v_threshold;
    buck->outer.ki = 0.0f;
    buck->outer.kd = 0.0f;
    buck->duty = 0.0f;
    buck->state = PID_STATE_IDLE;
    buck->inner.output_cache = 0.0f; /* ensure output cache starts at 0 for single PID */
}

void boost_single_pid_delta_init(CascadedPID *boost,
                                 float v_threshold,
                                 float i_kp, float i_ki, float i_kd)
{
    pid_delta_init(&boost->inner, i_kp, i_ki, i_kd, 10.0f, DEFAULT_DUTY_MAX);
    boost->outer.kp = v_threshold;
    boost->outer.ki = 0.0f;
    boost->outer.kd = 0.0f;
    boost->duty = 0.0f;
    boost->state = PID_STATE_IDLE;
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
    d = pid->kd * (error - (2 * pid->prev_error[0]) + pid->prev_error[1]);

    float out = p + pid->integral + d;

    pid->prev_error[1] = pid->prev_error[0];
    pid->prev_error[0] = error;

    pid->output_cache += out;

    if (pid->output_cache > pid->output_limit)
        pid->output_cache = pid->output_limit;
    else if (pid->output_cache < -pid->output_limit)
        pid->output_cache = -pid->output_limit;

    return pid->output_cache;
}

#define BUCK_OUTER_UPDATE_RATE 10 /* update outer loop every 10 calls */
static int buckOuterCounter = BUCK_OUTER_UPDATE_RATE;

float buck_pid_current_update(CascadedPID *buck,
                              float i_ref,
                              float isense)
{
    float i_in;
    float i_err;

    /* ADC scaling */
    i_in = isense;

    /* current loop error */
    i_err = i_ref - i_in;

    /* PID output */
    buck->duty = pid_delta_step(&buck->inner, i_err);

    /* clamp duty */
    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;

    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}

float buck_pid_delta_update(CascadedPID *buck,
                            float v_ref,
                            float vsense, /* ADC result [V] */
                            float isense)
{
    float v_in, i_in;
    float v_err, iref, i_err;

    /* scale ADC readings back to physical units */
    v_in = vsense;
    i_in = isense;

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

float buck_single_pid_delta_update(CascadedPID *buck, float i_ref, float vsense, float isense)
{
    float i_err;

    if (vsense >= buck->outer.kp)
    {
        if (buck->duty > 0.02f)
        {
            buck->duty = buck->duty - 0.02f;
            return buck->duty;
        }
        else
        {
            buck->state = PID_STATE_STOP;
            buck->duty = 0.0f;
            return buck->duty;
        }
    }

    buck->state = PID_STATE_CC;
    buck->outer.kd = 0.0f;

    i_err = i_ref - isense;
    buck->duty = pid_delta_step(&buck->inner, i_err);

    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;
    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}

float buck_single_pid_delta_hard_update(CascadedPID *buck, float i_ref, float vsense, float isense)
{
    float i_err;

    /* 硬过压 / 过流保护 */
    if ((vsense >= 4.30f) || (isense >= 0.80f))
    {
        buck->state = PID_STATE_STOP;
        buck->duty = 0.0f;
        buck->inner.output_cache = 0.0f;
        buck->inner.prev_error[0] = 0.0f;
        buck->inner.prev_error[1] = 0.0f;
        return 0.0f;
    }

    /* 进入 4.2V 限压区后，不允许继续做恒流增占空比 */
    if (vsense >= buck->outer.kp)
    {
        buck->state = PID_STATE_CV;

        if (buck->duty > 0.002f)
        {
            buck->duty -= 0.002f;
        }
        else
        {
            buck->duty = 0.0f;
        }

        buck->inner.output_cache = buck->duty;
        return buck->duty;
    }

    /* 4.2V 以下才做恒流控制 */
    buck->state = PID_STATE_CC;

    i_err = i_ref - isense;
    buck->duty = pid_delta_step(&buck->inner, i_err);

    if (buck->duty > DEFAULT_DUTY_MAX)
        buck->duty = DEFAULT_DUTY_MAX;

    if (buck->duty < DEFAULT_DUTY_MIN)
        buck->duty = DEFAULT_DUTY_MIN;

    return buck->duty;
}

float boost_single_pid_delta_update(CascadedPID *boost, float i_ref, float vsense_raw, float isense_raw)
{
    float i_in, v_in, i_err;

    v_in = vsense_raw / VOLTAGE_GAIN;
    boost->outer.ki = v_in;

    if (v_in >= boost->outer.kp)
    {
        boost->outer.kd = 999.0f;
        boost->duty = 0.0f;
        return boost->duty;
    }
    // boost->outer.kd = 0.0f;

    i_in = isense_raw / CURRENT_GAIN;
    i_err = i_ref - i_in;
    boost->duty = pid_delta_step(&boost->inner, i_err);

    if (boost->duty > DEFAULT_DUTY_MAX)
        boost->duty = DEFAULT_DUTY_MAX;
    if (boost->duty < DEFAULT_DUTY_MIN)
        boost->duty = DEFAULT_DUTY_MIN;

    return boost->duty;
}

void cc_pid_init(CC_PID *pid, float kp, float ki, float kd,
                 float i_limit, float dout_limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->prev_error[0] = 0.0f;
    pid->prev_error[1] = 0.0f;
    pid->i_limit = i_limit;
    pid->dout_limit = dout_limit;
}

float cc_pid_step(CC_PID *pid, float i_ref, float i_sense)
{
    float p, i, d;
    float error = i_ref - i_sense;

    /* I increment with saturation */
    i = pid->ki * error;
    if (i > pid->i_limit)
        i = pid->i_limit;
    if (i < -pid->i_limit)
        i = -pid->i_limit;

    /* P increment */
    p = pid->kp * (error - pid->prev_error[0]);

    /* D increment */
    d = pid->kd * (error - 2.0f * pid->prev_error[0] + pid->prev_error[1]);

    /* update error history */
    pid->prev_error[1] = pid->prev_error[0];
    pid->prev_error[0] = error;

    /* total delta */
    float dout = p + i + d;

    /* delta output saturation */
    if (dout > pid->dout_limit)
        dout = pid->dout_limit;
    if (dout < -pid->dout_limit)
        dout = -pid->dout_limit;

    return dout;
}