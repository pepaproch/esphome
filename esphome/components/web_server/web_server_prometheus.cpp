#include "web_server.h"
#include "web_server_prometheus.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/util.h"
#include "esphome/components/json/json_util.h"

#include "StreamString.h"

#include <cstdlib>

#ifdef USE_LOGGER
#include <esphome/components/logger/logger.h>
#endif

namespace esphome {
namespace web_server {

void WebServerPrometheus::handle_request(AsyncWebServerRequest *request) {
  AsyncResponseStream *stream = request->beginResponseStream("text/plain");

#ifdef USE_SENSOR
  this->sensor_type_(stream);
  for (auto *obj : App.get_sensors())
    this->sensor_row_(stream, obj);
#endif

#ifdef USE_BINARY_SENSOR
  this->binary_sensor_type_(stream);
  for (auto *obj : App.get_binary_sensors())
    this->binary_sensor_row_(stream, obj);
#endif

#ifdef USE_FAN
  this->fan_type_(stream);
  for (auto *obj : App.get_fans())
    this->fan_row_(stream, obj);
#endif

#ifdef USE_LIGHT
  this->light_type_(stream);
  for (auto *obj : App.get_lights())
    this->light_row_(stream, obj);
#endif

#ifdef USE_COVER
  this->cover_type_(stream);
  for (auto *obj : App.get_covers())
    this->cover_row_(stream, obj);
#endif

#ifdef USE_SWITCH
  this->switch_type_(stream);
  for (auto *obj : App.get_switches())
    this->switch_row_(stream, obj);
#endif

  request->send(stream);
}

// Type-specific implementation
#ifdef USE_SENSOR
void WebServerPrometheus::sensor_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_sensor_value GAUGE\n"));
  stream->print(F("#TYPE esphome_sensor_failed GAUGE\n"));
}
void WebServerPrometheus::sensor_row_(AsyncResponseStream *stream, sensor::Sensor *obj) {
  if (obj->is_internal())
    return;
  if (!isnan(obj->state)) {
    // We have a valid value, output this value
    stream->print(F("esphome_sensor_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 0\n"));
    // Data itself
    stream->print(F("esphome_sensor_value{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\",unit=\""));
    stream->print(obj->get_unit_of_measurement().c_str());
    stream->print(F("\"} "));
    stream->print(value_accuracy_to_string(obj->state, obj->get_accuracy_decimals()).c_str());
    stream->print('\n');
  } else {
    // Invalid state
    stream->print(F("esphome_sensor_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

// Type-specific implementation
#ifdef USE_BINARY_SENSOR
void WebServerPrometheus::binary_sensor_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_binary_sensor_value GAUGE\n"));
  stream->print(F("#TYPE esphome_binary_sensor_failed GAUGE\n"));
}
void WebServerPrometheus::binary_sensor_row_(AsyncResponseStream *stream, binary_sensor::BinarySensor *obj) {
  if (obj->is_internal())
    return;
  if (!isnan(obj->state)) {
    // We have a valid value, output this value
    stream->print(F("esphome_binary_sensor_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 0\n"));
    // Data itself
    stream->print(F("esphome_binary_sensor_value{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} "));
    stream->print(obj->state);
    stream->print('\n');
  } else {
    // Invalid state
    stream->print(F("esphome_binary_sensor_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

#ifdef USE_FAN
void WebServerPrometheus::fan_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_fan_value GAUGE\n"));
  stream->print(F("#TYPE esphome_fan_failed GAUGE\n"));
  stream->print(F("#TYPE esphome_fan_speed GAUGE\n"));
  stream->print(F("#TYPE esphome_fan_oscillation GAUGE\n"));
}
void WebServerPrometheus::fan_row_(AsyncResponseStream *stream, fan::FanState *obj) {
  if (obj->is_internal())
    return;
  if (!isnan(obj->state)) {
    // We have a valid value, output this value
    stream->print(F("esphome_fan_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 0\n"));
    // Data itself
    stream->print(F("esphome_fan_value{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} "));
    stream->print(obj->state);
    stream->print('\n');
    // Speed if available
    if (obj->get_traits().supports_speed()) {
      stream->print(F("esphome_fan_speed{id=\""));
      stream->print(obj->get_object_id().c_str());
      stream->print(F("\",name=\""));
      stream->print(obj->get_name().c_str());
      stream->print(F("\"} "));
      stream->print(obj->speed);
      stream->print('\n');
    }
    // Oscillation if available
    if (obj->get_traits().supports_oscillation()) {
      stream->print(F("esphome_fan_oscillation{id=\""));
      stream->print(obj->get_object_id().c_str());
      stream->print(F("\",name=\""));
      stream->print(obj->get_name().c_str());
      stream->print(F("\"} "));
      stream->print(obj->oscillating);
      stream->print('\n');
    }
  } else {
    // Invalid state
    stream->print(F("esphome_fan_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

#ifdef USE_LIGHT
void WebServerPrometheus::light_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_light_state GAUGE\n"));
  stream->print(F("#TYPE esphome_light_color GAUGE\n"));
  stream->print(F("#TYPE esphome_light_effect_active GAUGE\n"));
}
void WebServerPrometheus::light_row_(AsyncResponseStream *stream, light::LightState *obj) {
  if (obj->is_internal())
    return;
  // State
  stream->print(F("esphome_light_state{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\"} "));
  stream->print(obj->remote_values.is_on());
  stream->print(F("\n"));
  // Brightness and RGBW
  light::LightColorValues color = obj->current_values;
  float brightness, r, g, b, w;
  color.as_brightness(&brightness);
  color.as_rgbw(&r, &g, &b, &w);
  stream->print(F("esphome_light_color{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\",channel=\"brightness\"} "));
  stream->print(brightness);
  stream->print(F("\n"));
  stream->print(F("esphome_light_color{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\",channel=\"r\"} "));
  stream->print(r);
  stream->print(F("\n"));
  stream->print(F("esphome_light_color{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\",channel=\"g\"} "));
  stream->print(g);
  stream->print(F("\n"));
  stream->print(F("esphome_light_color{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\",channel=\"b\"} "));
  stream->print(b);
  stream->print(F("\n"));
  stream->print(F("esphome_light_color{id=\""));
  stream->print(obj->get_object_id().c_str());
  stream->print(F("\",name=\""));
  stream->print(obj->get_name().c_str());
  stream->print(F("\",channel=\"w\"} "));
  stream->print(w);
  stream->print(F("\n"));
  // Effect
  std::string effect = obj->get_effect_name();
  if (effect == "None") {
    stream->print(F("esphome_light_effect_active{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\",effect=\"None\"} 0\n"));
  } else {
    stream->print(F("esphome_light_effect_active{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\",effect=\""));
    stream->print(effect.c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

#ifdef USE_COVER
void WebServerPrometheus::cover_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_cover_value GAUGE\n"));
  stream->print(F("#TYPE esphome_cover_failed GAUGE\n"));
}
void WebServerPrometheus::cover_row_(AsyncResponseStream *stream, cover::Cover *obj) {
  if (obj->is_internal())
    return;
  if (!isnan(obj->position)) {
    // We have a valid value, output this value
    stream->print(F("esphome_cover_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 0\n"));
    // Data itself
    stream->print(F("esphome_cover_value{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} "));
    stream->print(obj->position);
    stream->print('\n');
    if (obj->get_traits().get_supports_tilt()) {
      stream->print(F("esphome_cover_tilt{id=\""));
      stream->print(obj->get_object_id().c_str());
      stream->print(F("\",name=\""));
      stream->print(obj->get_name().c_str());
      stream->print(F("\"} "));
      stream->print(obj->tilt);
      stream->print('\n');
    }
  } else {
    // Invalid state
    stream->print(F("esphome_cover_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

#ifdef USE_SWITCH
void WebServerPrometheus::switch_type_(AsyncResponseStream *stream) {
  stream->print(F("#TYPE esphome_switch_value GAUGE\n"));
  stream->print(F("#TYPE esphome_switch_failed GAUGE\n"));
}
void WebServerPrometheus::switch_row_(AsyncResponseStream *stream, switch_::Switch *obj) {
  if (obj->is_internal())
    return;
  if (!isnan(obj->state)) {
    // We have a valid value, output this value
    stream->print(F("esphome_switch_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 0\n"));
    // Data itself
    stream->print(F("esphome_switch_value{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} "));
    stream->print(obj->state);
    stream->print('\n');
  } else {
    // Invalid state
    stream->print(F("esphome_switch_failed{id=\""));
    stream->print(obj->get_object_id().c_str());
    stream->print(F("\",name=\""));
    stream->print(obj->get_name().c_str());
    stream->print(F("\"} 1\n"));
  }
}
#endif

}  // namespace web_server
}  // namespace esphome