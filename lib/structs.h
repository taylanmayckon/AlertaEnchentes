typedef struct {
    uint16_t vrx_value;
    uint16_t vry_value;
} Joy;

typedef struct {
    float water_level;
    float rain_volume;
} Sensors;

typedef struct{
    bool normal_mode;
    bool alert_water_level;
    bool alert_rain_volume;
} Alerts;