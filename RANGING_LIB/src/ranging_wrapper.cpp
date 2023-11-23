#include "ranging_wrapper.h"
#include <math.h>

volatile bool sx1280_lora_ranging = false; // can't be moved inside class. Shows if lora is actively doing ranging
void sx1280_ranging_end(void)
{
    sx1280_lora_ranging = false;
}
Ranging_Wrapper::Position Ranging_Wrapper::Position_Local::to_geodetic()
{
    Position result;
    result.lat = asin(this->z / _earth_radius);
    result.lng = atan2(this->y, this->x);
    result.height = this->length() - _earth_radius;
    return result;
}
double Ranging_Wrapper::Position_Local::length()
{
    return sqrt(pow(this->x, 2) + pow(this->x, 2) + pow(this->x, 2));
}
Ranging_Wrapper::Position_Local Ranging_Wrapper::Position_Local::cross(Position_Local other)
{
    Position_Local result;
    result.x = (this->y * other.z) - (this->z * other.y);
    result.y = (this->z * other.x) - (this->x * other.z);
    result.z = (this->x * other.y) - (this->y * other.x);
    return result;
}

Ranging_Wrapper::Position_Local Ranging_Wrapper::Position_Local::operator-(Position_Local const &other)
{
    Position_Local result;
    result.x = this->x - other.x;
    result.y = this->y - other.y;
    result.z = this->z - other.z;
    return result;
}
Ranging_Wrapper::Position_Local Ranging_Wrapper::Position_Local::operator+(Position_Local const &other)
{
    Position_Local result;
    result.x = this->x + other.x;
    result.y = this->y + other.y;
    result.z = this->z + other.z;
    return result;
}

