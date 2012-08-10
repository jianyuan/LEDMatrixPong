#include "LedControl.h"

#define PLAYER_1_PIN A1
#define PLAYER_2_PIN A0
#define BUZZER_PIN 6

LedControl lc = LedControl(12, 11, 10, 1);

// Game settings
const int winningPoints = 5;

// Game variables
int playerOnePoints = 0, playerTwoPoints = 0;
int playerOnePosition, playerTwoPosition;
int ballX = 3, ballY = 3;
int ballDirectionX = 1, ballDirectionY = 1;
boolean isGameOver = false, isRoundOver = false;
boolean playerOneWonLastRound = true;
int speed, speedCount = 0;
long ballDelayCount = 0;

char digits[][5] = {
  {0b111,0b101,0b101,0b101,0b111},
  {0b010,0b010,0b010,0b010,0b010},
  {0b111,0b001,0b111,0b100,0b111},
  {0b111,0b001,0b111,0b001,0b111},
  {0b101,0b101,0b111,0b001,0b001},
  {0b111,0b100,0b111,0b001,0b111},
  {0b111,0b100,0b111,0b101,0b111},
  {0b111,0b001,0b001,0b001,0b001},
  {0b111,0b101,0b111,0b101,0b111},
  {0b111,0b101,0b111,0b001,0b111},
};

void setup()
{
  pinMode(BUZZER_PIN, OUTPUT);
  
  randomSeed(analogRead(A5));
  
  // wakeup LED controller
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);

  resetGame();
  resetRound();
}

void loop()
{
  if (++ballDelayCount >= (speed * 5))
  {
    ballDelayCount = 0;
    
    checkCollision();
    refreshGameState();
    
    if (isRoundOver)
    {
      renderRoundOver();
      resetRound();
    }
    else
    {
      moveAndRenderBall();
    }
    
    if (isGameOver)
    {
      renderGameOver();
      resetGame();
    }
  }
  
  refreshControls();
  renderPaddles();
}

void checkCollision()
{
  boolean changedDirection = false;
  if (ballX == 1)
  {
    if (ballY == playerOnePosition || ballY == (playerOnePosition + 1) || ((ballY == 0 || ballY == 7) && ((ballY - ballDirectionY) == playerOnePosition || (ballY - ballDirectionY) == (playerOnePosition + 1))))
    {
      ballDirectionX = -ballDirectionX;
      changedDirection = true;
    }
    else if ((ballY + ballDirectionY) == playerOnePosition || (ballY + ballDirectionY) == (playerOnePosition + 1))
    {
      ballDirectionX = -ballDirectionX;
      ballDirectionY = -ballDirectionY;
      changedDirection = true;
    }
  }
  else if (ballX == 6)
  {
    if (ballY == playerTwoPosition || ballY == (playerTwoPosition + 1) || ((ballY == 0 || ballY == 7) && ((ballY - ballDirectionY) == playerTwoPosition || (ballY - ballDirectionY) == (playerTwoPosition + 1))))
    {
      ballDirectionX = -ballDirectionX;
      changedDirection = true;
    }
    else if ((ballY + ballDirectionY) == playerTwoPosition || (ballY + ballDirectionY) == (playerTwoPosition + 1))
    {
      ballDirectionX = -ballDirectionX;
      ballDirectionY = -ballDirectionY;
      changedDirection = true;
    }
  }
  
  if (changedDirection)
  {
    tone(BUZZER_PIN, 523.25, 20);
  }
  
  if (ballY <= 0 || ballY >= 7)
  {
    ballDirectionY = -ballDirectionY;
    changedDirection = true;
    
    tone(BUZZER_PIN, 261.63, 20);
  }
  
  if (changedDirection)
  {
    recalculateSpeed();
  }
}

void moveAndRenderBall()
{
  lc.setLed(0, ballY, ballX, false);

  ballX += ballDirectionX;
  ballY += ballDirectionY;

  if (ballX > 0 && ballX < 7)
  {
    lc.setLed(0, ballY, ballX, true);
  }
}

void refreshControls()
{
  playerOnePosition = constrain(map(analogRead(PLAYER_1_PIN), 0, 1024, 0, 7), 0, 6);
  playerTwoPosition = constrain(map(analogRead(PLAYER_2_PIN), 0, 1024, 0, 7), 0, 6);
}

void resetRound()
{
  isRoundOver = false;
  
  if (playerOneWonLastRound)
  {
    ballX = 2;
    ballDirectionX = 1;
  }
  else
  {
    ballX = 5;
    ballDirectionX = -1;
  }
  
  ballY = random(0, 7);
  ballDirectionY = (random(0, 1) == 0) ? -1 : 1;
  speed = 10;
  speedCount = 0;
  ballDelayCount = 0;
  
  /*
  ballDirectionX = -1;
  ballDirectionY = 1;
  ballX = 4;
  ballY = 4;
  */
  
  /*
  ballDirectionX = -1;
  ballDirectionY = -1;
  ballX = 4;
  ballY = 3;
  */
  
  /*
  ballDirectionX = -1;
  ballDirectionY = -1;
  ballX = 4;
  ballY = 0;
  */
}

void renderPaddles()
{
  renderPaddle(playerOnePosition, 0);
  renderPaddle(playerTwoPosition, 7);
}

void renderPaddle(int pos, int col)
{
  if (pos >= 0 && pos <= 6)
  {
    lc.setColumn(0, col, 0b00000011 << (6 - pos));
  }
}

void recalculateSpeed()
{
  if (++speedCount % 5 == 0 && speed > 1)
  {
    speed -= 1;
  }
}

void refreshGameState()
{
  // Ball hits left edge
  if (ballX == 0)
  {
    playerTwoPoints++;
    playerOneWonLastRound = false;
    isRoundOver = true;
  }
  // Ball hits right edge
  else if (ballX == 7)
  {
    playerOnePoints++;
    playerOneWonLastRound = true;
    isRoundOver = true;
  }
  
  if (playerOnePoints >= winningPoints || playerTwoPoints >= winningPoints)
  {
    isGameOver = true;
  }
}

void renderRoundOver()
{
  tone(BUZZER_PIN, 130.81, 640);
  
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0, i, 0b11111111);
    delay(80);
  }
  
  lc.clearDisplay(0);
  
  // Display score
  for (int i = 0; i < 5; i++)
  {
    lc.setRow(0, (1 + i), digits[playerOnePoints][i] << 5 | digits[playerTwoPoints][i]);
  }
  
  delay(1000);
  lc.clearDisplay(0);
}

void renderGameOver()
{
  playNotes();
  
  if (playerOnePoints >= winningPoints)
  {
    // p1 wins
  }
  else
  {
    // p2 wins
  }
}

void resetGame()
{
  isGameOver = false;
  playerOnePoints = playerTwoPoints = 0;
  playerOneWonLastRound = true;
}


// http://www.arduino.cc/en/Tutorial/Melody
int length = 11; // the number of notes
char notes[] = "CCCC45C 5C "; // a space represents a rest
int beats[] = { 1, 1, 1, 3, 3, 2, 1, 1, 1, 6 };
int tempo = 110; // reverse of what you would think

void playNotes()
{
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }
    
    // pause between notes
    delay(tempo / 2); 
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', '1', 'd', '2', 'e', 'f', '3', 'g', '4', 'a', '5', 'b', 'C' };
  int tones[] = { 1915, 1805, 1700, 1608, 1519, 1432, 1351, 1275, 1205, 1136, 1073, 1014, 956 };
  
  // play the tone corresponding to the note name
  for (int i = 0; i < 13; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(tone);
  }
}
