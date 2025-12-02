int const buttonPin = 7;
int const pot1pin = A0;
int const pot2pin = A2;
int const pot3pin = A1;

int pot1val = 0;
int pot2val = 0;
int pot3val = 0;

int currentMode = 0;

int const rPin = 2;
int const gPin = 3;
int const bPin = 4;

int const micPin = A5; //MAX4466

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);

  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);

  Serial.begin(9600);
}

void customColor(int r, int g, int b){
  //Serial.println(r);
  //Serial.println(g);
  //Serial.println(b);
  analogWrite(rPin, (255-r));
  analogWrite(gPin, (255-g));
  analogWrite(bPin, (255-b));
  delay(50);
}

void fading(){
  const static byte colors[][3] = {
    {255, 0, 0},     // Rosso
    {255, 128, 0},   // Arancione
    {255, 255, 0},   // Giallo
    {0, 255, 0},     // Verde
    {0, 0, 255},     // Blu
    {128, 0, 255},   // Viola
    {255, 0, 255}    // Magenta
  };
  const static int NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

  // Variabili statiche per mantenere lo stato tra le chiamate
  static unsigned long lastUpdateTime = 0;
  static int step = 0;
  static byte startColor[3] = {colors[0][0], colors[0][1], colors[0][2]};
  static byte currentColor[3] = {colors[0][0], colors[0][1], colors[0][2]};
  static int currentTargetIndex = 0;
  static int nextTargetIndex = 1;

  // Lettura del potenziometro e mappatura della velocità (con limiti)
  int potValue = analogRead(pot2pin);
  int stepDelay = map(potValue, 0, 1023, 100, 5);  // Da lento (100ms) a veloce (5ms)
  stepDelay = constrain(stepDelay, 5, 100);         // Limiti di sicurezza
  
  // Controllo del tempo per l'aggiornamento
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateTime >= stepDelay) {
    lastUpdateTime = currentMillis;
    // Reset del colore iniziale all'inizio di ogni transizione
     if (step == 0) {
         startColor[0] = currentColor[0];
         startColor[1] = currentColor[1];
         startColor[2] = currentColor[2];
     }
    step++;  // Passo successivo
    // Interpolazione lineare dei colori
    for (int i = 0; i < 3; i++) {
        int diff = (int)colors[nextTargetIndex][i] - startColor[i];
        currentColor[i] = startColor[i] + (diff * step) / 100;
    }
    // Scrittura del colore (common anode: valori invertiti)
    analogWrite(rPin, 255 - currentColor[0]);
    analogWrite(gPin, 255 - currentColor[1]);
    analogWrite(bPin, 255 - currentColor[2]);
    // Passaggio al colore successivo alla fine della transizione
    if (step >= 100) {
        step = 0;
        currentTargetIndex = nextTargetIndex;
        nextTargetIndex = (nextTargetIndex + 1) % NUM_COLORS;
    }
  }
}

void hsvToRgb(float h, float s, float v, int &r, int &g, int &b) {
  int i = int(h / 60.0) % 6;
  float f = (h / 60.0) - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  
  switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break; // case 5
  }
  r = constrain(r * 255, 0, 255);
  g = constrain(g * 255, 0, 255);
  b = constrain(b * 255, 0, 255);
}

/*void musicReactive(){
  static unsigned long lastSampleTime = 0;
  const unsigned long sampleInterval = 10; // 10ms
  
  static unsigned long lastColorUpdate = 0;
  static float hue = 0.0;
  
  static int runningAverage = 512;
  static int currentPeak = 0;
  static int beatCount = 0;
  static unsigned long lastBeatTime = 0;
  
  unsigned long currentMillis = millis();
  
  // Campionamento microfono
  if (currentMillis - lastSampleTime >= sampleInterval) {
    lastSampleTime = currentMillis;
    
    int sample = analogRead(micPin);
    runningAverage = (runningAverage * 7 + sample) / 8;  // Filtro passa-basso
    int amplitude = abs(sample - runningAverage);
    
    // Rilevamento beat
    if (amplitude > currentPeak) {
        currentPeak = amplitude;
    } else {
        currentPeak = (currentPeak * 97) / 100;  // Decadimento del 3%
    }
    
    // Soglia dinamica per il beat
    int threshold = runningAverage + (currentPeak * 2);
    if (amplitude > threshold && currentMillis - lastBeatTime > 100) {
        beatCount++;
        lastBeatTime = currentMillis;
    }
    
    // Lettura potenziometri
    int colorSpeed = map(analogRead(pot1pin), 0, 1023, 1, 20);
    int sensitivity = map(analogRead(pot2pin), 0, 1023, 5, 50);
    int saturation = map(analogRead(pot3pin), 0, 1023, 50, 100) / 100.0;
    
    // Calcolo luminosità in base al suono
    int brightness = map(min(currentPeak * sensitivity, 1023), 0, 1023, 0, 255);
    brightness = constrain(brightness, 0, 255);
    
    // Rotazione colore basata sul tempo e sui beat
    if (currentMillis - lastColorUpdate >= 20) {
        lastColorUpdate = currentMillis;
        
        // Due modalità di cambio colore:
        if (beatCount > 0) {
            // Cambio colore istantaneo al beat
            hue = int(hue + 30 + (beatCount * 60)) % 360;
            beatCount = 0;
        } else {
            // Rotazione graduale
            hue = fmod(hue + colorSpeed * 0.02, 360);
        }
    }
    
    // Conversione HSV to RGB
    int r, g, b;
    hsvToRgb(
        hue,                             // Hue (0-360)
        saturation,                       // Saturazione (0.0-1.0)
        brightness / 255.0,               // Luminosità (0.0-1.0)
        r, g, b
    );
    
    // Applica ai LED (common anode)
    analogWrite(rPin, 255 - r);
    analogWrite(gPin, 255 - g);
    analogWrite(bPin, 255 - b);
  }
}*/

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(buttonPin) == LOW){
    Serial.println("Cambio modalità!");
    delay(1000);
    currentMode++;
  }
  if (currentMode > 2){
    currentMode = 0;
  }

  if (currentMode == 0){
    pot1val = analogRead(pot1pin);
    pot2val = analogRead(pot2pin);
    pot3val = analogRead(pot3pin);
    int r = map(pot1val, 0, 1023, 0, 255);
    int g = map(pot2val, 0, 1023, 0, 255);
    int b = map(pot3val, 0, 1023, 0, 255);
    customColor(r, g, b);
  }
  else if (currentMode == 1){
    fading();
  }
  //else if (currentMode == 2){
  //  musicReactive();
  //}
}
