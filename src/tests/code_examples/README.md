# Demo Examples

This folder contains standalone demo programs to test individual hardware components and features.

## Available Demos

1. **LVGL RTOS Demo** - Tests the display and touch screen with LVGL widgets
2. **QR Code Reader Demo** - Tests the QR code scanner
3. **Fingerprint Demo** - Tests the fingerprint sensor
4. **SD Card FatFS Demo** - Tests SD card read/write operations
5. **WiFi Demo** - Tests WiFi connectivity and networking features

## How to Run a Demo

### Step 1: Select the Demo

Edit the `CMakeLists.txt` file in the project root and change the `BUILD_TARGET` variable:

```cmake
set(BUILD_TARGET "SDCARD_DEMO" CACHE STRING "...")
```

Available options:
- `"LVGL_DEMO"` - LVGL RTOS demo
- `"QR_DEMO"` - QR code reader demo
- `"FINGERPRINT_DEMO"` - Fingerprint demo
- `"SDCARD_DEMO"` - SD card FatFS demo
- `"WIFI_DEMO"` - WiFi connectivity demo
- `"MAIN"` - Main application (default)

### Step 2: Compile the Project

Run the "Compile Project" task:
- Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
- Type "Tasks: Run Task"
- Select "Compile Project"

Or use the terminal:
```bash
cmake -B build
ninja -C build
```

### Step 3: Upload to Pico

Run the "Run Project" task:
- Press `Ctrl+Shift+P`
- Type "Tasks: Run Task"
- Select "Run Project"

Or manually copy the `.uf2` file:
1. Hold BOOTSEL button while connecting the Pico
2. Copy `build/medical-clinic-checkin-pico.uf2` to the RPI-RP2 drive

### Step 4: Monitor Serial Output

Connect to the USB serial port to see debug output:

```bash
minicom -D /dev/ttyACM0
```

Or use any serial terminal (115200 baud, 8N1).

---

## Demo Details

### 1. LVGL RTOS Demo (`lvgl_rtos_demo.cpp`)

**What it does:**
- Initializes the ST7796 display driver
- Initializes the FT6336U touch controller
- Runs LVGL's demo widgets with FreeRTOS

**Hardware required:**
- ST7796 display (480x320)
- FT6336U touch panel
- SPI and I2C connections

**Expected output:**
- Display shows LVGL demo widgets
- Touch screen is responsive
- LED blinks once per second

---

### 2. QR Code Reader Demo (`qr_code_reader_demo.cpp`)

**What it does:**
- Initializes the GM67 QR code scanner
- Continuously scans for QR codes
- Prints scanned data to serial console

**Hardware required:**
- GM67 QR code scanner module
- UART connection (default: UART1)

**Expected output:**
```
=== QR Scanner Test ===
UART1 @ 9600 baud (TX=4, RX=5)

Initializing scanner...
Ready!

>>> QR: https://example.com
>>> QR: 1234567890
```

**Usage:**
- Point QR codes at the scanner
- Scanned data appears in the serial console
- LED blinks every 500ms

---

### 3. Fingerprint Demo (`fingerprint_demo.cpp`)

**What it does:**
- Initializes the R307S fingerprint sensor
- Interactive menu to enroll, verify, search, and delete fingerprints
- Manages fingerprint database

**Hardware required:**
- R307S fingerprint sensor module
- UART connection (default: UART0)

**Expected output:**
```
=============================
  R307S Fingerprint Demo
=============================
UART0 @ 57600 baud (TX=0, RX=1)

Initializing R307S fingerprint sensor...
Sensor ready
Current stored fingerprints: 0

====== Fingerprint Menu ====
1. Enroll new fingerprint
2. Verify fingerprint
3. Search fingerprint
4. Delete fingerprint by ID
5. Clear all fingerprints
6. Get template count
7. Read system parameters
0. Show menu
==============================
Enter option:
```

**Menu options:**
1. **Enroll** - Register a new fingerprint with an ID (1-1000)
2. **Verify** - Check if a finger matches a specific ID
3. **Search** - Find which ID matches the current finger
4. **Delete** - Remove a fingerprint by ID
5. **Clear all** - Erase entire fingerprint database
6. **Count** - Show how many fingerprints are stored
7. **Parameters** - Display sensor information

**Usage tips:**
- Follow on-screen instructions carefully
- Place finger firmly on sensor
- Use same finger position for both enrollment scans
- LED blinks once per second during operation

---

### 4. SD Card FatFS Demo (`sdcard_fatfs_demo.cpp`)

**What it does:**
- Mounts SD card using FatFS library
- Performs various file operations:
  - Write text file
  - Read text file
  - Append to log file
  - Get file information
  - Show storage statistics
  - List directory contents

**Hardware required:**
- SD card module
- SPI connection
- SD card formatted as FAT32

**Expected output:**
```
--- SD Card FatFS Demo ---
Build: Nov 9 2025 10:30:00

Initializing SD card...
Mounting filesystem...
Filesystem mounted ok

Test 1: Writing to file
Opening test.txt...
File opened
Writing data...
Wrote 71 bytes
File closed

Test 2: Reading from file
Opening test.txt...
File opened

--- File content:
Hello from RP2040!
SD card is working!
Medical Clinic Check-in System
--- End

File closed

Test 3: Appending to file
[... additional tests ...]

Demo complete. Files created:
  - test.txt
  - log.txt
```

