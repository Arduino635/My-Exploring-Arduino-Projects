Week 2: Machine learning with peizo buzzer

## Materials

- Arduino Uno 
- Passive piezo buzzer
- Jumper wires 
- SoftPot membrane strip (earlier piano attempt)
- 10 kΩ resistor (SoftPot pulldown)

**Wiring that actually ran**

| Piece | Wiring |
|---|---|
| Piezo | + → Digital Pin 9, − → GND |
| SoftPot | one end → 5V, other end → GND, wiper (middle end) → A0 and 10 kΩ → GND |

---

## What I built

An Uno plays a short Flight of the Bumblebee on a piezo, counts how often each pitch follows the one before it, and stores those counts in a transition table. Given the note just played, the table says how often each next note occurred. 

From that table it invents a new tune. At each step the program uses the note it just played and chooses the next one. The choice is random, but not uniform: notes that followed this one more often in the original are more likely to be selected. Basically, it's the probability that each note comes after the current note. The program then picks at random, but the pick is weighted: notes that followed this note more often are more likely.


Serial Monitor at 9600 prints the transition(count) table and the invented pitch names.

**Count Table**

11:58:32.384 -> ready 9600
11:58:32.384 -> === original (train once) ===
11:58:40.713 -> 
11:58:40.713 -> nonzero pairs (from -> to  count)
11:58:40.746 -> B4 -> C5  x1
11:58:40.779 -> C5 -> B4  x1
11:58:40.779 -> C5 -> C#5  x2
11:58:40.779 -> C#5 -> C5  x2
11:58:40.813 -> C#5 -> D5  x4
11:58:40.813 -> D5 -> C#5  x4
11:58:40.846 -> D5 -> D#5  x4
11:58:40.846 -> D#5 -> D5  x4
11:58:40.879 -> D#5 -> E5  x4
11:58:40.879 -> E5 -> D#5  x4
11:58:40.912 -> E5 -> F5  x5
11:58:40.912 -> E5 -> E6  x1
11:58:40.945 -> F5 -> E5  x4
11:58:40.945 -> F5 -> F#5  x5
11:58:41.072 -> F#5 -> F5  x4
11:58:41.072 -> F#5 -> G5  x6
11:58:41.072 -> G5 -> F#5  x5
11:58:41.072 -> G5 -> G#5  x6
11:58:41.072 -> G#5 -> G5  x5
11:58:41.072 -> G#5 -> A5  x7
11:58:41.072 -> A5 -> E5  x1
11:58:41.072 -> A5 -> G#5  x6
11:58:41.072 -> A5 -> A#5  x8
11:58:41.087 -> A#5 -> A5  x7
11:58:41.111 -> A#5 -> B5  x7
11:58:41.144 -> B5 -> E5  x1
11:58:41.144 -> B5 -> A5  x1
11:58:41.177 -> B5 -> A#5  x6
11:58:41.177 -> B5 -> C6  x8
11:58:41.177 -> B5 -> E6  x1
11:58:41.210 -> C6 -> B5  x8
11:58:41.210 -> C6 -> C#6  x5
11:58:41.243 -> C#6 -> C6  x5
11:58:41.243 -> C#6 -> D6  x7
11:58:41.277 -> C#6 -> E6  x1
11:58:41.277 -> D6 -> C#6  x8
11:58:41.310 -> D6 -> D#6  x5
11:58:41.310 -> D#6 -> D6  x6
11:58:41.343 -> D#6 -> E6  x6
11:58:41.343 -> E6 -> E5  x1
11:58:41.376 -> E6 -> B5  x2
11:58:41.376 -> E6 -> D#6  x7
11:58:41.376 -> E6 -> F6  x2
11:58:41.409 -> E6 -> F#6  x1
11:58:41.409 -> F6 -> E6  x2
11:58:41.443 -> F#6 -> E6  x1
11:58:41.443 -> F#6 -> G6  x1
11:58:41.476 -> G6 -> F#6  x1
11:58:41.476 -> nonzero cells: 48


**What does nonzero cells: 48 mean?**

nonzero cells: 48 means 48 entries in the 21×21 table are not zero.
The table has 21 times 21 = 441 possible pairs. Most are 0 (that succession never happened in the written bumblebee). 48 of them happened at least once, so countTable[i][j] ≥ 1, basically meaning out of the 441 possible pairs 48 of them happened at least once.

---

## Why the SoftPot piano did not work that well

The strip is one analog slider, not real keys.

