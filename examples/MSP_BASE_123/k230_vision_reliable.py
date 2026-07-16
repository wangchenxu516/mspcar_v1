# -*- coding: utf-8 -*-
"""
K230D BOX real-time symbol order recognition and MCU UART reporting.

Read order: top-left, top-right, bottom-left, bottom-right.
Allowed arrangements:
ABCD -> 90 degrees
DCBA -> 180 degrees
ACBD -> 270 degrees
CBDA -> 360 degrees

UART2 packet: AA 55 01 angle_low angle_high checksum
checksum = (AA + 55 + 01 + angle_low + angle_high) & 0xFF

Wiring:
GPIO44 (K230 UART2_TX) -> MCU PB3 (UART3_RX)
GPIO45 (K230 UART2_RX) <- MCU PB2 (UART3_TX)
K230 GND <-> MCU GND
UART: 115200, 8-N-1, 3.3 V levels.
"""

import os
import gc

from machine import FPIOA, UART
from libs.PlatTasks import DetectionApp
from libs.PipeLine import PipeLine
from libs.Utils import *


# 正点原子 K230D BOX onboard LCD.
DISPLAY_MODE = "st7701"
RGB888P_SIZE = [1280, 720]
ROOT_PATH = "/sdcard/mp_deployment_source/"

# Recognition must remain unchanged for this many consecutive frames.
STABLE_FRAMES = 8

# Send the same confirmed result several times. A complete packet is only
# six bytes, so this adds little UART load but avoids a one-shot packet loss.
SEND_REPEAT = 5

# Read order: top-left, top-right, bottom-left, bottom-right.
ORDER_TO_ANGLE = {
    "ABCD": 90,
    "DCBA": 180,
    "ACBD": 270,
    "CBDA": 360,
}


def init_uart():
    fpioa = FPIOA()
    fpioa.set_function(44, FPIOA.UART2_TXD)
    fpioa.set_function(45, FPIOA.UART2_RXD)

    return UART(
        UART.UART2,
        baudrate=115200,
        bits=UART.EIGHTBITS,
        parity=UART.PARITY_NONE,
        stop=UART.STOPBITS_ONE,
    )


def build_target_packet(angle):
    angle_low = angle & 0xFF
    angle_high = (angle >> 8) & 0xFF
    checksum = (
        0xAA + 0x55 + 0x01 + angle_low + angle_high
    ) & 0xFF

    return bytes([
        0xAA,
        0x55,
        0x01,
        angle_low,
        angle_high,
        checksum,
    ])


def send_target_angle(uart, angle):
    packet = build_target_packet(angle)
    successful_writes = 0

    print(
        "Target:", angle,
        "packet:", list(packet),
        "length:", len(packet),
    )

    for attempt in range(SEND_REPEAT):
        written = uart.write(packet)

        print(
            "TX attempt:", attempt + 1,
            "written:", written,
            "/", len(packet),
        )

        if written == len(packet):
            successful_writes += 1
        else:
            print("Warning: incomplete UART write")

    print(
        "UART full writes:", successful_writes,
        "/", SEND_REPEAT,
    )

    # If all five writes fail, do not lock. The main loop will retry after
    # the next valid recognition frame.
    return successful_writes > 0


def get_order(result, labels):
    """Parse PlatTasks result: scores, class indices and bounding boxes."""
    if not isinstance(result, dict):
        return None

    if (
        "scores" not in result
        or "idx" not in result
        or "boxes" not in result
    ):
        return None

    scores = result["scores"]
    indices = result["idx"]
    boxes = result["boxes"]
    count = min(len(scores), len(indices), len(boxes))
    best_by_label = {}

    for i in range(count):
        try:
            class_id = int(indices[i])
            score = float(scores[i])
            x1 = float(boxes[i][0])
            y1 = float(boxes[i][1])
            x2 = float(boxes[i][2])
            y2 = float(boxes[i][3])
        except Exception:
            continue

        if class_id < 0 or class_id >= len(labels):
            continue

        label = labels[class_id]
        old = best_by_label.get(label)

        if old is None or score > old[0]:
            center_x = (x1 + x2) / 2
            center_y = (y1 + y2) / 2
            best_by_label[label] = (
                score,
                center_x,
                center_y,
            )

    # A valid target must contain A, B, C and D.
    if not all(
        label in best_by_label
        for label in ("A", "B", "C", "D")
    ):
        return None

    items = []
    for label in ("A", "B", "C", "D"):
        score, center_x, center_y = best_by_label[label]
        items.append((label, center_x, center_y))

    # Top two symbols first, then bottom two; each row left to right.
    items.sort(key=lambda item: item[2])
    top_row = sorted(items[:2], key=lambda item: item[1])
    bottom_row = sorted(items[2:], key=lambda item: item[1])

    return "".join(
        item[0]
        for item in top_row + bottom_row
    )


def load_config():
    conf = read_json(ROOT_PATH + "deploy_config.json")
    anchors = []

    if conf["model_type"] == "AnchorBaseDet":
        anchors = (
            conf["anchors"][0]
            + conf["anchors"][1]
            + conf["anchors"][2]
        )

    return (
        ROOT_PATH + conf["kmodel_path"],
        conf["categories"],
        conf["confidence_threshold"],
        conf["nms_threshold"],
        conf["img_size"],
        conf["nms_option"],
        conf["model_type"],
        anchors,
    )


uart = None
pl = None
det_app = None

try:
    # Initialize UART before the model and display so the port is ready well
    # before the first stable recognition result is produced.
    uart = init_uart()

    (
        kmodel_path,
        labels,
        confidence_threshold,
        nms_threshold,
        model_input_size,
        nms_option,
        model_type,
        anchors,
    ) = load_config()

    pl = PipeLine(
        rgb888p_size=RGB888P_SIZE,
        display_mode=DISPLAY_MODE,
    )
    pl.create()
    display_size = pl.get_display_size()

    det_app = DetectionApp(
        "video",
        kmodel_path,
        labels,
        model_input_size,
        anchors,
        model_type,
        confidence_threshold,
        nms_threshold,
        RGB888P_SIZE,
        display_size,
        debug_mode=0,
    )
    det_app.config_preprocess()

    candidate_order = None
    candidate_frames = 0
    angle_sent = False
    last_printed_order = None

    print("Waiting for A, B, C, D in one frame...")

    while True:
        os.exitpoint()

        with ScopedTiming("total", 0):
            img = pl.get_frame()
            result = det_app.run(img)
            det_app.draw_result(pl.osd_img, result)

            order = get_order(result, labels)
            angle = ORDER_TO_ANGLE.get(order)

            if order != last_printed_order:
                print("Spatial order:", order)
                last_printed_order = order

            if angle is None:
                candidate_order = None
                candidate_frames = 0
            else:
                if order == candidate_order:
                    candidate_frames += 1
                else:
                    candidate_order = order
                    candidate_frames = 1

                if (
                    candidate_frames >= STABLE_FRAMES
                    and not angle_sent
                ):
                    angle_sent = send_target_angle(uart, angle)

                    if angle_sent:
                        print(
                            "Command transmitted and locked. "
                            "Restart the script for the next target."
                        )
                    else:
                        print(
                            "UART transmission failed. "
                            "Will retry on the next valid frame."
                        )

            pl.show_image()
            gc.collect()

except KeyboardInterrupt as e:
    print("User stop:", e)
except BaseException as e:
    print("Exception:", e)
finally:
    if det_app is not None:
        det_app.deinit()

    if pl is not None:
        pl.destroy()

    gc.collect()
