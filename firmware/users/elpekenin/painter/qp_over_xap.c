// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO
// - Maybe the XAP answer should be success/failure based on the output from QP functions, returning success for now as in "your request has been handled"

#include "qp.h"
#include "xap.h"

#include "graphics.h"
#include "user_utils.h"

#define DEVICE (qp_devices_pekenin[arg->device_id])
#define FONT   (qp_fonts[arg->font_id])
#define IMAGE  (qp_images[arg->image_id])

// ===========================================================
// Handlers
bool xap_execute_qp_clear(xap_token_t token, xap_route_user_quantum_painter_clear_arg_t* arg) {
    xap_respond_success(token);

    qp_clear(DEVICE);

    return true;
}

bool xap_execute_qp_setpixel(xap_token_t token, xap_route_user_quantum_painter_setpixel_arg_t* arg) {
    xap_respond_success(token);

    qp_setpixel(DEVICE, arg->x, arg->y, arg->hue, arg->sat, arg->val);

    return true;
}

bool xap_execute_qp_line(xap_token_t token, xap_route_user_quantum_painter_draw_line_arg_t* arg) {
    xap_respond_success(token);

    qp_line(DEVICE, arg->x0, arg->y0, arg->x1, arg->y1, arg->hue, arg->sat, arg->val);

    return true;
}

bool xap_execute_qp_rect(xap_token_t token, xap_route_user_quantum_painter_draw_rect_arg_t* arg) {
    xap_respond_success(token);

    qp_rect(DEVICE, arg->left, arg->top, arg->right, arg->bottom, arg->hue, arg->sat, arg->val, arg->filled);

    return true;
}

bool xap_execute_qp_circle(xap_token_t token, xap_route_user_quantum_painter_draw_circle_arg_t* arg) {
    xap_respond_success(token);

    qp_circle(DEVICE, arg->x, arg->y, arg->radius, arg->hue, arg->sat, arg->val, arg->filled);

    return true;
}

bool xap_execute_qp_ellipse(xap_token_t token, xap_route_user_quantum_painter_draw_ellipse_arg_t* arg) {
    xap_respond_success(token);

    qp_ellipse(DEVICE, arg->x, arg->y, arg->sizex, arg->sizey, arg->hue, arg->sat, arg->val, arg->filled);

    return true;
}

bool xap_execute_qp_drawimage(xap_token_t token, xap_route_user_quantum_painter_drawimage_arg_t* arg) {
    xap_respond_success(token);

    qp_drawimage(DEVICE, arg->x, arg->y, IMAGE);

    return true;
}

bool xap_execute_qp_drawimage_recolor(xap_token_t token, xap_route_user_quantum_painter_drawimage_recolor_arg_t* arg) {
    xap_respond_success(token);

    qp_drawimage_recolor(DEVICE, arg->x, arg->y, IMAGE, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);

    return true;
}

bool xap_execute_qp_animate(xap_token_t token, xap_route_user_quantum_painter_animate_arg_t* arg) {
    xap_respond_success(token);

    qp_animate(DEVICE, arg->x, arg->y, IMAGE);

    return true;
}

bool xap_execute_qp_animate_recolor(xap_token_t token, xap_route_user_quantum_painter_animate_recolor_arg_t* arg) {
    xap_respond_success(token);

    qp_animate_recolor(DEVICE, arg->x, arg->y, IMAGE, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);

    return true;
}

bool xap_respond_qp_drawtext(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_drawtext_arg_t  __arg;
    xap_route_user_quantum_painter_drawtext_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    xap_respond_success(token);

    qp_drawtext(DEVICE, arg->x, arg->y, FONT, (const char *)arg->text);

    return true;
}

bool xap_respond_qp_drawtext_recolor(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_drawtext_recolor_arg_t  __arg;
    xap_route_user_quantum_painter_drawtext_recolor_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    xap_respond_success(token);

    qp_drawtext_recolor(DEVICE, arg->x, arg->y, FONT, (const char *)arg->text, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);

    return true;
}

bool xap_execute_qp_get_geometry(xap_token_t token, xap_route_user_quantum_painter_get_geometry_arg_t* arg) {
    uint16_t           width;
    uint16_t           height;
    painter_rotation_t rotation;
    uint16_t           offset_x;
    uint16_t           offset_y;

    qp_get_geometry(DEVICE, &width, &height, &rotation, &offset_x, &offset_y);

    uint8_t ret[9] = {u16_TO_u8(width), u16_TO_u8(height), rotation, u16_TO_u8(offset_x), u16_TO_u8(offset_y)};
    xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)ret, sizeof(ret));

    return true;
}

bool xap_execute_qp_flush(xap_token_t token, xap_route_user_quantum_painter_flush_arg_t* arg) {
    xap_respond_success(token);

    qp_flush(DEVICE);

    return true;
}

bool xap_execute_qp_viewport(xap_token_t token, xap_route_user_quantum_painter_viewport_arg_t* arg) {
    xap_respond_success(token);

    qp_viewport(DEVICE, arg->left, arg->top, arg->right, arg->bottom);

    return true;
}

bool xap_respond_qp_pixdata(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_pixdata_arg_t  __arg;
    xap_route_user_quantum_painter_pixdata_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    // 64 bytes
    //    - 2 token
    //    - 1 payload length
    //    - 3 route
    //    - 1 screen id
    // ------
    //     57 bytes left -> 28 RGB565(2-byte) pixels
    uint16_t pixels[28] = {0};
    uint8_t  n_pix = (data_len - 1) / 2;
    memcpy(pixels, arg->pixels, n_pix * 2);

    xap_respond_success(token);

    qp_pixdata(DEVICE, (const void *)pixels, n_pix);

    return true;
}

bool xap_respond_qp_textwidth(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_textwidth_arg_t  __arg;
    xap_route_user_quantum_painter_textwidth_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    int16_t width = qp_textwidth(FONT, (const char *)arg->text);

    uint8_t ret[2] = { u16_TO_u8(width) };
    xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)ret, sizeof(ret));

    return true;
}

bool xap_respond_draw_scrolling_text(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_scrolling_text_arg_t  __arg;
    xap_route_user_quantum_painter_scrolling_text_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    deferred_token def_token = draw_scrolling_text(DEVICE, arg->x, arg->y, qp_fonts[arg->font_id], (const char *)arg->text, arg->n_chars, arg->delay);

    xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)&def_token, sizeof(def_token));

    return true;
}

bool xap_respond_stop_scrolling_text(xap_token_t token, xap_route_user_quantum_painter_stop_scrolling_text_arg_t* arg) {
    xap_respond_success(token);

    stop_scrolling_text(arg->token);

    return true;
}

bool xap_respond_extend_scrolling_text(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_route_user_quantum_painter_extend_scrolling_text_arg_t  __arg;
    xap_route_user_quantum_painter_extend_scrolling_text_arg_t* arg = &__arg;
    memcpy(arg, data, data_len);

    xap_respond_success(token);

    extend_scrolling_text(arg->token, (const char *)arg->text);

    return true;
}

