import RPi.GPIO as GPIO
from pathlib import Path
import time

CONFIG_FILE = "pinout.cfg"
POLL_INTERVAL = 0.01  # seconds
UPDATE_CHECK_INTERVAL = 1.0  # seconds

CONTROL_PANEL_FILE = Path("Control_Panel.ssv")
CONTROL_PANEL_FLAG_FILE = Path("Control_Panel_Flag.ssv")

afferents = []
efferents = []

def parse_config(path):
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            parts = line.split()
            mode = parts[0]
            pins = list(map(int, parts[1:-1]))
            fpath = Path(parts[-1])

            if mode == "A":
                afferents.append({'type': 'gpio', 'pins': pins, 'path': fpath})
            elif mode == "E":
                if len(pins) != 1:
                    print(f"[WARN] Efferent must have 1 pin: {line}")
                    continue
                efferents.append({'pin': pins[0], 'path': fpath})
            elif mode == "A1W":
                # Example line: A1W 28-000005e2fdc3 temp_sensor_1.txt
                sensor_id = parts[1]
                fpath = Path(parts[2])
                afferents.append({'type': '1wire', 'sensor': sensor_id, 'path': fpath})

def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    for aff in afferents:
        for pin in aff['pins']:
            GPIO.setup(pin, GPIO.IN)
    for eff in efferents:
        GPIO.setup(eff['pin'], GPIO.OUT)

def read_gpio_to_bus(pins):
    bits = [GPIO.input(pin) for pin in pins]
    val = 0
    for i, bit in enumerate(bits):
        shift = len(bits) - 1 - i
        val |= (bit << shift)
    return val

def bridge_once():
    # Efferents: file → GPIO
    for eff in efferents:
        try:
            with open(eff['path'], "r") as f:
                val = int(f.read().strip())
                GPIO.output(eff['pin'], GPIO.HIGH if val else GPIO.LOW)
        except:
            pass

    
    for aff in afferents:
        try:
            # Afferents: GPIO → file, 1-Wire → file
            if aff['type'] == 'gpio':
                val = read_gpio_to_bus(aff['pins'])
            elif aff['type'] == '1wire':
                sensor_path = f"/sys/bus/w1/devices/{aff['sensor']}/w1_slave"
                with open(sensor_path, "r") as f:
                    lines = f.readlines()
                    if lines[0].strip().endswith("YES"):
                        temp_str = lines[1].split("t=")[-1].strip()
                        val = float(temp_str) / 1000.0
                    else:
                        continue  # bad CRC, skip
            else:
                continue  # unknown type

            with open(aff['path'], "w") as f:
                f.write(f"{val}\n")
        except:
            pass

def bridge_loop():
    last_update_check = time.time()

    while True:
        current_time = time.time()
        if current_time - last_update_check >= UPDATE_CHECK_INTERVAL:
            last_update_check = current_time

            try:
                if CONTROL_PANEL_FLAG_FILE.exists():
                    with open(CONTROL_PANEL_FLAG_FILE, "r") as f:
                        if f.read().strip() != "1":
                            bridge_once()

                            with open(CONTROL_PANEL_FILE, "w") as f:
                                f.write("eval Testermon.txt 0.0\n")

                            with open(CONTROL_PANEL_FLAG_FILE, "w") as f:
                                f.write("1\n")
            except:
                pass

        time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    parse_config(CONFIG_FILE)
    setup_gpio()
    bridge_loop()
