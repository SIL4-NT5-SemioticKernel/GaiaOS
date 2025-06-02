import RPi.GPIO as GPIO
from pathlib import Path
import time
import logging
import traceback
import signal
import sys
import os
import tempfile

# ——————————————————————————————————————————————————————————————————————————————
# Configuration Constants
# ——————————————————————————————————————————————————————————————————————————————
CONFIG_FILE           = "pinout.cfg"
POLL_INTERVAL         = 0.01            # Seconds between loop iterations
UPDATE_CHECK_INTERVAL = 1.0             # Seconds between checking Control_Panel flag
MAX_US_RANGE_CM       = 450             # Max ultrasonic range in cm
US_TIMEOUT            = (MAX_US_RANGE_CM * 2) / 34300 + 0.005
                                                # Seconds to wait for echo (~30 ms + margin)

# Control-Panel files (written atomically)
CONTROL_PANEL_FILE      = Path("Control_Panel.ssv")
CONTROL_PANEL_FLAG_FILE = Path("Control_Panel_Flag.ssv")

# ——————————————————————————————————————————————————————————————————————————————
# Set up logging
# ——————————————————————————————————————————————————————————————————————————————
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s: %(message)s"
)
logger = logging.getLogger(__name__)

# LOG: Driver started
logger.info("=== RPi_Driver starting up ===")

# ——————————————————————————————————————————————————————————————————————————————
# Global lists to hold our “afferent” and “efferent” descriptors
#   afferents: list of dicts, each one has keys depending on type:
#       • 'type':  'gpio' / '1wire' / 'ultrasonic' / 'ph'
#       • 'pins': [pin, …]           (for type='gpio')
#       • 'sensor': “28-…-…”         (for type='1wire')
#       • 'trig', 'echo': pin, pin   (for type='ultrasonic')
#       • 'addr', 'chan', 'slope', 'offset': info for type='ph'
#       • 'path': Path object to file where we write the numeric result
#
#   efferents: list of dicts, each one has:
#       • 'pin': BCM pin number
#       • 'path': Path object to file we read (0 or 1 int)
# ——————————————————————————————————————————————————————————————————————————————
afferents = []
efferents = []

# ——————————————————————————————————————————————————————————————————————————————
# Utility: write text to a file atomically to avoid half-writes
# ——————————————————————————————————————————————————————————————————————————————
def write_atomic(path: Path, data: str):
    """
    Atomically write data to 'path' by writing to a temp file and renaming.
    Ensures readers never see a half-written file.
    """
    dirpath = path.parent or Path(".")
    with tempfile.NamedTemporaryFile("w", dir=str(dirpath), delete=False) as tf:
        tf.write(data)
        tf.flush()
        os.fsync(tf.fileno())
    os.replace(tf.name, str(path))

    # LOG: Confirm file written atomically
    logger.debug(f"Atomic write to {path}: {data.strip()}")

# ——————————————————————————————————————————————————————————————————————————————
# Clean shutdown on SIGTERM / SIGHUP (and optionally SIGINT)
# ——————————————————————————————————————————————————————————————————————————————
def _handle_signal(signum, frame):
    logger.info(f"Signal {signum} received — cleaning up GPIO and exiting")
    GPIO.cleanup()
    sys.exit(0)

# Capture SIGTERM and SIGHUP
for _sig in (signal.SIGTERM, signal.SIGHUP):
    signal.signal(_sig, _handle_signal)

# LOG: Signal handlers registered

