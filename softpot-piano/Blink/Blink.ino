#include <avr/pgmspace.h>

#define BUZZER 9
#define REST   0
#define N      38

#define EPS_JUMP     18
#define EPS_NEIGHBOR 22

#define B4  494
#define C5  523
#define CS5 554
#define D5  587
#define DS5 622
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define GS5 831
#define A5  880
#define AS5 932
#define B5  988
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568

const int melody[][2] PROGMEM = {
  {E6,N},{DS6,N},{D6,N},{CS6,N},{C6,N},{B5,N},{AS5,N},{A5,N},
  {GS5,N},{G5,N},{FS5,N},{F5,N},{E5,N},{DS5,N},{D5,N},{CS5,N},
  {C5,N},{CS5,N},{D5,N},{DS5,N},{E5,N},{F5,N},{FS5,N},{G5,N},
  {GS5,N},{A5,N},{AS5,N},{B5,N},{C6,N},{CS6,N},{D6,N},{DS6,N},

  {E6,N},{DS6,N},{D6,N},{CS6,N},{D6,N},{CS6,N},{C6,N},{B5,N},
  {C6,N},{B5,N},{AS5,N},{A5,N},{AS5,N},{A5,N},{GS5,N},{G5,N},
  {FS5,N},{G5,N},{GS5,N},{A5,N},{AS5,N},{B5,N},{C6,N},{CS6,N},
  {D6,N},{DS6,N},{E6,N*2},{B5,N*2},{E6,N*3},{REST,N*2},

  {E5,N},{F5,N},{FS5,N},{G5,N},{GS5,N},{A5,N},{AS5,N},{B5,N},
  {A5,N},{GS5,N},{G5,N},{FS5,N},{F5,N},{E5,N},{DS5,N},{D5,N},
  {CS5,N},{D5,N},{DS5,N},{E5,N},{F5,N},{FS5,N},{G5,N},{GS5,N},
  {A5,N},{AS5,N},{B5,N},{C6,N},{B5,N},{AS5,N},{A5,N*2},{E5,N*3},

  {E6,N},{F6,N},{E6,N},{DS6,N},{E6,N},{DS6,N},{D6,N},{CS6,N},
  {D6,N},{CS6,N},{C6,N},{B5,N},{C6,N},{CS6,N},{D6,N},{DS6,N},
  {E6,N},{FS6,N},{G6,N},{FS6,N},{E6,N},{DS6,N},{D6,N},{CS6,N},
  {C6,N},{B5,N},{AS5,N},{A5,N},{GS5,N},{A5,N},{AS5,N},{B5,N},

  {C6,N},{B5,N},{AS5,N},{A5,N},{GS5,N},{G5,N},{FS5,N},{F5,N},
  {E5,N},{DS5,N},{D5,N},{CS5,N},{D5,N},{DS5,N},{E5,N},{F5,N},
  {FS5,N},{G5,N},{GS5,N},{A5,N},{AS5,N},{B5,N},{C6,N},{CS6,N},
  {D6,N},{DS6,N},{E6,N},{F6,N},{E6,N},{DS6,N},{D6,N},{CS6,N},

  {E6,N},{DS6,N},{D6,N},{CS6,N},{C6,N},{B5,N},{AS5,N},{A5,N},
  {GS5,N},{G5,N},{FS5,N},{F5,N},{E5,N},{DS5,N},{D5,N},{CS5,N},
  {C5,N},{B4,N},{C5,N},{CS5,N},{D5,N},{DS5,N},{E5,N},{F5,N},
  {FS5,N},{G5,N},{GS5,N},{A5,N},{AS5,N},{B5,N},{C6,N},{CS6,N},
  {D6,N},{DS6,N},{E6,N*2},{B5,N*2},{E5,N*8},{REST,N*6}
};

const int NUM = sizeof(melody) / sizeof(melody[0]);

const int pool[] = {
  B4, C5, CS5, D5, DS5, E5, F5, FS5, G5, GS5,
  A5, AS5, B5, C6, CS6, D6, DS6, E6, F6, FS6, G6
};
const int POOL = 21;

