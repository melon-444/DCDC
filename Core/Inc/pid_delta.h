
#ifndef PID_DELTA_H
#define PID_DELTA_H

typedef enum {
    PID_STATE_IDLE = 0,
    PID_STATE_CC,
    PID_STATE_CV,
    PID_STATE_STOP
} pid_state_t;

#ifndef PID_STRUCT
#define PID_STRUCT
typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error[2]; /* store current and previous error for delta calculation */
    float integral_limit;
    float output_limit;
    float output_cache;/*optimal store last output*/
} PID_Controller;

typedef struct {
    PID_Controller inner;   /* current loop */
    PID_Controller outer;   /* voltage loop */
    float vref;             /* target voltage [V] (initial default only) */
    float duty;             /* PWM duty cycle [0.0 ~ 1.0] */
    float iref_limit;       /* inner reference clamp */
    pid_state_t state;      /* current PID state */
} CascadedPID;
#endif /*PID_STRUCT*/

/* -------- current / voltage scaling constants -------- */
#define CURRENT_GAIN        0.40f     /* Uiout = 0.40 * Iin  [V/A] */
#define VOLTAGE_GAIN        (0.80f / 41.0f) /* Vout = gain * Vin */

/* -------- defaults -------- */
#define DEFAULT_VREF        12.0f
#define DEFAULT_IREF_MAX    5.0f
#define DEFAULT_DUTY_MIN    0.0f
#define DEFAULT_DUTY_MAX    0.35f

/* ---------- initialisation ---------- */
void pid_delta_init(PID_Controller *pid,
                            float kp, float ki, float kd,
                            float ilim, float olim);

void buck_pid_delta_init(CascadedPID *buck,
                                 float v_kp, float v_ki, float v_kd,
                                 float i_kp, float i_ki, float i_kd);

void buck_single_pid_delta_init(CascadedPID *buck,
                                 float v_threshold,
                                 float i_kp, float i_ki, float i_kd);

void boost_single_pid_delta_init(CascadedPID *boost,
                                 float v_threshold,
                                 float i_kp, float i_ki, float i_kd);

/* ---------- PID step ---------- */
float pid_delta_step(PID_Controller *pid, float error);


/* ---------- core control routine (call every Ts) ---------- */
/* vref is passed as a parameter for dynamic voltage tracking */
float buck_pid_delta_update(CascadedPID *buck,float v_ref,/* target voltage [V] */float vsense_raw,/* ADC result [V] */float isense_raw);

float buck_single_pid_delta_update(CascadedPID *buck,float i_ref
                                 , float vsense_raw, float isense_raw);
                                 
float boost_single_pid_delta_update(CascadedPID *boost,float i_ref
                                 , float vsense_raw, float isense_raw);
/* ADC result [V] */

float buck_pid_current_update(CascadedPID *buck,
                              float i_ref,
                              float isense);


/* ---------- convenience helpers ---------- */

#ifndef PID_D_INLINE
#define PID_D_INLINE
inline void buck_set_vref(CascadedPID *buck, float vref)
{
    buck->vref = vref;
}


inline float buck_get_duty(const CascadedPID *buck)
{
    return buck->duty;
}

inline int single_pid_should_stop(const CascadedPID *pid)
{
    return pid ->state == PID_STATE_STOP;
}
#endif /*PID_D_INLINE*/
#endif /* PID_DELTA_H */