**Files created on SD card:**
- `test.txt` - Test file with sample text
- `log.txt` - Log file with timestamps

**Troubleshooting:**
If you see "f_mount failed":
- Check that SD card is properly inserted
- Verify SD card is formatted as FAT32
- Check SPI wiring connections
- Try a different SD card

---

### 5. WiFi Demo (`wifi_demo.cpp`)

**What it does:**
- Initializes the CYW43439 WiFi chip
- Connects to a WiFi network
- Demonstrates all WiFi HAL functions:
  - Network information (IP, MAC, SSID, RSSI)
  - WiFi scanning
  - DNS resolution
  - TCP echo server
  - Signal strength monitoring

**Hardware required:**
- Raspberry Pi Pico W (with CYW43439 WiFi chip)
- WiFi network (2.4 GHz)

**Configuration:**
Edit `config/wifi_config.hpp` with your WiFi credentials:
```cpp
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
#define WIFI_COUNTRY_CODE "BR"  // Your country code
```

**Expected output:**
```
*****************STARTING********************

Step 1: Initializing WiFi...
Creating WiFi HAL instance...
Initializing CYW43 driver (country: BR)...
SUCCESS: WiFi initialized

Step 2: Connecting to WiFi...
Network: YourNetworkName
Security: WPA2-AES
Attempting connection (timeout: 30000 ms)...
SUCCESS: Connected to network

Step 3: Displaying network info...
Querying network interface...
IP Address: 192.168.0.11
MAC Address: 28:CD:C1:XX:XX:XX
SSID: YourNetworkName
Signal Strength: -46 dBm
SUCCESS: Network info displayed

Step 4: Scanning WiFi networks...
Starting active scan (timeout: 10s)...
Found 10 network(s)

#   SSID                             Ch   Signal    Security
--- -------------------------------- ---- -------- ----------
1   YourNetworkName                  6    -46 dBm  WPA2
2   NeighborWiFi                     11   -62 dBm  WPA2
3   PublicWiFi                       1    -75 dBm  OPEN
...
SUCCESS: WiFi scan completed

Step 5: Testing DNS connectivity...
Target: google.com
Resolving hostname...
SUCCESS: DNS resolved

Step 6: Starting TCP echo server...
Creating TCP server on port 7...
Echo server on port 7
SUCCESS: Echo server running on port 7
Usage: telnet <pico-ip> 7

Step 7: Sampling RSSI...
Collecting 5 signal strength samples (1s interval)...
Sample 1: RSSI = -46 dBm
Sample 2: RSSI = -47 dBm
Sample 3: RSSI = -46 dBm
Sample 4: RSSI = -45 dBm
Sample 5: RSSI = -46 dBm
SUCCESS: RSSI samples complete

Demo completed.
Cleaning up WiFi HAL...
```

**Features tested:**
1. ✅ WiFi initialization with country code
2. ✅ WPA2 network connection
3. ✅ DHCP IP address assignment
4. ✅ MAC address retrieval
5. ✅ Active network scanning
6. ✅ DNS hostname resolution
7. ✅ TCP server (echo on port 7)
8. ✅ RSSI signal strength measurement
9. ✅ LED control via CYW43 chip

**Testing the TCP echo server:**
After the demo starts the server, you can test it from your computer:
```bash
# Replace <pico-ip> with the IP address shown in the output
telnet 192.168.0.11 7

# Type any text and press Enter
# The server will echo it back
```

**Signal strength (RSSI) interpretation:**
- `-30` to `-50 dBm`: Excellent signal
- `-50` to `-70 dBm`: Good signal
- `-70` to `-80 dBm`: Weak signal
- Below `-80 dBm`: Very weak (may have connection issues)

**Troubleshooting:**

If you see **"ERROR: WiFi initialization failed"**:
- Make sure you're using a Pico W (not a regular Pico)
- Verify the country code is valid (e.g., "BR", "US", "JP")

If you see **"ERROR: Connection failed"**:
- Check SSID and password in `wifi_config.hpp`
- Ensure the network is 2.4 GHz (5 GHz is not supported)
- Verify WPA2 security mode matches your network
- Check if signal strength is sufficient (> -80 dBm)
- Increase timeout if network is slow to respond

**Additional documentation:**
- See `/docs/dev/wifi/como_funciona_wifi.md` for detailed WiFi architecture
- HAL implementation: `/src/hal/rp2040/hal_wifi_rp2040.cpp`
- lwIP configuration: `/config/lwipopts.h`

---

## Switching Back to Main Application

To return to the main application, change `BUILD_TARGET` back to `"MAIN"`:

```cmake
set(BUILD_TARGET "MAIN" CACHE STRING "...")
```

Then recompile and upload.

---

## Hardware Connections

Refer to `config/pin_config.hpp` for the exact GPIO pin assignments for each component.

## Troubleshooting

**Problem:** Demo doesn't compile
- **Solution:** Make sure you saved the `CMakeLists.txt` file after changing `BUILD_TARGET`

**Problem:** No serial output
- **Solution:** USB stdio is enabled. Connect to the USB serial port (usually `/dev/ttyACM0` on Linux)

**Problem:** Hardware not responding
- **Solution:** Check wiring and power connections. Verify pin assignments in `config/pin_config.hpp`

**Problem:** Pico won't connect
- **Solution:** Hold BOOTSEL button while plugging in USB to enter bootloader mode
