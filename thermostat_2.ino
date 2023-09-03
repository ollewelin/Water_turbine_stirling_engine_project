
/*
#Temperature (°C) | Resistance (kΩ)
#---------------------------------
#24               | 260
#26               | 245
#27               | 230
#29               | 214
#30               | 200
#34               | 161
#50               | 107
#55               | 72
#60               | 58
#70               | 40
#75               | 35
#80               | 29
#90               | 20.2
#100              | 14.5
#103              | 13.0
#105              | 12.1
#106              | 11.7
#120              | 7.9
#130              | 6.0
#135              | 5.3
#140              | 4.5
#150              | 3.47
#160              | 2.65
#170              | 2.08
#180              | 1.66
#183              | 1.55
#186              | 1.45
#189              | 1.38
#191              | 1.31
#193              | 1.25
#194              | 1.22
#195              | 1.17
#196              | 1.145
#200              | 1.071666667   Extrapolated by -0.073333333 kΩ / °C
*/


#include <math.h>


const int NTC_pin = A0; // Analog pin for NTC temperature sensor

const int numPoints = 32; // Number of data points in the table

const float resistanceTable[numPoints] = {
  260, 245, 230, 214, 200, 161, 107, 72, 58, 40, 35, 29, 20.2, 14.5, 13.0, 12.1, 11.7, 7.9, 6.0, 5.3, 4.5, 3.47, 2.65, 2.08, 1.66, 1.55, 1.45, 1.38, 1.31, 1.25, 1.22, 1.17 };

const float temperatureTable[numPoints] = {
  24, 26, 27, 29, 30, 34, 50, 55, 60, 70, 75, 80, 90, 100, 103, 105, 106, 120, 130, 135, 140, 150,
  160, 170, 180, 183, 186, 189, 191, 193, 194, 195
};
#include <math.h>

#define CONTROLLER_MODE_RESET_PID 0
#define CONTROLLER_MODE_RUN_PID 1
#define CONTROLLER_MODE_PAUSE_PID 2

typedef struct {
    float PID_control_value;
    int cont_mode;
    float integrator;
    float filtered_feedback;
    float prev_filt_feedback;
    float antiwindup_filter;
    float PID_d_filter_constant;
    float PID_res_i_filter_constant;
    float sample_time;
    float d_part;
    float i_part;
    float p_part;
    float cv_before_limit;
    float PID_fb;
    float PID_setp;
    float PID_par_p;
    float PID_par_i;
    float PID_par_d;
    float PID_par_cvu;
    float PID_par_cvl;
    float PID_par_tau_i;
    float PID_par_tau_d;
    float PID_error;
} generic_pid_controller;

generic_pid_controller controller; // Global controller instance

unsigned long previousMillis = 0;
const unsigned long interval = 100; // Interval in milliseconds (1 second)



void setup() {
    // Initialize controller parameters and sample_time here
    controller.PID_par_p = 1.0; // Example value
    controller.PID_par_i = 0.1; // Example value
    controller.PID_par_d = 0.01; // Example value
    controller.sample_time = 0.1; // Example value
    Serial.begin(115200);
}


void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        // It's been 1 second, execute the controller logic
        previousMillis = currentMillis;

          // Read NTC temperature sensor value
  int ntcRawValue = analogRead(NTC_pin);
  // Convert NTC value to resistance
  float ntcResistance = calculateNTCResistance(ntcRawValue);

  Serial.print("R=");
  Serial.print(ntcResistance, 2);
  Serial.print(" Ω ");

  // Convert resistance to temperature in Celsius
  float ntcTemperature = calculateNTCTemperature(ntcResistance);
  Serial.print("T=");
  Serial.print(ntcTemperature, 2);
  Serial.println(" °C");
  controller.cont_mode = CONTROLLER_MODE_RUN_PID;
          
 // Serial.print("controller.sample_time = ");
 // Serial.println(controller.sample_time, 4);

        run1sample();

        // Access controller outputs as needed
        float d_filt_fb = get_d_filt_fb();
        float antiwindup_filt = get_antiwindup_filt();
        float d_part = get_d_part();
        float i_part = get_i_part();
        float p_part = get_p_part();
        float integrator = get_integrator();

        // Your application logic here

        
    }

    // Other non-blocking tasks or code can run here
}

