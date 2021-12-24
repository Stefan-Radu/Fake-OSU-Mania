# Kinda OSU!
### It's like OSU! Mania _kinda_

## Introduction

I created this project as part of the "Introduction to Robotics" course I took
in the 3rd year @FMI UNIBUC. The goal was to create a game featuring an 8x8 LED
Matrix.

## Backstory

I really like rhythm games and it seemed like a challenge to implement one on
such restrictive hardware. Thus I chose to make something resembling OSU! Mania
(or piano tiles if you may).

## Challenges encounteres

I tried using an mp3player and actual OSU! files (stored on SDCard) translated
to fit the restrictions. After a lot of brainstorming, failing to get the
hardware to work properly, I almost dropped the idea. The @teach suggested I
used a second arduino. In the end I implemented a midway solution, using
_Arduino Songs_ stored on the SDCard.

## 🕹️ How to Play 🕹️

* There are 4 columns and 4 buttons. Each button corresponds to one of the collumns.
* Press (and hold) the button when the bar reaches the bottom. Release when it ends
* Your goal is to hit as many & much of the falling bars in order to play the song
* If you miss too many you lose
* If you play well you'll get your lives back


## 🔧 Used components 🔩

⚙  Arduino Uno - 2  
⚙  8x8 LED Matrix - 1  
⚙  Matrix Driver - 1  
⚙  16x2 LCD Display - 1  
⚙  74HC595 Shift Register - 1  
⚙  10k Ohm Potentiometer - 1  
⚙  Joystick - 1  
⚙  Button - 4  
⚙  Speaker - 1  
⚙  MicroSD Card Adapter - 1  
⚙  Resistors - 2  
⚙  Capacitors - 3  
⚙  BreadBoards - a few  
⚙  Wires - lots of them  
⚙  Shoe Box - 1  

### Picture & Video

![Setup]("./assets/setup.png")
