# Introduction To Robotics (2025-2026)

This repo is dedicated to the Introduction to Robotics lab homeworks, taken in the 3rd year at the Faculty of Mathematics and Computer Science, University of Bucharest. Each main part is dedicated to each lab homework and will include requirements, implementation details, code and various image files.

<details>
<summary>

  ## Project 1
  
</summary>
  
  ### Task requirements
  
  Control each color of the RGB LED using 3 potentiometers that are hooked up to an Arduino, which will map the read values to the LED pins.

  ### Components needed:
  
  1. Arduino Uno
  2. 3 potentiometers
  3. 1 RGB LED
  4. A couple resistors (for current limiting)
  5. Wires (for connecting everything)

  ### Schematics and photos of the setup
  
  Initially, I created the following schematic in KiCad so I could easily assemble the final circuit:
  
  ![KiCad schematic](/Project_1/KiCadSchematic.png)

  I chose to use 5 100 ohm resistors since the calculations for each LED gave me a value of 120 ohms for the green and blue LED respectively and a value of 160 ohms for the red one. Since I didn't have the appropriate value for all of them, I decided to group resistors in series and parallel to get a desirable and relatively close result. 
  
  The final form of the circuit is visible in the following images:
  
  ![Top-down view](/Project_1/top_down_view.jpg)
  
  ![Side view](/Project_1/side_view.jpg)

</details>
  <details>
<summary>

  ## Project 2
  
