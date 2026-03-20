hciattach -s 1500000 /dev/ttyS1 any 1500000 flow nosleep
hciconfig -a
# 2. Start BlueZ ALSA audio proxy (handles A2DP/HSP audio streams)
bluealsa -p a2dp-source -p hsp-hs &
# 3. Check hci0 status
hciconfig hci0 up
hciconfig hci0 piscan  # Make device discoverable
# Enter Bluetooth interactive console
bluetoothctl

# Execute inside console:
power on          # Enable Bluetooth
agent on          # Enable pairing agent
default-agent     # Set as default agent
scan on           # Start scanning devices
# Wait for your Bluetooth headset/speaker to appear (MAC address like XX:XX:XX:XX:XX:XX)
scan off          # Stop scanning
pair DB:1B:AE:51:B1:B3   # Pair device
trust DB:1B:AE:51:B1:B3  # Trust device
connect DB:1B:AE:51:B1:B3 # Connect device
#paired-devices  # View paired devices
#disconnect DB:1B:AE:51:B1:B3 # Disconnect
#remove DB:1B:AE:51:B1:B3 # Remove pairing info
quit
# List audio devices
aplay -L
aplay -D bluealsa example.wav

#[root@canaan ~ ]#aplay  -L
#null
#    Discard all samples (playback) or generate zero samples (capture)
#default:CARD=K230I2SINNO
#    K230_I2S_INNO, Audio 9140e000.inno_codec-0
#    Default Audio Device
#sysdefault:CARD=K230I2SINNO
#    K230_I2S_INNO, Audio 9140e000.inno_codec-0
#    Default Audio Device
#[root@canaan ~ ]#
#[root@canaan ~ ]#aplay -D bluealsa example.wav
#Playing WAVE 'example.wav' : Signed 16 bit Little Endian, Rate 48000 Hz, Stereo
#^CAborted by signal Interrupt...
#aplay: pcm_write:2178: write error: Interrupted system call
#[root@canaan ~ ]#
