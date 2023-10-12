struct Camera {
  int out, in, gate;
  struct {
    int in,out;
  } counts;
} cameras [] =  {
  // Using gpio ids
 [0] = { .out = 0, .in = 3, .gate = 6},
 [1] = { .out = 11, .in = 14, .gate = 16},
 [2] = { .out = 19, .in = 22, .gate = 28},
};

//
//    COMMANDS
//


// commands
int emit(int camera_id) {
  Serial.print("EMIT ");
  Serial.println(camera_id,DEC);
  return 0;
}

int query_out(int camera_id) {
  Serial.print("query_out ");
  Serial.println(camera_id,DEC);
  return 0;
}

int query_in(int camera_id) {
  Serial.print("query_in ");
  Serial.println(camera_id,DEC);
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
  if(result.result==OK) 
  { 
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
    input.result==ERR;
  }

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
    input.result==ERR;
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
      cmd.cmd(cmd.arg);
    }
  } while(p.result==OK);
  inbox = p.rest;
}



void loop() {
  arduino::String inbox;
  while(1) {
    read(inbox);
    eval(inbox);
  }
}
