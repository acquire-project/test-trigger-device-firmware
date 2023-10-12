struct Camera {
  int out, in, gate;
  struct {
    int in,out;
  } counts;
} cameras [] =  {
  // Using pin ids (not gpio ids)
 [0] = { .out = 1, .in = 5, .gate = 9},
 [1] = { .out = 15, .in = 19, .gate = 21},
 [2] = { .out = 34, .in = 29, .gate = 25},
};

void setup() {
  Serial.begin(9600); 
  Serial.setTimeout(100); // milliseconds

  for(const auto & camera: cameras) {
    pinMode(camera.in,INPUT);
    pinMode(camera.gate,INPUT);
    pinMode(camera.out,OUTPUT);
  }
}

arduino::String input;

// Appends any bytes from the serial port to the input stream
void read() {
  if(Serial.available()) {
    // Get all the data available, up to the timeout
    // Avoid readStringUntil() since it also times out, but consumes the terminator.
    input += Serial.readString();     
  }
}

enum Kind {
  EMIT,
  QOUT,
  QIN,
};

struct Command {
  enum Kind kind;
  int camera_id;
}

void eval() {

}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    incoming = Serial.readString();

    // say what you got:
    Serial.printf("I received: %s\n\r\n",incoming.c_str());
  }
}