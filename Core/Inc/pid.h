#ifndef PID_H
#define PID_H

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
} BuckCascadedPID;
#endif /*PID_STRUCT*/

/* -------- current / voltage scaling constants -------- */
#define CURRENT_GAIN        0.40f     /* Uiout = 0.40 * Iin  [V/A] */
#define VOLTAGE_GAIN        (0.80f / 41.0f) /* Vout = gain * Vin */

/* -------- defaults -------- */
#define DEFAULT_VREF        12.0f
#define DEFAULT_IREF_MAX    5.0f
#define DEFAULT_DUTY_MIN    0.0f
#define DEFAULT_DUTY_MAX    0.95f

/* ---------- initialisation ---------- */
void pid_init(PID_Controller *pid,
                            float kp, float ki, float kd,
                            float ilim, float olim);

void buck_pid_init(BuckCascadedPID *buck,
                                 float v_kp, float v_ki, float v_kd,
                                 float i_kp, float i_ki, float i_kd);

void buck_single_pid_init(BuckCascadedPID *buck,
                                 float v_kp, float v_ki, float v_kd,
                                 float i_kp, float i_ki, float i_kd);

void boost_single_pid_init(BuckCascadedPID *boost,
                                 float v_kp, float v_ki, float v_kd,
                                 float i_kp, float i_ki, float i_kd);
/* ---------- PID step ---------- */
float pid_step(PID_Controller *pid, float error);


/* ---------- core control routine (call every Ts) ---------- */
/* vref is passed as a parameter for dynamic voltage tracking */
float buck_pid_update(BuckCascadedPID *buck,float v_ref,/* target voltage [V] */float vsense_raw,/* ADC result [V] */float isense_raw);
/* ADC result [V] */


/* ---------- convenience helpers ---------- */

#ifndef PID_INLINE
#define PID_INLINE
inline void buck_set_vref(BuckCascadedPID *buck, float vref)
{
    buck->vref = vref;
}


inline float buck_get_duty(const BuckCascadedPID *buck)
{
    return buck->duty;
}
#endif /*PID_INLINE*/
#endif /* PID_H */