# ——————————————————————————————————————————————————————————————————————————————
# parse_config: load pinout.cfg, populate 'afferents' and 'efferents'
# ——————————————————————————————————————————————————————————————————————————————
def parse_config(path: str):
    """
    Expects a whitespace-delimited file with lines like:
      A    <pin> <pin> …   <filepath>
      E    <pin>           <filepath>
      A1W  <sensor_id>     <filepath>
      US   <trig_pin> <echo_pin> <filepath>
      PH   <i2c_addr_hex> <chan> <filepath>
    """
    global afferents, efferents
    afferents = []
    efferents = []

    try:
        with open(path, "r") as f:
            logger.info(f"Parsing config file: {path}")
            for line in f:
                line = line.strip()
                if not line or line.startswith("//"):
                    continue

                parts = line.split()
                mode = parts[0]

                # Basic validation of token counts
                if mode == "A" and len(parts) < 3:
                    logger.warning(f"Skipping malformed A line: '{line}'")
                    continue
                if mode == "E" and len(parts) != 3:
                    logger.warning(f"Skipping malformed E line: '{line}'")
                    continue
                if mode == "A1W" and len(parts) != 3:
                    logger.warning(f"Skipping malformed A1W line: '{line}'")
                    continue
                if mode == "US" and len(parts) != 4:
                    logger.warning(f"Skipping malformed US line: '{line}'")
                    continue
                if mode == "PH" and len(parts) != 4:
                    logger.warning(f"Skipping malformed PH line: '{line}'")
                    continue

                try:
                    if mode == "A":
                        # GPIO bus (read multiple pins as bits)
                        pins = list(map(int, parts[1:-1]))
                        fpath = Path(parts[-1])
                        afferents.append({
                            "type":  "gpio",
                            "pins":  pins,
                            "path":  fpath
                        })
                        # LOG: Loaded GPIO afferent
                        logger.info(f"Configured GPIO afferent on pins {pins} → {fpath}")

                    elif mode == "E":
                        # Efferent: single GPIO pin → read file
                        pin = int(parts[1])
                        fpath = Path(parts[2])
                        efferents.append({
                            "pin":  pin,
                            "path": fpath
                        })
                        # LOG: Loaded Efferent
                        logger.info(f"Configured efferent pin {pin} ← {fpath}")

                    elif mode == "A1W":
                        # 1-Wire sensor: directory named by sensor ID
                        sensor_id = parts[1]
                        fpath     = Path(parts[2])
                        afferents.append({
                            "type":   "1wire",
                            "sensor": sensor_id,
                            "path":   fpath
                        })
                        # LOG: Loaded 1-Wire afferent
                        logger.info(f"Configured 1-Wire sensor {sensor_id} → {fpath}")

                    elif mode == "US":
                        # HC-SR04 ultrasonic: trig + echo pins
                        trig, echo = map(int, parts[1:3])
                        fpath      = Path(parts[3])
                        afferents.append({
                            "type":  "ultrasonic",
                            "trig":  trig,
                            "echo":  echo,
                            "path":  fpath
                        })
                        # LOG: Loaded Ultrasonic afferent
                        logger.info(f"Configured HC-SR04 (TRIG={trig}, ECHO={echo}) → {fpath}")

                    elif mode == "PH":
                        # pH over ADS1115: address + channel + file
                        addr   = int(parts[1], 16)   # parse hex, e.g. “0x48”
                        chan   = int(parts[2])       # 0 for A0, 1 for A1, etc.
                        fpath  = Path(parts[3])

                        # YOU MUST MEASURE V7 and V4 at runtime or hard-code them here
                        # Example: V7=1.48 V, V4=1.59 V  →  slope=(7–4)/(1.48–1.59)
                        V7    = 1.48   # measured voltage at pH 7.00
                        V4    = 1.59   # measured voltage at pH 4.00
                        slope = (7.0 - 4.0) / (V7 - V4)
                        offset= 7.0 - slope * V7

                        afferents.append({
                            "type":   "ph",
                            "addr":   addr,
                            "chan":   chan,
                            "path":   fpath,
                            "slope":  slope,
                            "offset": offset
                        })
                        # LOG: Loaded pH afferent
                        logger.info(f"Configured pH on ADS1115@0x{addr:02X}, channel {chan} → {fpath}")

                except Exception as e:
                    logger.error(f"Error parsing line '{line}': {e}")
    except Exception as e:
        logger.error(f"Could not open config file '{path}': {e}")
        sys.exit(1)

