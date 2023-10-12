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

enum Result {
  OK,
  ERR
};

struct Command {
  enum Kind kind;
  int camera_id;
};

struct MaybeCommand {
  enum Result result;
  struct Command command;
};

struct MaybeCommand parse(const arduino::String& line) {
  struct {String name; Kind kind;} commands[] = {
    {"emit", EMIT},
    {"qout", QOUT},
    {"qin", QIN},
  };

  for(const auto& cmd: commands) {
    if(String.startsWith(cmd.name))
  }
}

void eval() {
  // consume input dividing it into "/n" lines, until the input buffer is empty
  {
    int cur=0;
    int eol=0;
    while( (eol=input.indexOf('\n'))>=0) {
      const auto line = input.substring(cur,eol);
      struct comm
    }
  }

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
