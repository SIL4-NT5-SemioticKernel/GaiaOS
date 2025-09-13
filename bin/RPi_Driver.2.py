# =====================================================================================
# Raspberry Pi 5–ready version: GPIO access via lgpio with an RPi.GPIO-like shim
# =====================================================================================
from pathlib import Path
import time
import logging
import traceback
import signal
import sys
import os
import tempfile
import smbus2

# ─────────────────────────────────────────────────────────────────────────────────────
# GPIO shim (Pi 5): replicate minimal RPi.GPIO API using lgpio
# ─────────────────────────────────────────────────────────────────────────────────────
try:
    import lgpio  # python3-lgpio
except ImportError as _e:
    raise SystemExit(
        "python3-lgpio is required on Raspberry Pi 5. Install with: sudo apt install python3-lgpio"
    )

class _GPIOShim:
    # Constants to match RPi.GPIO usage in the original code
    BCM  = 11
    BOARD= 10  # not implemented; kept for completeness
    IN   = 1
    OUT  = 0
    LOW  = 0
    HIGH = 1

    def __init__(self):
        self._mode = None
        self._chip_handle = None
        self._claimed = set()  # track claimed line numbers (BCM)
        self._output_state = {}  # remember last set output if needed

    # Compatibility: no-op but validate mode
    def setmode(self, mode):
        if mode not in (self.BCM, self.BOARD):
            raise ValueError("Unsupported mode")
        if mode == self.BOARD:
            raise NotImplementedError("BOARD numbering not supported by this shim; use BCM.")
        self._mode = mode
        if self._chip_handle is None:
            # On Pi 5, the main GPIO controller is exposed as gpiochip0
            self._chip_handle = lgpio.gpiochip_open(0)

    def _require_chip(self):
        if self._chip_handle is None:
            self.setmode(self.BCM)

    def setup(self, pin, direction):
        """
        Claim a GPIO line. 'pin' is BCM number (line offset).
        """
        self._require_chip()
        # Release if already claimed with different direction
        if pin in self._claimed:
            lgpio.gpio_free(self._chip_handle, pin)
            self._claimed.discard(pin)

        if direction == self.IN:
            lgpio.gpio_claim_input(self._chip_handle, pin)
        elif direction == self.OUT:
            # default LOW on claim
            lgpio.gpio_claim_output(self._chip_handle, pin, self.LOW)
            self._output_state[pin] = self.LOW
        else:
            raise ValueError("Invalid direction (use GPIO.IN or GPIO.OUT)")
        self._claimed.add(pin)

    def input(self, pin) -> int:
        self._require_chip()
        # It's valid to read regardless of claim direction
        return lgpio.gpio_read(self._chip_handle, pin)

    def output(self, pin, value: int):
        self._require_chip()
        v = self.HIGH if value else self.LOW
        # Claim as output on-the-fly if not already
        if pin not in self._claimed:
            lgpio.gpio_claim_output(self._chip_handle, pin, v)
            self._claimed.add(pin)
        else:
            # Ensure it's configured as output; if not, re-claim as output
            try:
                lgpio.gpio_write(self._chip_handle, pin, v)
            except lgpio.error:  # likely not configured as output
                lgpio.gpio_free(self._chip_handle, pin)
                lgpio.gpio_claim_output(self._chip_handle, pin, v)
        self._output_state[pin] = v

    def cleanup(self):
        if self._chip_handle is not None:
            for pin in list(self._claimed):
                try:
                    lgpio.gpio_free(self._chip_handle, pin)
                except Exception:
                    pass
                self._claimed.discard(pin)
            try:
                lgpio.gpiochip_close(self._chip_handle)
            except Exception:
                pass
            self._chip_handle = None
        self._mode = None
        self._output_state.clear()

# Instantiate shim with the same name as before
GPIO = _GPIOShim()

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
logger.info("=== RPi_Driver (Pi 5 + lgpio) starting up ===")

# ——————————————————————————————————————————————————————————————————————————————
# Global lists
# ——————————————————————————————————————————————————————————————————————————————
afferents = []
efferents = []

# ——————————————————————————————————————————————————————————————————————————————
# Utility: write text to a file atomically
# ——————————————————————————————————————————————————————————————————————————————
def write_atomic(path: Path, data: str):
    dirpath = path.parent or Path(".")
    with tempfile.NamedTemporaryFile("w", dir=str(dirpath), delete=False) as tf:
        tf.write(data)
        tf.flush()
        os.fsync(tf.fileno())
    os.replace(tf.name, str(path))
    logger.debug(f"Atomic write to {path}: {data.strip()}")

