#include <TFT_eSPI.h> /* Graphics and font library for ST7735 driver chip */

/* PROGMEM arrays for graphics */
#include "include/graphics/background.h"
#include "include/graphics/gameplay.h"
#include "include/graphics/icon.h"
#include "include/graphics/lose.h"
#include "include/graphics/sprite.h"
#include "include/graphics/win.h"

/* Macros, &c. (for analog read processing) */
#include "include/move.h"

#include "include/misc.h"

TFT_eSPI tft = TFT_eSPI();

/* Sprites for game graphics */
TFT_eSprite background     = TFT_eSprite(&tft);
TFT_eSprite score          = TFT_eSprite(&tft);
TFT_eSprite game_sprite    = TFT_eSprite(&tft);
TFT_eSprite redbull_sprite = TFT_eSprite(&tft);
TFT_eSprite paper_sprite   = TFT_eSprite(&tft);

/* To keep track of animation state. */
int frame = 0;

/* Game status: has the player won or lost? */
int status = LOAD;

unsigned long curr_score = 0;
unsigned long offset = 0;

/* Starting x position */
int x = 55;

int delta = 8;

int jump = 0;
int duck = 0;

int command = RESET;

int enemy[4] = {0 - PAPER_WIDTH, 0 - PAPER_WIDTH, 0 - PAPER_WIDTH, 0 - PAPER_WIDTH};

/*
 * TFT library doesn't give us a means of natively right-aligning text.
 */
int align_score(unsigned long __score)
{
  if (!(__score / 10))
    return MAX_ALIGN;

  return align_score(__score / 10) - 7;
}

/*
 * Process the joystick read.
 */
void move()
{
  /* Variables to store the joystick values read. */
  int x_val = 0;
  int y_val = 0;

  command = RESET;

  /* Decrement timers (until next jump/duck). */
  if (jump)
    jump--;

  if (jump && JUMP_TIMEOUT - jump > 6)
    jump--;

  if (duck)
    duck--;

  x_val = analogRead(xyPins[0]);
  y_val = analogRead(xyPins[1]);

  if (x_val < LEFT_THRESHOLD)
    command = command | COMMAND_LEFT;
  else if (x_val > RIGHT_THRESHOLD)
    command = command | COMMAND_RIGHT;

  if (y_val < UP_THRESHOLD)
    command = command | COMMAND_UP;
  else if (y_val > DOWN_THRESHOLD)
    command = command | COMMAND_DOWN;

  /* We only want the sprite to move within the dimensions of the screen. */
  if ((command & COMMAND_LEFT) && (x >= 20))
    x -= 2;
  else if ((command & COMMAND_RIGHT) && (x <= (TFT_HEIGHT - 70)))
    x += 2;

  if ((command & COMMAND_UP) && !jump)
    jump = JUMP_TIMEOUT;
  else if ((command & COMMAND_DOWN) && !duck)
    duck = DUCK_TIMEOUT;
}

void render_game()
{
  int i;
  int max_pos = 0 - PAPER_WIDTH;

  background.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, bg);

  /* Append current score text. */
  score.fillSprite(TFT_BLACK);
  score.drawString(String(curr_score - offset), 0, 0, 2);
  score.pushToSprite(&background, align_score(curr_score - offset), 7, TFT_BLACK);

  /* Render character. */
  game_sprite.pushImage(0, 0, SPRITE_WIDTH, SPRITE_HEIGHT, ivy[frame]);
  game_sprite.pushToSprite(&background, x, 4, TFT_BLACK);

  for_each_enemy(i) {
    if (enemy[i] > max_pos)
      max_pos = enemy[i];
  }

  if (max_pos < TFT_HEIGHT - (PAPER_WIDTH + 40)) {
    for_each_enemy(i) {
      /* Can we place another "enemy"? */
      if (enemy[i] <= 0 - PAPER_WIDTH && !random(69)) {
        enemy[i] = TFT_HEIGHT - 1;
        break;
      }
    }
  }

  for_each_enemy(i) {
    if (enemy[i] > 0 - PAPER_WIDTH) {
      enemy[i] -= delta;
      paper_sprite.pushImage(0, 0, PAPER_WIDTH, PAPER_HEIGHT, paper);
      paper_sprite.pushToSprite(&background, enemy[i], 108, TFT_BLACK);
    }
  }

  if (status != WIN)
    background.pushSprite(0, 0);
}

void render_endscreen_win()
{
  background.pushImage(0, 0, BG_WIDTH, BG_HEIGHT,
                        win_bg[++frame % LEN(win_bg)]);

  background.pushSprite(0, 0);
}

void render_endscreen_lose()
{
  background.pushImage(0, 0, BG_WIDTH, BG_HEIGHT,
                        lose_bg[++frame % LEN(lose_bg)]);

  background.pushSprite(0, 0);
}

void check_reset()
{
  int i;

  if (digitalRead(BUTTON_PIN))
    return;
  
  /* Reset some state. */
  status = PLAY;
  frame = 0;
  offset = millis() / 240;
  x = 55;

  jump = 0;
  duck = 0;

  for_each_enemy(i) {
    enemy[i] = 0 - PAPER_WIDTH;
  }
}

int check_status()
{
  int i;

  if (status = ((curr_score - offset) >= 1000)) {
    /* Player just won. */
    background.fillSprite(TFT_WHITE);
    background.pushSprite(0, 0);
    delay(300);

    return WIN;
  }

  if (frame > 2)
    return PLAY;

  for_each_enemy(i) {
    if ((x + 10) < enemy[i] + PAPER_WIDTH && (x + SPRITE_WIDTH - 7) > enemy[i]) {
      /* Player just lost. */
      background.fillSprite(TFT_BLACK);
      background.pushSprite(0, 0);
      delay(300);

      return LOSE;
    }
  }

  return PLAY;
}

void setup()
{
  tft.init();

  /* For correct colors. */
  tft.setSwapBytes(true);

  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);

  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  background.createSprite(BG_WIDTH, BG_HEIGHT);
  game_sprite.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);
  score.createSprite(80, 80);
  redbull_sprite.createSprite(REDBULL_WIDTH, REDBULL_HEIGHT);
  paper_sprite.createSprite(PAPER_WIDTH, PAPER_HEIGHT);

  score.setTextColor(TFT_WHITE, TFT_BLACK);
  background.setSwapBytes(true);

  background.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, howto);
  background.pushSprite(0, 0);
}

void loop()
{
  curr_score = millis() / 240;

  if (status == LOAD) {
    check_reset();
  } else if (status == WIN) {
    render_endscreen_win();
    check_reset();
  } else if (status == LOSE) {
    render_endscreen_lose();
    check_reset();
  } else {
    move();
    render_game();

    if (JUMP_TIMEOUT - jump <= 8)
      frame = jump / 2;
    else if (DUCK_TIMEOUT - duck <= 2)
      frame = 2;
    else
      frame = !frame;

    status = check_status();
  }

  delay(200);
}