void run1sample() {
    controller.PID_res_i_filter_constant = do_filter_constant(controller.sample_time, controller.PID_par_tau_i);
    controller.PID_d_filter_constant = do_filter_constant(controller.sample_time, controller.PID_par_tau_d);

    controller.integrator = check_and_clear_NaN(controller.integrator);
    controller.filtered_feedback = check_and_clear_NaN(controller.filtered_feedback);
    controller.antiwindup_filter = check_and_clear_NaN(controller.antiwindup_filter);

    switch (controller.cont_mode) {
        case CONTROLLER_MODE_RESET_PID:
            controller.integrator = 0.0;
            controller.filtered_feedback = controller.PID_fb;
            controller.antiwindup_filter = 0.0;
            break;

        case CONTROLLER_MODE_RUN_PID:
            controller.PID_error = controller.PID_setp - controller.PID_fb;
            controller.p_part = controller.PID_error * controller.PID_par_p;

            if (controller.PID_par_i != 0.0) {
                controller.integrator += controller.PID_error;
            }
            controller.i_part = controller.integrator * controller.PID_par_i;

            controller.prev_filt_feedback = controller.filtered_feedback;
            controller.filtered_feedback = (controller.PID_fb - controller.filtered_feedback) * controller.PID_d_filter_constant + controller.filtered_feedback;
            controller.d_part = (controller.prev_filt_feedback - controller.filtered_feedback) * controller.PID_par_d;

            controller.cv_before_limit = controller.p_part + controller.i_part + controller.d_part;

            if (controller.cv_before_limit > controller.PID_par_cvu) {
                controller.PID_control_value = controller.PID_par_cvu;
            }
            else if (controller.cv_before_limit < controller.PID_par_cvl) {
                controller.PID_control_value = controller.PID_par_cvl;
            }
            else {
                controller.PID_control_value = controller.cv_before_limit;
            }

            controller.antiwindup_filter = (controller.PID_control_value - controller.cv_before_limit) * controller.PID_res_i_filter_constant;
            controller.integrator += controller.antiwindup_filter;
            break;

        case CONTROLLER_MODE_PAUSE_PID:
            // Just pause PID calculation, do nothing
            controller.filtered_feedback = controller.PID_fb;
            break;
    }
}


float get_d_filt_fb() {
    return controller.filtered_feedback;
}

float get_antiwindup_filt() {
    return controller.antiwindup_filter;
}

float get_d_part() {
    return controller.d_part;
}

float get_i_part() {
    return controller.i_part;
}

float get_p_part() {
    return controller.p_part;
}

float get_integrator() {
    return controller.integrator;
}

float check_and_clear_NaN(float arg1) {
    if (isnan(arg1)) {
        arg1 = 0.0;
    }
    return arg1;
}

float do_filter_constant(float samp_time, float tau) {
    float filter_constant = 0.0;
    if (tau != 0.0) { // Zero division protection
        filter_constant = samp_time / tau;
    }
    return filter_constant;
}
// Calculate NTC resistance based on analog value
float calculateNTCResistance(int rawValue) {
  float voltage = (float)rawValue * 5.0 / 1023.0;
  float ntcResistance = 4700.0 * voltage / (5.0 - voltage);
  return ntcResistance;
}

// Convert NTC resistance to temperature using interpolation and extrapolation
float calculateNTCTemperature(float resistance) {
  float below_table_res = resistanceTable[0]*1000.0;
  float above_table_res = resistanceTable[numPoints - 1]*1000.0;
    
  if (resistance >= below_table_res) {
    // Extrapolation below 24°C
    //Serial.println(" Extrapolation below 24°C");
    return temperatureTable[0] - (resistanceTable[0]*1000.0 - resistance) / -7.5;
  } else if (resistance <= above_table_res) {
    // Extrapolation above 200°C
    //Serial.println(" Extrapolation above 200°C");
    return temperatureTable[numPoints - 1] - (resistance - resistanceTable[numPoints - 1]*1000.0) / 0.073333333;
  } else {
    // Interpolation within the table
  //  Serial.println(" Interpolation within the table");
    int i;
    for (i = 0; i < numPoints - 1; i++) {

      float res_table_i = resistanceTable[i]*1000;
      float res_table_ip1 = resistanceTable[i + 1]*1000;

      if (resistance >= res_table_ip1 && resistance <= res_table_i) {
        break;
      }
    }
    //Serial.println(" Interpolation within the table");
    float temperatureRange = temperatureTable[i + 1] - temperatureTable[i];
    float resistanceRange = resistanceTable[i + 1]*1000 - resistanceTable[i]*1000;
    float slope = temperatureRange / resistanceRange;

    return temperatureTable[i] + slope * (resistance - resistanceTable[i]*1000);
  }
}