# ——————————————————————————————————————————————————————————————————————————————
# setup_gpio: configure all GPIO pins based on 'afferents' and 'efferents'
# ——————————————————————————————————————————————————————————————————————————————
def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    logger.info("Setting up GPIO pins")

    for aff in afferents:
        if aff["type"] == "gpio":
            for pin in aff["pins"]:
                GPIO.setup(pin, GPIO.IN)
            # LOG: Confirm bus pins as inputs
            logger.debug(f"GPIO bus set as inputs on pins {aff['pins']}")

        elif aff["type"] == "ultrasonic":
            GPIO.setup(aff["trig"], GPIO.OUT)
            GPIO.setup(aff["echo"], GPIO.IN)
            # LOG: Confirm ultrasonic pins
            logger.debug(f"Ultrasonic TRIG pin {aff['trig']} set as OUT, ECHO pin {aff['echo']} set as IN")

        # PH-4502C → ADS1115 nodes do not use direct GPIO (they use I²C),
        # and 1-Wire is handled by the kernel’s w1 driver, so no direct GPIO setup here.

    for eff in efferents:
        GPIO.setup(eff["pin"], GPIO.OUT)
        # LOG: Confirm efferent pins
        logger.debug(f"Efferent pin {eff['pin']} set as OUT")

    logger.info("GPIO setup complete")

# ——————————————————————————————————————————————————————————————————————————————
# read_gpio_to_bus: collapse multiple GPIO inputs into one integer
# ——————————————————————————————————————————————————————————————————————————————
def read_gpio_to_bus(pins):
    """
    Read each GPIO in 'pins' (MSB first) and combine into a single integer.
    E.g. if pins=[17,27,22] and states are [1,0,1], val = 0b101 = 5.
    """
    val = 0
    for i, pin in enumerate(pins):
        bit   = GPIO.input(pin)
        shift = len(pins) - 1 - i
        val  |= (bit << shift)
    # LOG: Show bus read
    logger.debug(f"GPIO bus pins {pins} read value {val}")
    return val

# ——————————————————————————————————————————————————————————————————————————————
# read_ultrasonic: fire HC-SR04 and measure echo pulse width
# ——————————————————————————————————————————————————————————————————————————————
def read_ultrasonic(trig_pin, echo_pin):
    # LOG: Starting ultrasonic measurement
    logger.debug(f"Ultrasonic: triggering on pin {trig_pin}")
    # Trigger a 10 µs HIGH pulse
    GPIO.output(trig_pin, False)
    time.sleep(0.002)
    GPIO.output(trig_pin, True)
    time.sleep(0.00001)
    GPIO.output(trig_pin, False)

    start_time = time.time()
    # Wait for ECHO to go HIGH (rising edge)
    while GPIO.input(echo_pin) == 0:
        if time.time() - start_time > US_TIMEOUT:
            logger.warning(f"Ultrasonic timeout waiting for echo HIGH on pin {echo_pin}")
            return None
    pulse_start = time.time()

    # Wait for ECHO to go LOW (falling edge)
    while GPIO.input(echo_pin) == 1:
        if time.time() - pulse_start > US_TIMEOUT:
            logger.warning(f"Ultrasonic timeout waiting for echo LOW on pin {echo_pin}")
            return None
    pulse_end = time.time()

    pulse_duration = pulse_end - pulse_start
    distance_cm = round(pulse_duration * 17150, 2)
    # LOG: Report computed distance
    logger.info(f"Ultrasonic (TRIG={trig_pin}, ECHO={echo_pin}) distance: {distance_cm} cm")
    return distance_cm

# ——————————————————————————————————————————————————————————————————————————————
# read_ph: use ADS1115 to read channel and convert voltage → pH
# ——————————————————————————————————————————————————————————————————————————————
# We’ll initialize ADS instances later, keyed by I²C address.
ads_devices = {}

def read_ph(aff):
    """
    aff has fields: 'addr', 'chan', 'slope', 'offset'.
    Return a floating-point pH value or None on timeout/error.
    """
    from board import SCL, SDA
    import busio
    from adafruit_ads1x15.ads1115 import ADS1115
    from adafruit_ads1x15.analog_in import AnalogIn

    addr = aff["addr"]
    if addr not in ads_devices:
        i2c = busio.I2C(SCL, SDA)
        ads_devices[addr] = ADS1115(i2c, address=addr)
        # LOG: ADS1115 initialized
        logger.info(f"Initialized ADS1115 at I²C address 0x{addr:02X}")

    ads = ads_devices[addr]
    chan_name = f"P{aff['chan']}"    # e.g. "P0" or "P1"
    chan = AnalogIn(ads, getattr(ADS1115, chan_name))

    voltage = chan.voltage
    pH_val = aff["slope"] * voltage + aff["offset"]
    # LOG: Report raw voltage and computed pH
    logger.info(f"pH read (ADS@0x{addr:02X} CH{aff['chan']}): V={voltage:.3f} → pH={pH_val:.2f}")
    return pH_val

