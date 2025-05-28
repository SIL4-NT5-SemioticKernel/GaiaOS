import RPi.GPIO as GPIO
from pathlib import Path
import time
import logging
import traceback
import signal
import sys
import os
import tempfile

# Configuration Constants
CONFIG_FILE = "pinout.cfg"
POLL_INTERVAL = 0.01               # seconds
UPDATE_CHECK_INTERVAL = 1.0       # seconds
MAX_US_RANGE_CM = 450             # sensor max range (cm)
US_TIMEOUT = (MAX_US_RANGE_CM*2)/34300 + 0.005  # seconds

CONTROL_PANEL_FILE = Path("Control_Panel.ssv")
CONTROL_PANEL_FLAG_FILE = Path("Control_Panel_Flag.ssv")

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s: %(message)s"
)
logger = logging.getLogger(__name__)

afferents = []
efferents = []

def write_atomic(path: Path, data: str):
    """
    Atomically write data to a file by writing to a temp file then renaming.
    """
    dirpath = path.parent or Path('.')
    with tempfile.NamedTemporaryFile('w', dir=str(dirpath), delete=False) as tf:
        tf.write(data)
        tf.flush()
        os.fsync(tf.fileno())
    os.replace(tf.name, str(path))

# Signal handlers for clean shutdown
def _handle_signal(signum, frame):
    logger.info(f"Signal {signum} received, cleaning up and exiting")
    GPIO.cleanup()
    sys.exit(0)

for _sig in (signal.SIGTERM, signal.SIGHUP):
    signal.signal(_sig, _handle_signal)

# -- Configuration Parsing --
def parse_config(path: str):
    """
    Parses pinout.cfg file. Supports modes:
      A <pins> <path>
      E <pin> <path>
      A1W <sensor_id> <path>
      US <trig_pin> <echo_pin> <path>
    """
    global afferents, efferents
    afferents = []
    efferents = []

    try:
        with open(path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('//'):
                    continue
                parts = line.split()
                mode = parts[0]
                # Validate minimal parts
                if mode == 'A' and len(parts) < 3:
                    logger.warning(f"Skipping malformed A line: '{line}'")
                    continue
                if mode == 'E' and len(parts) != 3:
                    logger.warning(f"Skipping malformed E line: '{line}'")
                    continue
                if mode == 'A1W' and len(parts) != 3:
                    logger.warning(f"Skipping malformed A1W line: '{line}'")
                    continue
                if mode == 'US' and len(parts) != 4:
                    logger.warning(f"Skipping malformed US line: '{line}'")
                    continue

                try:
                    if mode == 'A':
                        pins = list(map(int, parts[1:-1]))
                        path_ = Path(parts[-1])
                        afferents.append({'type': 'gpio', 'pins': pins, 'path': path_})
                    elif mode == 'E':
                        pin = int(parts[1])
                        path_ = Path(parts[2])
                        efferents.append({'pin': pin, 'path': path_})
                    elif mode == 'A1W':
                        sensor_id = parts[1]
                        path_ = Path(parts[2])
                        afferents.append({'type': '1wire', 'sensor': sensor_id, 'path': path_})
                    elif mode == 'US':
                        trig, echo = map(int, parts[1:3])
                        path_ = Path(parts[3])
                        afferents.append({'type': 'ultrasonic', 'trig': trig, 'echo': echo, 'path': path_})
                except Exception as e:
                    logger.error(f"Error parsing line '{line}': {e}")
    except Exception as e:
        logger.error(f"Could not open config file '{path}': {e}")
        sys.exit(1)

# -- GPIO Setup --
def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    for aff in afferents:
        if aff['type'] == 'gpio':
            for pin in aff['pins']:
                GPIO.setup(pin, GPIO.IN)
        elif aff['type'] == 'ultrasonic':
            GPIO.setup(aff['trig'], GPIO.OUT)
            GPIO.setup(aff['echo'], GPIO.IN)
    for eff in efferents:
        GPIO.setup(eff['pin'], GPIO.OUT)

# Read multiple pins as bit-bus
def read_gpio_to_bus(pins):
    val = 0
    for i, pin in enumerate(pins):
        bit = GPIO.input(pin)
        shift = len(pins) - 1 - i
        val |= (bit << shift)
    return val

# Single ultrasonic distance measurement
def read_ultrasonic(trig_pin, echo_pin):
    GPIO.output(trig_pin, False)
    time.sleep(0.002)
    GPIO.output(trig_pin, True)
    time.sleep(0.00001)
    GPIO.output(trig_pin, False)

    start_time = time.time()
    # wait for echo rising edge
    while GPIO.input(echo_pin) == 0:
        if time.time() - start_time > US_TIMEOUT:
            return None
    pulse_start = time.time()
    # wait for echo falling edge
    while GPIO.input(echo_pin) == 1:
        if time.time() - pulse_start > US_TIMEOUT:
            return None
    pulse_end = time.time()

    duration = pulse_end - pulse_start
    # convert to distance in cm
    return round(duration * 17150, 2)

# Perform one read/write cycle
def bridge_once():
    # Efferent writes: file -> GPIO
    for eff in efferents:
        try:
            raw = eff['path'].read_text().strip()
            val = int(raw)
            GPIO.output(eff['pin'], GPIO.HIGH if val else GPIO.LOW)
        except Exception as e:
            logger.exception(f"Efferent write error on {eff['path']}")

    # Afferent reads: GPIO/1-wire/ultrasonic -> file
    for aff in afferents:
        try:
            if aff['type'] == 'gpio':
                val = read_gpio_to_bus(aff['pins'])
            elif aff['type'] == '1wire':
                sensor_dir = Path(f"/sys/bus/w1/devices/{aff['sensor']}")
                slave = sensor_dir / 'w1_slave'
                lines = slave.read_text().splitlines()
                if len(lines) < 2 or not lines[0].endswith('YES'):
                    continue
                if 't=' not in lines[1]:
                    continue
                temp_c = float(lines[1].split('t=')[-1]) / 1000.0
                val = temp_c
            elif aff['type'] == 'ultrasonic':
                val = read_ultrasonic(aff['trig'], aff['echo'])
                if val is None:
                    continue
            else:
                continue

            write_atomic(aff['path'], f"{val}\n")
        except Exception as e:
            logger.exception(f"Afferent write error on {aff['path']}")

# Main loop handling control panel flag and periodic bridge
def bridge_loop():
    last_check = time.time()
    while True:
        now = time.time()
        if now - last_check >= UPDATE_CHECK_INTERVAL:
            last_check = now
            try:
                flag = CONTROL_PANEL_FLAG_FILE.read_text().strip() if CONTROL_PANEL_FLAG_FILE.exists() else None
                if flag != '1':
                    bridge_once()
                    write_atomic(CONTROL_PANEL_FILE, "eval Testermon.txt 0.0\n")
                    write_atomic(CONTROL_PANEL_FLAG_FILE, "1\n")
            except Exception:
                logger.exception("Control panel bridge error")
        time.sleep(POLL_INTERVAL)

if __name__ == '__main__':
    try:
        parse_config(CONFIG_FILE)
        setup_gpio()
        bridge_loop()
    except KeyboardInterrupt:
        logger.info("Interrupted by user, exiting")
    finally:
        GPIO.cleanup()