- `analogRead(A0)` was chopped into 8 bins (C D E F G A B C).
- Finger position is sloppy. The log was almost only **keys 5, 6, 7** (the high end).
- Sliding across a boundary counted as a new note every few milliseconds → huge **6 ↔ 7** spam, not a melody.
- Quiet / dead zones and a missing or weak pulldown make “finger up” look like random low values.
- Hard to hit a **wide range of pitches** on purpose, so the brain only learned a high trill.

Same Markov idea can work on a SoftPot if you **press, lift, press** and use the whole strip. In practice it was a bad teacher. Three clean lifted hooks beat fifteen sloppy slides; we still barely got a scale.


* Same next-note counting, but keys from `analogRead(A0)` mapped to 8 scale notes. Teach by playing; invent after ~2.5 s idle or on D11. One song is enough. Clean lifts matter more than many repeats.

---

## Why the piezo-buzzer idea worked

The source tune is a **fixed list of frequencies** in flash. Every note is an exact Hz, not a noisy finger position.

So the pair table fills with real bumblebee motion (lots of half steps around E) instead of slider jitter. Invent then has something consistent to sample.

The buzzer is also the right output for this model: the model only ever emits **one pitch**, and `tone()` can play that pitch.

---

## Summary

### What the final project is

Arduino Uno + passive piezo buzzer on Digital 9. Started as song playback. Ended as a tiny next-note learner:

1. Play a Flight of the Bumblebee reduction.
2. Count which pitch follows which.
3. Sample a new monophonic line from those counts.

Serial at 9600 dumps the table and invented notes.

### How the brain works

Not a saved song. A **21×21 count matrix** in RAM.

- `pool[]` = 21 allowed Hz (B4…G6)
- `freqToIdx(Hz)` → index 0…20, or −1 if rest / unknown
- `countTable[i][j]` = times pitch *j* followed pitch *i*

A lone note stores nothing. **B then C** does `countTable[B][C]++`.
Invent after B only reads **row B**. Reset / unplug clears RAM.

Training runs **once** (`trained`). Later loops keep the table fixed and roll new dice.

### What the sketch does in order

1. Play the written bumblebee from `PROGMEM`.
2. For each adjacent sounding pair, increment the table.
3. Print nonzero cells.
4. Invent: random start, length 48–96, next notes from a mixture, duration \(N \pm 10\) ms.
5. Repeat invent only.

How it chooses the next note
On the current note i, roll 0–99:

18% — any of the 21 pitches
22% — one step up or down in pool (a half step)
60% — weighted sample of C[i][j] + 1

+1 makes unseen next notes legal (rare, not impossible).
pickStart() picks a starting note from notes that actually appeared as “from” notes in the original. If the table is empty, it uses E6.
That is why invented lines change but still buzz like the source.
Math (compressed)
Alphabet P: 21 pitches.

C[i][j]: how many times j followed i.

N[i]: sum of row i (how often i had a next note).
Smoothed next-note rule:
P_smooth(j given i) = (C[i][j] + 1) / (N[i] + 21)
Mixture:
P_mix = 0.18 * (1/21) + 0.22 * (half-step neighbor) + 0.60 * P_smooth
Table branch: draw r from 0 to W-1, where W = sum over j of (C[i][j] + 1). Walk across the row, adding weights, until the running total passes r. That column is the next note.
Start:
P_start(i) = N[i] / (sum of all N[k])
Not modeled: original rhythm, two-note context, exact replay of the written sequence.
Code map


Symbol                   Role
melody[][2]PROGMEM       Source {Hz, ms} in flash

countTable               The brain

learnPair                C[i][j] += 1

inventNext               One sample from P_mix

pickStart                First invented note

playOriginalAndLearn     Play source and fill C

playInvented             Random walk

printBrain               Serial dump of nonzero pairs

freqTOIdx                Maps freq(Hz) to pool index

pool(0-20)               List of allowed notes(0-20)

**CODE**
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

## learnPair records one transition: it takes the previous note and the next note as frequencies, maps each to a pool index with freqToIdx, ignores the pair if either index is invalid (rest or a pitch not in pool), and otherwise adds one to countTable[a][b], which means “after note a, note b occurred once more.” That single increment is how the program learns; invent later reads that cell as part of the weighted chance that b follows a.
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


##Limits
Limits
The buzzer can play only one note at a time. The model remembers only pairs: given this note, what usually came next. It does not remember longer phrases, original timing, or the full written melody. Invented output will resemble the bumblebee’s motion. It will not sound like an orchestra and it will not reproduce the original sequence.
