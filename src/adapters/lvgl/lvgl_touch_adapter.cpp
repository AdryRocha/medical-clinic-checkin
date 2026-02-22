#include "lvgl_touch_adapter.hpp"

LVGLTouchAdapter::LVGLTouchAdapter(TouchInterface* touch,
                                   uint16_t display_width,
                                   uint16_t display_height,
                                   uint8_t rotation)
    : touch_(touch),
      display_width_(display_width),
      display_height_(display_height),
      rotation_(rotation % 4),
      offset_x_(0),
      offset_y_(0),
      indev_(nullptr) {

    last_point_.x = 0;
    last_point_.y = 0;
    last_point_.event = 0;
    last_point_.valid = false;
}

void LVGLTouchAdapter::transformCoordinates(const TouchPoint& point,
                                            int16_t& x_out,
                                            int16_t& y_out) {
    // Rotaciona de acordo com a orientação configurada
    switch (rotation_) {
        case 0:  // Portrait (0°) - no transformation needed
            x_out = point.x;
            y_out = point.y;
            break;
            
        case 1:  // Landscape (90° clockwise)
            // x_display = y_touch
            // y_display = height - x_touch
            x_out = point.y;
            y_out = display_height_ - point.x;
            break;
            
        case 2:  // Portrait inverted (180°)
            // x_display = width - x_touch
            // y_display = height - y_touch
            x_out = display_width_ - point.x;
            y_out = display_height_ - point.y;
            break;
            
        case 3:  // Landscape inverted (270° clockwise)
            // x_display = width - y_touch
            // y_display = x_touch
            x_out = display_width_ - point.y;
            y_out = point.x;
            break;
            
        default:
            x_out = point.x;
            y_out = point.y;
            break;
    }
}

void LVGLTouchAdapter::readCallback(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    LVGLTouchAdapter* adapter = (LVGLTouchAdapter*)drv->user_data;
    
    if (adapter->touch_ != nullptr) {
        TouchPoint point;
        if (adapter->touch_->readTouch(&point) && point.valid) {
            int16_t x_transformed, y_transformed;
            adapter->transformCoordinates(point, x_transformed, y_transformed);
            
            data->point.x = x_transformed;
            data->point.y = y_transformed;
            data->state = LV_INDEV_STATE_PR;  // Pressed
            
            adapter->last_point_.x = x_transformed;
            adapter->last_point_.y = y_transformed;
            adapter->last_point_.valid = true;
        } else {
            data->point.x = adapter->last_point_.x;
            data->point.y = adapter->last_point_.y;
            data->state = LV_INDEV_STATE_REL;  // Released
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

lv_indev_t* LVGLTouchAdapter::registerInputDevice() {
    lv_indev_drv_init(&indev_drv_);
    indev_drv_.type = LV_INDEV_TYPE_POINTER;
    indev_drv_.read_cb = readCallback;
    indev_drv_.user_data = this;
    
    indev_ = lv_indev_drv_register(&indev_drv_);
    return indev_;
}

void LVGLTouchAdapter::setRotation(uint8_t rotation) {
    rotation_ = rotation % 4;
}