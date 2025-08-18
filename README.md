# NanoHat OLED with YouTube stats

[![Build](https://img.shields.io/badge/build-make-blue)](https://www.gnu.org/software/make/)  
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)  
[![Platform](https://img.shields.io/badge/platform-Armbian-orange)](https://www.armbian.com/)

Resource-friendly NanoHat OLED and GPIO button control for Armbian.
Displays YouTube channel statistics (views, subs, videos) on a small SSD1306 OLED.

## Acknowledgements
This project uses ideas and code from:

- https://github.com/crouchingtigerhiddenadam/nano-hat-oled-armbian
- https://github.com/Digilent/linux-userspace-examples
- https://github.com/OnionIoT/i2c-exp-driver
- https://github.com/Narukara/SSD1306
- https://android.googlesource.com/kernel/common/+/refs/heads/upstream-master/tools/gpio
- https://www.asciiart.eu

## Getting Started

### Prerequisites (enable I²C)
Edit:
```
sudo nano /boot/armbianEnv.txt
```
Add `i2c0` to the `overlays=` line, e.g.:
```
overlays=i2c0 usbhost1 usbhost2
```
Save (`ctrl+x`, `ctrl+y` and `enter`) and reboot:
```
sudo reboot now
```

### Dependencies
Install the dependencies from APT:
```
sudo apt -y install \
  gcc \
  git \
  libssl-dev \
  make \
  python3
```

### Get the Code
Clone from Github:
```
cd /tmp
git clone https://github.com/corazgrubszy/nanohat-oled.git
cd nanohat-oled/src
```

### Configure (generate local, obfuscated config)
Do **not** edit `yt.c`. Generate `src/yt_config.h` with your own values:
```
./tools/gen_yt_config.py \
  --channel-id "YOUR_CHANNEL_ID" \
  --api-key    "YOUR_API_KEY" \
  --out src/yt_config.h \
  --key 0x5A
```
Notes:
- `src/yt_config.h` is **git-ignored** (kept local).
- Obfuscation key can be changed: `--key 0x5A`.
- See `src/yt_config.h.template` for what gets generated.
- If you don’t know your key/ID:  
  [How to fetch statistics from YouTube API](https://hackernoon.com/how-to-fetch-statistics-from-youtube-api-using-python)

### Build
```
make
```

## Install & Service

### One-time systemd setup (manual)
Create the runtime directory:
```
sudo mkdir -p /usr/share/nanohatoled
```
Create the service unit:
```
sudo nano /etc/systemd/system/ytstats.service
```
Paste the following content:
```
[Unit]
Description=YouTube stats OLED display
After=network.target
  
[Service]
ExecStart=/usr/bin/nice -n 10 /usr/share/nanohatoled/ytstats
Restart=on-failure
User=root
  
[Install]
WantedBy=multi-user.target
```
Save the file and enable the service to start on boot:
```  
sudo systemctl daemon-reexec
sudo systemctl enable ytstats.service
sudo systemctl start ytstats.service
```

### Install / Update (subsequent iterations)
The **Makefile** automates updates. It stops the service, copies the new binary, sets permissions, and starts the service again:
```
sudo make install
```

### Uninstall
```
sudo make uninstall
```

### Clean build artifacts
```
make clean
```

### Check status/logs
```
sudo systemctl status ytstats.service
journalctl -u ytstats.service
```

## Troubleshooting

### BakeBit / NanoHatOLED conflicts
This project does **not** use FriendlyElec's BakeBit / NanoHatOLED software. If it's installed, disable it:
```
sudo nano /etc/rc.local
```
Comment out the line:
```
# /usr/local/bin/oled-start
exit 0
```
Save and reboot:
```
sudo reboot now
```

### Common issues
- **Service won’t start**: run the binary by hand to see stderr:
  ```
  /usr/share/nanohatoled/ytstats
  ```
- **No I²C device**: confirm overlay and bus:
  ```
  ls /dev/i2c-*     # expect /dev/i2c-0 on NanoPi (after enabling i2c0)
  ```
- **TLS errors**: ensure `libssl-dev` installed at build time; device has correct time (NTP) for TLS validation.

## Development notes
- Build: `make`  
- Update service with new build: `sudo make install`  
- Remove: `sudo make uninstall`  
- Clean: `make clean`  

PRs welcome. Keep comments in English to help others reuse the code.