</summary>
  
  ### Task requirements
  
  Create a traffic light system that respects 4 different states:
    1. Idle: the system waits for a button press to start the cycle (pedestrians are not allowed to cross)
    2. Transition: starts after an 8 second delay after the button press in state 1 (cars' yellow light turns on, signaling them to stop)
    3. Pedestrians Go (pedestrians get the green light and a buzzer signal, cars have to stop)
    4. Warning (the green light and the buzzer start blinking; pedestrians have to speed up)   

  The code respects these 4 states using a switch instruction as well as a variable that stores the current state (called trafficState) so there won't be any issues where the system could accidentally enter an unwanted state. 
  Aditionally, 2 functions were used besides the main loop & setup functions for displaying numbers on the 7-segment display (called displayNumbers) and for handleing the interrupt from the button to start the state cycle (conveniently called handleInterrupt).
  ### Components needed:
  
  1. Arduino Uno
  2. 5x LEDs (2 for pedestrians, 3 for cars)
  3. 1 buzzer
  4. 7 - segment display
  5. Wires & resistors as needed

  ### Schematics and photos of the setup
  
  Initially, I created the following schematic in KiCad so I could easily assemble the final circuit:
  
  ![KiCad schematic](/Project_2/KiCad_Schematic.png)

  Using the KiCad schematic, I implemented the circuit in Tinkercad first, resulting in the following system:

  ![Tinkercad system](/Project_2/Tinkercad_image.jpg)
  
  The final form of the circuit is visible in the following images:
  
  ![Top-down view](/Project_2/top_down_view.jpeg)
  
  ![Side view](/Project_2/side_view.jpeg)

  ### Youtube video showcasing functionality
  
  [Video Link](https://youtu.be/Zlj6oTO0wZQ)

</details>

<details>
  <summary>
   
  ## Project 3
  
  </summary>

  ### Task requirements

  Design and implement a simple home alarm system controlled via the Serial Monitor. The system cand be armed or disarmed and can respond to multiple commands including testing the alarm and changing each parameter of the system. Normally, when an intrusion is detected, an alarm sequence should play out.

  ### Components needed:
  1. Ultrasonic sensor (HC-SR04 or equivalent)
  2. Photoresistor (LDR)
  3. Buzzer
  4. Red LED
  5. Green LED
  6. Resistors and jumpers as needed

  ### Schematics and photos of the setup

  ![KiCad schematic](Project_3/KiCad_Schematic.png)
  ![Tinkercad Model](Project_3/Tinkercad_Model.png)
  ![Top-Down View](Project_3/Top_down_view.jpeg)
  ![Side View](Project_3/Side_view.jpeg)

  ### Youtube video showcasing functionality
  [Video Link](https://youtu.be/81CxpTphdrc)
  
</details>

<details>
<summary>

  ## Project 4
  
</summary>
  
  ### Task requirements
  
  The subject of project is to design and implement a game that resembles the template of "Simon Says", only in this case, the player is presented with a character sequence on a 4-digit 7-segment display and have to reproduce it by selecting each character using a joystick. The player also has a button that pauses the current state of the game and displays the main menu.
  Regarding the design of the circuit, the most important part was displaying text on the 7-segment display, which was done using the 74HC595 8-bit SIPO shift register. To effectively control the data outputs of the register, I used SPI for its ease of use and for its transfer speeds.
  Regarding the actual software implementation of the game, I used a finite-state machine to separate every part of the game from each other. The main state of the game are:
  
  1. Main Menu - contains the 3 initial options for the game - PLAY, SCORE and STOP
  2. Score Menu - displays the highest score achieved since the game was turned on
  3. Stop - returns the game to the main menu
     
  After starting the game phase, there are also some secondary states the game was split into:
  
  1. Sequence display - the target sequence is displayed on the 7-segment display for the player to remember
  2. Input phase - the player is presented with a default input in order to set the input sequence
  3. Result checking - after a long press of the joystick, the input sequence is checked
  4. Show result - after checking the input, the player is presented the current score if the input is correct or and error message otherwis. The former would take the player to the next round while the latter returns them to the main menu.
  5. Pause - pressing the pause button causes the game to go in this state which display the menu with the aforementioned options.

  Another design choice that helped the implementation of this circuit was the use of the SPI communication protocol over bit banging. In this case, i had to wire up the Arduino and the shift register using the SPI-specific pins:
  1. SCK (pin D13) - for shifting each bit in the shift register to the left
  2. COPI (pin D11) - for sending the actual encoding to the shift register
  3. CS (pin D10) - this could have been any other pin besides D12 (which is designated as CIPO), since it only needs to be set HIGH or LOW in order to select when to output the register's data on its buffered output.
  
  ### Components needed:
  
  1. 74HC595 shift register IC
  2. Joystick module
  3. SH5461AS common-cathode 4-digit 7-segment display
  4. Pushbutton
  5. Buzzer
  6. 2 x 100nF Capacitors for physical debouncing
  7. Jumper wires and resistors as needed

  ### Schematics and photos of the setup
  
  Initially, I created the following schematic in KiCad so I could easily assemble the final circuit:
  
  ![KiCad schematic](Project_4/tema4_schematic.png)

  ![Wokwi schematic](Project_4/Wokwi_schematic.png)
  
  The final form of the circuit is visible in the following images:
  
  ![Top-down view](Project_4/Top_down_view.jpeg)
  
  ![Side View](Project_4/Side_view.jpeg)

### Youtube video showcasing functionality
  
  [Video Link](https://youtu.be/J4s3nbRbCSY)

</details>

<details>

  <summary>
  
  ## Project 5
  
  </summary>

  ### Task requirements

  Design and implement a 2D platformer game constructed using SoC (Separation of Concerns).
  Initially, I implemented the GameModel class that takes care of the game itself and how it updates based on the player's inputs, which is then controlled by the GameController. THe GameController connects all further classes with eachother, including HardwareManager, which reads inputs and updates outputs, and the classes that inherit IRenderer, which acts as an interface for whatever display the player would like to use (in this case Serial or LCD).

  ### Components needed:

  1. 1x 16x2 LCD (HD44780 or compatible)
  2. 1x joystick
  3. 1x push button for Pause
  4. 3x LEDs for displaying the states of the game
  5. EEPROM (built-in) for saving the top 3 highest scores
  6. Buzzer
  
  ### Schematics and photos of the setup

  The circuit itself started out as this schematic:
  
  ![KiCad schematic](Project_5/Tema5_schematic.png)
  
  I also implemented the circuit in the Wokwi circuit simulator to dispolay a phyisical diagram:
  
  ![Wokwi Model](Project_5/Wokwi_model.png)
  
  The previous diagrams where then used to implement the final circuit:
  
  ![Top-Down View](Project_5/Top_down_view.jpeg)
  ![Side View](Project_5/SIde_view.jpeg)

  ### Youtube video showcasing functionality
  [Video Link](https://youtu.be/pufFJ4CxjFs)
  
</details>
