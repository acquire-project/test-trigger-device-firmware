struct Camera {
  int out, in, gate;
  struct {
    int in,out;
  } counts;
  int should_emit;
} cameras [] =  {
  // Using gpio ids
 [0] = { .out = 0, .in = 3, .gate = 6},
 [1] = { .out = 11, .in = 14, .gate = 16},
 [2] = { .out = 19, .in = 22, .gate = 28},
};


#define ERR(msg) Serial.println("ERR: " # msg)
#define countof(e) (sizeof(e)/sizeof(*e))

//
//    COMMANDS
//

int emit(int camera_id) {
  cameras[camera_id].should_emit = 1;
  Serial.println("emit "+String(camera_id,DEC));
  return 0;
}

int query_out(int camera_id) {
  Serial.println("qout "+String(camera_id,DEC)+" "+String(cameras[camera_id].counts.out));
  return 0;
}

int query_in(int camera_id) {
  Serial.println("qin "+String(camera_id,DEC)+" "+String(cameras[camera_id].counts.in));
  return 0;
}

//
//    PARSING
//

enum Result {
  OK,
  ERR
};

struct Command {
  int (*cmd)(int camera_id);
  int arg;
};

struct ParseResult {
  enum Result result;
  String rest;
};

// Consume digits, storing the corresponding integer in `out`.
struct ParseResult number(int* out, struct ParseResult input) {
  if(input.result!=OK) 
  {
    return input;
  }

  int cur=0,end=input.rest.length();
  while(cur<end && isDigit(input.rest[cur])) ++cur;
  
  if(cur==0) {
    // no digits
    ERR("Expected digits");
    input.result=ERR;
    return input;
  } else {
    // consume digits and translate to int
    *out = input.rest.substring(0,cur).toInt();
    return (struct ParseResult) {
      .result = OK,
      .rest = input.rest.substring(cur,end)
    };
  }
}

// Consume alpha characters, storing the corresponding lowercase string in `out`.
struct ParseResult alphas(String* out, struct ParseResult input) {
  if(input.result!=OK) 
  {
    return input;
  }

  int cur=0,end=input.rest.length();
  while(cur<end && isAlpha(input.rest[cur])) ++cur;
  
  if(cur==0) {
    // nothing consumed
    ERR("Expected alphas");
    input.result=ERR;
    return input;
  } else {
    // consume alphas and translate to lowercase
    *out = input.rest.substring(0,cur);
    out->toLowerCase();
    return (struct ParseResult) {
      .result = OK,
      .rest = input.rest.substring(cur,end)
    };
  }
}

struct ParseResult whitespace(struct ParseResult input) {
  if(input.result!=OK) 
  {
    return input;
  }
  input.rest.trim();
  return input;
}

// Syntax: ([a-z]+)\w(\d+), output corresponding Command in `out`
struct ParseResult command(struct Command * out, struct  ParseResult input) {
  static const struct {String name; int (*cmd)(int camera_id);} commands[] = {
    {"emit", emit},
    {"qout", query_out},
    {"qin", query_in},
  };

  if(input.result!=OK) 
  {
    return input;
  }

  String cmd;
  int camera_id=-1;
  ParseResult result = number(&camera_id,whitespace(alphas(&cmd,input)));
  if(result.result!=OK)  {
    ERR("Command not well-formed");
    return result;
  }

  for(const auto c: commands) {
    if(cmd == c.name) {
      *out = (struct Command) {
        .cmd = c.cmd,
        .arg = camera_id
      };
      return result;
    }
  }
  // no command recognized
  ERR("Command not recognized");
  input.result=ERR;
  return input;
}

struct ParseResult line(struct ParseResult *out, struct ParseResult input) {
  out->result = ERR;
  if(input.result!=OK) 
  {
    return input;
  }
  int i = input.rest.indexOf('\n');
  if(i<0) {
    input.result=ERR;
    return input;
  } 

  *out = {
    .result = OK,
    .rest = input.rest.substring(0,i)
  };

  input.rest=input.rest.substring(i+1,input.rest.length());
  return input;
}

//
// EXECUTION
//

void setup() {
  Serial.begin(9600); 
  Serial.setTimeout(100); // milliseconds

  for(const auto & camera: cameras) {
    pinMode(camera.in,INPUT);
    pinMode(camera.gate,INPUT);
    pinMode(camera.out,OUTPUT);
  }
}

// Appends any bytes from the serial port to the input stream
void read(String& inbox) {
  if(Serial.available()) {
    // Get all the data available, up to the timeout
    // Avoid readStringUntil() since it also times out, but consumes the terminator.
    inbox += Serial.readString();     
  }
}

void eval(String& inbox) {
  struct ParseResult p = {.result=OK,.rest=inbox};
  do {
    // Consume the line, executing a command if recognized
    struct ParseResult inner;
    struct Command cmd;    
    p = line(&inner,p);
    inner = command(&cmd,inner);
    if(inner.result==OK) {      
      // Execute 
      if(cmd.arg<countof(cameras))
      cmd.cmd(cmd.arg);
    }
  } while(p.result==OK);
  inbox = p.rest;
}

void update() {
  
  for(auto& c: cameras) {
    if(c.should_emit && digitalRead(c.gate)) {
      digitalWrite(c.out,1);
      c.should_emit=0;
      ++c.counts.out;
    } else {
      digitalWrite(c.out,0);
    }

    if(digitalRead(c.in))
      ++c.counts.in;
  }
}

void loop() {
  arduino::String inbox;
  int last_len=0;
  while(1) {
    read(inbox); // blocks for Serial.setTimeout(ms)
    if(inbox.length()!=last_len) {
      eval(inbox);
      last_len = inbox.length();
    }
    update();
  }
}
