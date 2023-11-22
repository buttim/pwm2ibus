// Based on ppm_to_ibus_serial.ino by wdcossey

#define IBUS_FRAME_LENGTH 0x20                                  // iBus packet size (2 byte header, 14 channels x 2 bytes, 2 byte checksum)
#define IBUS_COMMAND40 0x40                                     // Command is always 0x40
#define IBUS_MAXCHANNELS 14                                     // iBus has a maximum of 14 channels

#define IBUS_DEFAULT_VALUE (uint16_t)1500
#define MAX_CHANNELS 8

volatile uint16_t channel_data[MAX_CHANNELS] = { 0 };

byte serial_buffer[IBUS_FRAME_LENGTH] = { 0 };
int buffer_index = 0;

void setup() {
  setup_pwmRead();
  pinMode(LED_BUILTIN,OUTPUT);
	Serial.begin(115200);
}

void WriteSerial() {
	uint16_t ibus_checksum = ((uint16_t)0xFFFF);

  //      Serial.println("Channel Count: " + ((String)currChannelCount));

  buffer_index = 0;

  // Write the IBus buffer length
  serial_buffer[buffer_index++] = (byte)IBUS_FRAME_LENGTH;
  // Write the IBus Command 0x40
  serial_buffer[buffer_index++] = (byte)IBUS_COMMAND40;

  // Write the IBus channel data to the buffer
  for (int i = 0; i < min(MAX_CHANNELS, IBUS_MAXCHANNELS); i++) {
    serial_buffer[buffer_index++] = (byte)(channel_data[i] & 0xFF);
    serial_buffer[buffer_index++] = (byte)((channel_data[i] >> 8) & 0xFF);
  }

  // Fill the remaining buffer channels with the default value
  if (MAX_CHANNELS < IBUS_MAXCHANNELS) {
    for (int i = 0; i < IBUS_MAXCHANNELS - MAX_CHANNELS; i++) {
      serial_buffer[buffer_index++] = (byte)(IBUS_DEFAULT_VALUE & 0xFF);
      serial_buffer[buffer_index++] = (byte)((IBUS_DEFAULT_VALUE >> 8) & 0xFF);
    }
  }

  // Calculate the IBus checksum
  for (int i = 0; i < buffer_index; i++) {
    ibus_checksum -= (uint16_t)serial_buffer[i];
  }

  // Write the IBus checksum to the buffer
  serial_buffer[buffer_index++] = (byte)(ibus_checksum & 0xFF);
  serial_buffer[buffer_index++] = (byte)((ibus_checksum >> 8) & 0xFF);

  // Write the buffer to the Serial pin
  Serial.write(serial_buffer, buffer_index);

  buffer_index = 0;
}

unsigned long tLastWriteSerial=0UL, tLastRead=0;
bool connesso=false;

void loop() {
  unsigned long t=millis();

  if (!connesso) {
  }

  if (tLastRead!=0 && t-tLastRead>100) {  //controllo disconnessione (troppo tempo dall'ultima lettura)
    if (connesso)
      Serial.println("DISCONNESSO");
    connesso=false;
    digitalWrite(LED_BUILTIN,millis()%200<100?HIGH:LOW);
  }
  
	if (t-tLastWriteSerial>=10) {
    tLastWriteSerial=t;
	  WriteSerial();
  }

  if (t-tLastRead>=25) {
    if(RC_avail()) {
      connesso=true;
      tLastRead=t;
      for (int i=0;i<MAX_CHANNELS;i++) 
        channel_data[i]=map(1000*RC_decode(i+1),-1500,1500,800.0,2200.0);
    }
  }
}