# ——————————————————————————————————————————————————————————————————————————————
# Clean shutdown on signals
# ——————————————————————————————————————————————————————————————————————————————
def _handle_signal(signum, frame):
    logger.info(f"Signal {signum} received — cleaning up GPIO and exiting")
    try:
        GPIO.cleanup()
    finally:
        sys.exit(0)

for _sig in (signal.SIGTERM, signal.SIGHUP):
    signal.signal(_sig, _handle_signal)

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
                        pins = list(map(int, parts[1:-1]))
                        fpath = Path(parts[-1])
                        afferents.append({
                            "type":  "gpio",
                            "pins":  pins,
                            "path":  fpath
                        })
                        logger.info(f"Configured GPIO afferent on pins {pins} → {fpath}")

                    elif mode == "E":
                        pin = int(parts[1])
                        fpath = Path(parts[2])
                        efferents.append({
                            "pin":  pin,
                            "path": fpath
                        })
                        logger.info(f"Configured efferent pin {pin} ← {fpath}")

                    elif mode == "A1W":
                        sensor_id = parts[1]
                        fpath     = Path(parts[2])
                        afferents.append({
                            "type":   "1wire",
                            "sensor": sensor_id,
                            "path":   fpath
                        })
                        logger.info(f"Configured 1-Wire sensor {sensor_id} → {fpath}")

                    elif mode == "US":
                        trig, echo = map(int, parts[1:3])
                        fpath      = Path(parts[3])
                        afferents.append({
                            "type":  "ultrasonic",
                            "trig":  trig,
                            "echo":  echo,
                            "path":  fpath
                        })
                        logger.info(f"Configured HC-SR04 (TRIG={trig}, ECHO={echo}) → {fpath}")

                    elif mode == "PH":
                        addr   = int(parts[1], 16)
                        chan   = int(parts[2])
                        fpath  = Path(parts[3])

                        # Update with your calibration points
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
    logger.info("Setting up GPIO pins (lgpio backend)")

    for aff in afferents:
        if aff["type"] == "gpio":
            for pin in aff["pins"]:
                GPIO.setup(pin, GPIO.IN)
            logger.debug(f"GPIO bus set as inputs on pins {aff['pins']}")

        elif aff["type"] == "ultrasonic":
            GPIO.setup(aff["trig"], GPIO.OUT)
            GPIO.setup(aff["echo"], GPIO.IN)
            logger.debug(f"Ultrasonic TRIG pin {aff['trig']} set as OUT, ECHO pin {aff['echo']} set as IN")

        # 1-Wire and pH (I2C) do not require direct GPIO setup here.

    for eff in efferents:
        GPIO.setup(eff["pin"], GPIO.OUT)
        logger.debug(f"Efferent pin {eff['pin']} set as OUT")

    logger.info("GPIO setup complete")

# ——————————————————————————————————————————————————————————————————————————————
# read_gpio_to_bus: collapse multiple GPIO inputs into one integer
# ——————————————————————————————————————————————————————————————————————————————
def read_gpio_to_bus(pins):
    val = 0
    for i, pin in enumerate(pins):
        bit   = GPIO.input(pin)
        shift = len(pins) - 1 - i
        val  |= (bit << shift)
    logger.debug(f"GPIO bus pins {pins} read value {val}")
    return val

