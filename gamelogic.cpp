#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"

volatile unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 200;           


struct Game {
  volatile uint8_t randomNumbers[100];
  volatile uint8_t userNumbers[100];
  volatile uint8_t currentIndex;
  volatile uint8_t userIndex;
  volatile uint8_t correctPressCount;
  volatile bool timeToCheckGameStatus;
  volatile unsigned long lastButtonPressTime;
  volatile uint8_t buttonPressThreshold;
  volatile bool gameStatus = false;
};


volatile struct Game game;

void initializeTimer() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;                        // 1 Hz if prescaler 1024
  TCCR1B |= (1 << WGM12);               // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // prescaler=1024
  TIMSK1 |= (1 << OCIE1A);              // timer1 interrupt enabled
}

void startTheGame() {

  game.gameStatus = true;
  game.currentIndex = 0;
  game.userIndex = 0;
  game.correctPressCount = 0;
  game.timeToCheckGameStatus = false;
  game.lastButtonPressTime = millis();
  game.buttonPressThreshold = 200;
  initializeTimer();

}

void stopTheGame() {
  TIMSK1 = 0;

}

void checkGame() {
  if (game.timeToCheckGameStatus) {
    game.correctPressCount = 0;
    for (uint8_t i = 0; i < game.currentIndex; ++i) {
      Serial.println(game.randomNumbers[i] == game.userNumbers[i]);
      if (game.randomNumbers[i] == game.userNumbers[i]) {
        game.correctPressCount++;
      } else {
        stopTheGame();
        break;
      }
      game.timeToCheckGameStatus = false;
    }
   
    showResults(game.correctPressCount);

   
  }

  if (TCNT1 > (OCR1A / 2)) {
    clearAllLeds();
  }
}



ISR(PCINT2_vect) {
 

  unsigned long currentTime = millis();
  if (millis() > (lastDebounceTime + debounceDelay)) {
    int buttonValue = buttonPush();
    if (buttonValue == 4) {
    
      startTheGame();
    } else {
      game.userNumbers[game.userIndex] = buttonValue;

     


      game.userIndex++;

    


      game.timeToCheckGameStatus = true;
    }
  }
  lastDebounceTime = currentTime;
}

ISR(TIMER1_COMPA_vect) {

  if (game.currentIndex < 100) {

    uint8_t randomNumber = random(4);
    game.randomNumbers[game.currentIndex] = randomNumber;

    
    setLed(randomNumber);
   

    game.currentIndex++;

   
   if (game.correctPressCount % 10 == 0 && game.correctPressCount > 10) {
      OCR1A = OCR1A - (OCR1A / 10);
    }
  } else {
    stopTheGame();
  }
}



void setup() {
  initButtonsAndButtonInterrupts();
  initializeLeds();
  initializeDisplay();
  Serial.begin(9600); 
}

void loop() {
  checkGame();
  delay(100);
}
