const std = @import("std");

const MCU = struct {
    name: []const u8,
    target: std.Target.Query,
};

const KnownMcus = [_]MCU{
    .{
        .name = "RP2040",
        .target = .{
            .cpu_arch = .thumb,
            .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_m0plus },
            .os_tag = .freestanding,
            .abi = .eabi,
        },
    },
};

fn getTarget(mcu: []const u8) std.Target.Query {
    inline for (KnownMcus) |known_mcu| {
        if (std.mem.eql(u8, mcu, known_mcu.name)) {
            return known_mcu.target;
        }
    }

    std.debug.panic("Unknown MCU: {s}", .{mcu});
}

pub fn build(b: *std.Build) !void {
    const mcu = b.option([]const u8, "mcu", "MCU to compile for") orelse @panic("Select a MCU");
    const target = b.resolveTargetQuery(getTarget(mcu));

    const optimize = b.standardOptimizeOption(.{});

    // const elpekenin = ".";
    // const users = elpekenin ++ "/..";
    // const qmk = users ++ "/..";
    // const platforms = qmk ++ "/platforms";
    // const lib = qmk ++ "/lib";
    // const chibios = lib ++ "/chibios/os";

    const elpekenin_lib = b.addStaticLibrary(.{
        .root_source_file = b.path("src/zig/elpekenin.zig"),
        .name = "elpekenin",
        .target = target,
        .optimize = optimize,
    });

    elpekenin_lib.is_linking_libc = true;

    // // ideally, we would only need these two paths
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = qmk});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = elpekenin ++ "/include"});

    // // ... but QMK's includes are a mess
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = chibios ++ "/common/portability/GCC"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = chibios ++ "/hal/include"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = chibios ++ "/hal/osal/rt-nil"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = chibios ++ "/license"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = chibios ++ "/rt/include"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = lib ++ "/printf/src/printf"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = platforms});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = platforms ++ "/chibios"});
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = qmk ++ "/quantum"});

    // // ... and my 3rd_party libs too :P
    // elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = elpekenin ++ "/3rd_party/backtrace/include"});

    // // hack newlib include path too...
    // elpekenin_lib.addSystemIncludePath(.{.cwd_relative = "/usr/include/newlib/"});

    // generate the .a file
    b.installArtifact(elpekenin_lib);
}