# ——————————————————————————————————————————————————————————————————————————————
# read_ultrasonic: fire HC-SR04 and measure echo pulse width
# ——————————————————————————————————————————————————————————————————————————————
def read_ultrasonic(trig_pin, echo_pin):
    logger.debug(f"Ultrasonic: triggering on pin {trig_pin}")

    # Ensure a clean low start
    GPIO.output(trig_pin, GPIO.LOW)
    time.sleep(0.002)

    # 10 µs high pulse
    GPIO.output(trig_pin, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(trig_pin, GPIO.LOW)

    start_time = time.time()
    # Wait for rising edge
    while GPIO.input(echo_pin) == 0:
        if time.time() - start_time > US_TIMEOUT:
            logger.warning(f"Ultrasonic timeout waiting for echo HIGH on pin {echo_pin}")
            return None
    pulse_start = time.time()

    # Wait for falling edge
    while GPIO.input(echo_pin) == 1:
        if time.time() - pulse_start > US_TIMEOUT:
            logger.warning(f"Ultrasonic timeout waiting for echo LOW on pin {echo_pin}")
            return None
    pulse_end = time.time()

    pulse_duration = pulse_end - pulse_start
    distance_cm = round(pulse_duration * 17150, 2)
    logger.info(f"Ultrasonic (TRIG={trig_pin}, ECHO={echo_pin}) distance: {distance_cm} cm")
    return distance_cm

# ——————————————————————————————————————————————————————————————————————————————
# read_ph: use ADS1115 to read channel and convert voltage → pH
# ——————————————————————————————————————————————————————————————————————————————
ads_buses = {}

ADS_CONV_REG    = 0x00
ADS_CONFIG_REG  = 0x01

CONFIG_OS_SINGLE    = 0x8000
CONFIG_MUX_OFFSET   = 12
CONFIG_PGA_4_096V   = 0x0200
CONFIG_MODE_SINGLE  = 0x0100
CONFIG_DR_128SPS    = 0x0080
CONFIG_COMP_QUE_N   = 0x0003

def read_ph_smbus(aff):
    addr = aff["addr"]
    chan = aff["chan"]  # 0..3

    if addr not in ads_buses:
        try:
            bus = smbus2.SMBus(1)  # Pi I2C-1
            ads_buses[addr] = bus
        except FileNotFoundError:
            logger.error(f"I2C bus /dev/i2c-1 not found. Is I²C enabled?")
            return None

    bus = ads_buses[addr]

    mux = 0x04 + chan  # A0→4, A1→5, A2→6, A3→7
    config = (
        CONFIG_OS_SINGLE |
        (mux << CONFIG_MUX_OFFSET) |
        CONFIG_PGA_4_096V |
        CONFIG_MODE_SINGLE |
        CONFIG_DR_128SPS |
        CONFIG_COMP_QUE_N
    )

    try:
        msb = (config >> 8) & 0xFF
        lsb = config & 0xFF
        bus.write_i2c_block_data(addr, ADS_CONFIG_REG, [msb, lsb])
    except OSError as e:
        logger.error(f"I2C write to ADS1115@0x{addr:02X} failed: {e}")
        return None

    time.sleep(1.0/128 + 0.005)

    try:
        data = bus.read_i2c_block_data(addr, ADS_CONV_REG, 2)
    except OSError as e:
        logger.error(f"I2C read from ADS1115@0x{addr:02X} failed: {e}")
        return None

    raw = (data[0] << 8) | data[1]
    if raw & 0x8000:
        raw -= 1 << 16

    voltage = raw * 0.000125  # 4.096V/32768
    pH_val = aff["slope"] * voltage + aff["offset"]
    logger.info(f"pH (I2C@0x{addr:02X} CH{chan}): raw={raw}, V={voltage:.3f} → pH={pH_val:.2f}")
    return pH_val

# ——————————————————————————————————————————————————————————————————————————————
# bridge_once: perform one cycle of reading all afferents and writing all efferents
# ——————————————————————————————————————————————————————————————————————————————
def bridge_once():
    # Efferents
    for eff in efferents:
        try:
            raw = eff["path"].read_text().strip()
            val = int(raw)
            GPIO.output(eff["pin"], GPIO.HIGH if val else GPIO.LOW)
            logger.info(f"Efferent pin {eff['pin']} set to {'HIGH' if val else 'LOW'} (file {eff['path']})")
        except Exception:
            logger.exception(f"Efferent write error on {eff['path']}")

    # Afferents
    for aff in afferents:
        try:
            if aff["type"] == "gpio":
                val = read_gpio_to_bus(aff["pins"])
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
                    logger.info(f"1-Wire sensor {aff['sensor']} temperature: {val:.2f} °C")
                except Exception:
                    logger.exception(f"Reading 1-Wire sensor {aff['sensor']}")
                    continue

            elif aff["type"] == "ultrasonic":
                val = read_ultrasonic(aff["trig"], aff["echo"])
                if val is None:
                    continue

            elif aff["type"] == "ph":
                val = read_ph_smbus(aff)
                if val is None:
                    continue

            else:
                continue

            write_atomic(aff["path"], f"{val}\n")
            logger.info(f"Afferent '{aff['type']}' → wrote {val} to {aff['path']}")

        except Exception:
            logger.exception(f"Afferent write error on {aff['path']}")

# ——————————————————————————————————————————————————————————————————————————————
# Main loop
# ——————————————————————————————————————————————————————————————————————————————
CONTROL_PANEL_FLAG_FILE = Path("Control_Panel_Flag.ssv")

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
                    #write_atomic(CONTROL_PANEL_FILE,       "eval Testermon.txt 0.0\n")
                    write_atomic(CONTROL_PANEL_FLAG_FILE,  "1\n")
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
        logger.info("Entering main bridge loop")
        bridge_loop()
    except KeyboardInterrupt:
        logger.info("Interrupted by user (KeyboardInterrupt), exiting")
    finally:
        try:
            GPIO.cleanup()
        finally:
            logger.info("GPIO cleaned up, driver exiting")