Ranging_Wrapper::Position_Local Ranging_Wrapper::Position_Local::operator*(double const &other)
{
    Position_Local result;
    result.x = this->x * other;
    result.y = this->y * other;
    result.z = this->z * other;
    return result;
}
// However, for ranging operation, the use of SFII and SF12 is not permitted. Similarly, the bandwidth configuration for ranging operations is restricted to the values 406.25 kHz. 812.5 kHz and 1625 kHz
String Ranging_Wrapper::begin_lora(Mode mode, Lora_Device config)
{
    String status;
    int state = _lora.begin();
    if (state != RADIOLIB_ERR_NONE)
    {
        status += " SX1280 error (" + String(state) + ")";
    }
    else
    {
        // setting paramaters
        _lora.setOutputPower(_config.TXPOWER);
        _lora.setSpreadingFactor(_config.SPREADING);
        _lora.setCodingRate(_config.CODING_RATE);
        _lora.setBandwidth(_config.SIGNAL_BW);
        _lora.setSyncWord(_config.SYNC_WORD);
        _lora.setFrequency(_config.FREQUENCY);
        _lora_initialized = true;
    }

    return status;
}
String Ranging_Wrapper::init(Mode mode, Lora_Device config)
{
    _lora_initialized = false;
    _lora = new Module(config.CS, config.DIO1, config.RESET, config.DIO0, *config.SPI);
    _mode = mode;
    _config = config;

    return begin_lora(mode, config);
}
// will return result from previous slave and start ranging on current slave
bool Ranging_Wrapper::master_read(Ranging_Slave slave, Ranging_Result &result, long int timeout)
{
    bool slave_done = false; // either timeout or result read
    if (_mode != Mode::MASTER || !_lora_initialized)
    {
        return slave_done;
    }
    if (sx1280_lora_ranging)
    {
        // check if should timeout
        if (millis() >= _ranging_start_time + timeout)
        {
            sx1280_lora_ranging = false;
            _lora_range_state = RADIOLIB_ERR_RANGING_TIMEOUT;
            _lora.clearDio1Action();
            _lora.finishTransmit();
        }
    }
    if (!sx1280_lora_ranging)
    {

        // if available read result
        if (_lora_range_state == RADIOLIB_ERR_NONE)
        {
            result.distance = _lora.getRangingResult();
            result.distance = _lora.get
            result.time = millis();
            result.rssi = _lora.getRSSI();
            result.snr = _lora.getSNR();
            result.f_error = _lora.getFrequencyError();
            // Serial.print("Good: " + String(result.distance) + " Addr: ");
            // Serial.println(slave.address, HEX);
        }
        else
        {
            result = {0, 0, 0, 0, 0};
            // Serial.print("Ranging failed: " + String(_lora_range_state) + " Addr: ");
            // Serial.println(slave.address, HEX);
        }
        slave_done = true;
        // clean up
        _lora.clearDio1Action();
        _lora.finishTransmit();
        _lora_range_state = -1;

        // start ranging but first reset lora by calling begin lora again
        begin_lora(_mode, _config);

        // setup interrupt
        _lora.setDio1Action(sx1280_ranging_end);
        sx1280_lora_ranging = true;
        _lora_range_state = _lora.startRanging(true, slave.address);
        _ranging_start_time = millis();
        if (_lora_range_state != RADIOLIB_ERR_NONE)
        {
            Serial.println("Ranging error: " + String(_lora_range_state));
        }
    }
    return slave_done;
}
bool Ranging_Wrapper::slave_reenable(long int timeout, Ranging_Slave slave)
{
    if (_mode != Mode::SLAVE || !_lora_initialized)
    {
        return false;
    }
    bool result = false;
    if (sx1280_lora_ranging)
    {
        // check if should timeout
        if (millis() >= _ranging_start_time + timeout)
        {
            sx1280_lora_ranging = false;
            _lora_range_state = RADIOLIB_ERR_RANGING_TIMEOUT;
            _lora.clearDio1Action();
            _lora.finishTransmit();
        }
        result = false;
    }
    if (!sx1280_lora_ranging)
    {

        // if available read result
        if (_lora_range_state == RADIOLIB_ERR_NONE)
        {
            result = true;
        }
        // clean up
        _lora.clearDio1Action();
        _lora.finishTransmit();
        _lora_range_state = -1;

        // start ranging but first reset lora
        _lora.reset();
        begin_lora(_mode, _config);

        // setup interrupt
        _lora.setDio1Action(sx1280_ranging_end);
        sx1280_lora_ranging = true;
        _lora_range_state = _lora.startRanging(false, slave.address);
        _ranging_start_time = millis();
        if (_lora_range_state != RADIOLIB_ERR_NONE)
        {
            Serial.println("Ranging error");
            result = false;
        }
    }
    return result;
}

Ranging_Wrapper::Position Ranging_Wrapper::local_point_to_global_space(Position_Local movable_point, Position p[3], Position_Local p_local[3])
{

    // convert fixed global points to cartesian
    Position_Local p_cartesian[3];
    p_cartesian[0] = p[0].to_absolute_cartesian();
    p_cartesian[1] = p[1].to_absolute_cartesian();
    p_cartesian[2] = p[2].to_absolute_cartesian();

    Position_Local result_local;
    // move the global point along the local points x axis
    Position_Local d_1_0 = p_cartesian[1] - p_cartesian[0];
    double d_1_0_scale = movable_point.x / d_1_0.length();
    Position_Local x_offset = d_1_0 * d_1_0_scale;

    // move the global point along the local y axis
    // first make a point that will make a perpendicular movment along local y axis when moving from the new point torwards p2
    Position_Local d_1_0_mid = p_cartesian[1] - p_cartesian[0];
    double d_1_0_mid_scale = p_local[2].x / p_local[1].x;
    Position_Local y_offset = d_1_0_mid * d_1_0_mid_scale;

    // use this point to move torwards p2
    Position_Local d_mid_2 = p_cartesian[2] - d_1_0_mid;
    double d_mid_2_scale = movable_point.y / p_local[2].y;
    d_mid_2 = d_mid_2 * d_mid_2_scale;

    // calculate cross product and move torwords the Z direction (check that the cross product has the right sequance by checking which turns out in the right direction)
    Position_Local d_2_0 = p_cartesian[2] - p_cartesian[0];

    Position_Local z_normalied;
    if ((d_1_0.cross(d_2_0)).to_geodetic().height > 0)
    {
        // we can use this cross product
        z_normalied = d_1_0.cross(d_2_0);
    }
    else
    {
        // use other cross product
        z_normalied = d_2_0.cross(d_1_0);
    }
    // divide to normalize
    z_normalied = z_normalied * pow(z_normalied.length(), -1);
    // to get the z offset multiply normalized vector by the local space z value
    Position_Local z_offset = z_normalied * p_local->z;

    // apply offsets
    Position_Local result = p_cartesian[0] + x_offset + y_offset + z_offset;
    return result.to_geodetic();
}