const char* poolName[] = {
  "B4","C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5",
  "A5","A#5","B5","C6","C#6","D6","D#6","E6","F6","F#6","G6"
};

int countTable[21][21];
bool trained = false;

int freqToIdx(int f) {
  for (int i = 0; i < POOL; i++) {
    if (pool[i] == f) return i;
  }
  return -1;
}

void playFreq(int f, int d) {
  if (f == REST || d <= 0) {
    delay(d);
    return;
  }
  tone(BUZZER, f, d);
  delay(d);
  noTone(BUZZER);
}

void learnPair(int fromF, int toF) {
  int a = freqToIdx(fromF);
  int b = freqToIdx(toF);
  if (a < 0 || b < 0) return;
  countTable[a][b]++;
}

void printBrain() {
  Serial.println();
  Serial.println(F("nonzero pairs (from -> to  count)"));
  int pairs = 0;
  for (int i = 0; i < POOL; i++) {
    for (int j = 0; j < POOL; j++) {
      if (countTable[i][j] > 0) {
        Serial.print(poolName[i]);
        Serial.print(F(" -> "));
        Serial.print(poolName[j]);
        Serial.print(F("  x"));
        Serial.println(countTable[i][j]);
        pairs++;
      }
    }
  }
  Serial.print(F("nonzero cells: "));
  Serial.println(pairs);
  Serial.println();
}

void playOriginalAndLearn() {
  int prev = REST;
  for (int i = 0; i < NUM; i++) {
    int f = pgm_read_word(&melody[i][0]);
    int d = pgm_read_word(&melody[i][1]);
    playFreq(f, d);
    if (f != REST && prev != REST) learnPair(prev, f);
    if (f != REST) prev = f;
  }
}

int inventNext(int fromF) {
  int a = freqToIdx(fromF);
  if (a < 0) a = random(POOL);

  int roll = random(100);
  if (roll < EPS_JUMP) return pool[random(POOL)];

  if (roll < EPS_JUMP + EPS_NEIGHBOR) {
    int n = a + (random(2) ? 1 : -1);
    if (n < 0) n = 1;
    if (n > POOL - 1) n = POOL - 2;
    return pool[n];
  }

  // smoothed weights: every next note possible
  long total = 0;
  for (int j = 0; j < POOL; j++) total += (countTable[a][j] + 1);

  long r = random(total);
  long run = 0;
  for (int j = 0; j < POOL; j++) {
    run += (countTable[a][j] + 1);
    if (r < run) return pool[j];
  }
  return pool[a];
}

int pickStart() {
  long mass = 0;
  for (int i = 0; i < POOL; i++) {
    for (int j = 0; j < POOL; j++) mass += countTable[i][j];
  }
  if (mass == 0) return E6;
  long r = random(mass);
  long run = 0;
  for (int i = 0; i < POOL; i++) {
    long row = 0;
    for (int j = 0; j < POOL; j++) row += countTable[i][j];
    run += row;
    if (r < run) return pool[i];
  }
  return E6;
}

void playInvented() {
  int nNotes = 48 + random(49);
  int f = pickStart();

  Serial.print(F("invented start="));
  int si = freqToIdx(f);
  Serial.print(si >= 0 ? poolName[si] : "?");
  Serial.print(F("  len="));
  Serial.println(nNotes);

  for (int i = 0; i < nNotes; i++) {
    int dur = N - 10 + random(21);
    playFreq(f, dur);
    int idx = freqToIdx(f);
    Serial.print(idx >= 0 ? poolName[idx] : "?");
    Serial.print(' ');
    if ((i + 1) % 16 == 0) Serial.println();
    f = inventNext(f);
  }
  Serial.println();
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  randomSeed(analogRead(A0) + millis());
  Serial.println(F("ready 9600"));
}

void loop() {
  if (!trained) {
    Serial.println(F("=== original (train once) ==="));
    playOriginalAndLearn();
    printBrain();
    trained = true;
  }

  delay(600);
  Serial.println(F("=== invented ==="));
  playInvented();
  delay(2000 + random(1500));
}
