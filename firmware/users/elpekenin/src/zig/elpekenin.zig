const std = @import("std");

// const c = @cImport({
//     @cInclude("quantum/painter/qp.h");

//     @cInclude("elpekenin.h");
//     @cInclude("elpekenin/logging.h");
//     @cInclude("elpekenin/qp/graphics.h");
// });

const device = *const void;
const font = *const void;
const image = *const void;

extern fn logging(c_int, c_int, [*:0]const u8, ...) c_int;

extern fn qp_get_width(device) u16;
extern fn qp_get_height(device) u16;
extern fn qp_rect(device, u16, u16, u16, u16, u8, u8, u8, bool) bool;
extern fn qp_power(device, bool) bool;

const QP = 2;

const INFO = 3; // 2
const WARN = 3;

const K = [*:0]const u8;
fn Map(type_: type) type {
    return std.array_hash_map.AutoArrayHashMap(K, type_);
}

const DeviceMap = Map(device);
const FontMap = Map(font);
const ImageMap = Map(image);

var displays = DeviceMap.init(std.heap.c_allocator);
var fonts = FontMap.init(std.heap.c_allocator);
var images = ImageMap.init(std.heap.c_allocator);

//

export fn load_display(item: device, name: K) void {
    displays.put(name, item) catch |e| {
        _ = logging(QP, WARN, "Could not put %s (%p)", @errorName(e).ptr, item);
        return;
    };

    _ = logging(QP, INFO, "'%s' saved (%p)", name, item);
}

export fn load_font(item: font, name: K) void {
    fonts.put(name, item) catch |e| {
        _ = logging(QP, WARN, "Could not put %s (%p)", @errorName(e).ptr, item);
        return;
    };

    _ = logging(QP, INFO, "'%s' saved (%p)", name, item);
}

export fn load_image(item: image, name: K) void {
    images.put(name, item) catch |e| {
        _ = logging(QP, WARN, "Could not put %s (%p)", @errorName(e).ptr, item);
        return;
    };

    _ = logging(QP, INFO, "'%s' saved (%p)", name, item);
}

//

export fn qp_get_device_by_name(name: K) ?device {
    const val = displays.get(name);
    _ = logging(QP, INFO, "'%s' is: %p", name, val);
    return val;
}

export fn qp_get_font_by_name(name: K) ?font {
    const val = fonts.get(name);
    _ = logging(QP, INFO, "'%s' is: %p", name, val);
    return val;
}

export fn qp_get_img_by_name(name: K) ?image {
    const val = images.get(name);
    _ = logging(QP, INFO, "'%s' is: %p", name, val);
    return val;
}

//

export fn elpekenin_qp_deinit(bootloader: bool) void {
    _ = bootloader;

    for (displays.values()) |display| {
        _ = qp_rect(display, 0, 0, qp_get_width(display), qp_get_height(display), 0, 0, 0, true);
        _ = qp_power(display, false);
    }
}