// the x-axis goes through long,lat (0,0), so longitude 0 meets the equator;
// the y - axis goes through(0, 90);
// and the z - axis goes through the poles
Ranging_Wrapper::Position_Local Ranging_Wrapper::Position::to_absolute_cartesian()
{
    Position_Local result;
    double lat = this->lat * DEG_TO_RAD;
    double lng = this->lng * DEG_TO_RAD;

    result.x = (_earth_radius + this->height) * cos(lat) * sin(lng);
    result.y = (_earth_radius + this->height) * cos(lat) * sin(lng);
    result.z = (_earth_radius + this->height) * sin(lat);

    return result;
}

double Ranging_Wrapper::distance_between_earth_cordinates_m(Position p1, Position p2)
{
    Position_Local d_pos = (p1.to_absolute_cartesian()) - (p2.to_absolute_cartesian());
    double result = d_pos.length();
    return result;
}

bool Ranging_Wrapper::trilaterate_position(Ranging_Result readings[3], Ranging_Slave slaves[3], Position &result)
{
    // becuase not finished yet return false;
    return false;

    Position global_bs_pos[3];
    global_bs_pos[0] = slaves[0].position;
    global_bs_pos[1] = slaves[1].position;
    global_bs_pos[2] = slaves[2].position;
    /// global space to local space
    Position_Local local_bs_pos[3];
    // lets set the 0, 0, 0 as the first index and base everything of that
    local_bs_pos[0] = {0, 0, 0};
    double distance1_2 = distance_between_earth_cordinates_m(slaves[0].position, slaves[1].position);
    double distance1_3 = distance_between_earth_cordinates_m(slaves[0].position, slaves[2].position);
    double distance2_3 = distance_between_earth_cordinates_m(slaves[1].position, slaves[2].position);
    local_bs_pos[1] = {distance1_2, 0, 0};
    // calculate the third point
    local_bs_pos[2].y = (pow(distance1_2, 2) + pow(distance1_3, 2) - pow(distance2_3, 2)) / (2 * distance1_2); // add check if its trying to devide by 0
    local_bs_pos[2].x = sqrt(pow(distance1_3, 2) - pow(local_bs_pos[2].y, 2));
    local_bs_pos[2].z = 0;

    /// TODO check if data is usable if not return false;
    // is reading too small?, is reading too large?

    /// calculate pos in local space    https://en.wikipedia.org/wiki/True-range_multilateration#Three_Cartesian_dimensions,_three_measured_slant_ranges
    Position_Local calculated_local_pos;
    calculated_local_pos.x = (pow(readings[0].distance, 2) - pow(readings[1].distance, 2)) / (2 * local_bs_pos[2].x);
    calculated_local_pos.y = (pow(readings[0].distance, 2) - pow(readings[2].distance, 2) + pow(local_bs_pos[2].x, 2), pow(local_bs_pos[2].y, 2) - (2 * local_bs_pos[2].x * calculated_local_pos.x)) / (2 * local_bs_pos[2].y);
    calculated_local_pos.z = sqrt(pow(readings[1].distance, 2) - pow(calculated_local_pos.x, 2) - pow(calculated_local_pos.y, 2)); // this can be either positive or negative

    /// local space to global space
    Position calculated_global_pos = local_point_to_global_space(calculated_local_pos, global_bs_pos, local_bs_pos);
    /// set results
    result = calculated_global_pos;
    return true;
}

bool Ranging_Wrapper::get_init_status()
{
    return _lora_initialized;
}