# ——————————————————————————————————————————————————————————————————————————————
# bridge_once: perform one cycle of reading all afferents and writing all efferents
# ——————————————————————————————————————————————————————————————————————————————
def bridge_once():
    # — Efferents: read a file (0 or 1) and drive GPIO
    for eff in efferents:
        try:
            raw = eff["path"].read_text().strip()
            val = int(raw)
            GPIO.output(eff["pin"], GPIO.HIGH if val else GPIO.LOW)
            # LOG: Report efferent action
            logger.info(f"Efferent pin {eff['pin']} set to {'HIGH' if val else 'LOW'} (file {eff['path']})")
        except Exception:
            logger.exception(f"Efferent write error on {eff['path']}")

    # — Afferents: for each sensor type, read and write to file
    for aff in afferents:
        try:
            if aff["type"] == "gpio":
                val = read_gpio_to_bus(aff["pins"])
                # LOG: Report GPIO bus value before writing
                logger.info(f"GPIO bus {aff['pins']} read {val}")

            elif aff["type"] == "1wire":
                sensor_dir = Path(f"/sys/bus/w1/devices/{aff['sensor']}")
                slave_file = sensor_dir / "w1_slave"
                try:
                    lines = slave_file.read_text().splitlines()
                    if len(lines) < 2 or not lines[0].endswith("YES"):
                        logger.warning(f"1-Wire sensor {aff['sensor']} CRC bad or missing")
                        continue
                    if "t=" not in lines[1]:
                        logger.warning(f"1-Wire sensor {aff['sensor']} no temperature data")
                        continue
                    temp_c = float(lines[1].split("t=")[-1]) / 1000.0
                    val = temp_c
                    # LOG: Report 1-Wire temperature reading
                    logger.info(f"1-Wire sensor {aff['sensor']} temperature: {val:.2f} °C")
                except Exception:
                    logger.exception(f"Reading 1-Wire sensor {aff['sensor']}")
                    continue

            elif aff["type"] == "ultrasonic":
                val = read_ultrasonic(aff["trig"], aff["echo"])
                if val is None:
                    continue

            elif aff["type"] == "ph":
                val = read_ph(aff)
                if val is None:
                    continue

            else:
                continue

            write_atomic(aff["path"], f"{val}\n")
            # LOG: Confirm afferent write
            logger.info(f"Afferent '{aff['type']}' → wrote {val} to {aff['path']}")

        except Exception:
            logger.exception(f"Afferent write error on {aff['path']}")

# ——————————————————————————————————————————————————————————————————————————————
# Main loop: check control-panel flag, run bridge_once(), update flag
# ——————————————————————————————————————————————————————————————————————————————
def bridge_loop():
    last_check = time.time()
    while True:
        now = time.time()
        if now - last_check >= UPDATE_CHECK_INTERVAL:
            last_check = now
            try:
                flag = (
                    CONTROL_PANEL_FLAG_FILE.read_text().strip()
                    if CONTROL_PANEL_FLAG_FILE.exists()
                    else None
                )
                if flag != "1":
                    logger.debug("Control_Panel_Flag not 1 → running bridge_once()")
                    bridge_once()
                    write_atomic(CONTROL_PANEL_FILE,       "eval Testermon.txt 0.0\n")
                    write_atomic(CONTROL_PANEL_FLAG_FILE,  "1\n")
                    # LOG: Notified control panel
                    logger.info("Wrote Control_Panel.ssv and set Control_Panel_Flag.ssv to 1")
            except Exception:
                logger.exception("Control panel bridge error")

        time.sleep(POLL_INTERVAL)

# ——————————————————————————————————————————————————————————————————————————————
# Program entry point
# ——————————————————————————————————————————————————————————————————————————————
if __name__ == "__main__":
    try:
        parse_config(CONFIG_FILE)
        setup_gpio()
        # LOG: Entering main loop
        logger.info("Entering main bridge loop")
        bridge_loop()
    except KeyboardInterrupt:
        logger.info("Interrupted by user (KeyboardInterrupt), exiting")
    finally:
        GPIO.cleanup()
        logger.info("GPIO cleaned up, driver exiting")
