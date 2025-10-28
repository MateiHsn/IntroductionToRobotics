# Introduction To Robotics (2025-2026)

This repo is dedicated to the Introduction to Robotics lab homeworks, taken in the 3rd year at the Faculty of Mathematics and Computer Science, University of Bucharest. Each main part is dedicated to each lab homework and will include requirements, implementation details, code and various image files.

<details>
<summary>

  ## Homework 1
  
</summary>
  
  ### Task requirements
  
  Control each color of the RGB LED using 3 potentiometers that are hooked up to an Arduino, which will map the read values to the LED pins.

  ### Components needed:
  
  1. Arduino Uno
  2. 3 potentiometers
  3. 1 RGB LED
  4. A couple resistors (for current limiting)
  5. Wires (for connecting everything)

  ### Photos of the setup (including design schematic)
  
  Initially, I created the following schematic in KiCad so I could easily assemble the final circuit:
  
  ![KiCad schematic](/Homework1/KiCadSchematic.png)

  I chose to use 5 100 ohm resistors since the calculations for each LED gave me a value of 120 ohms for the green and blue LED respectively and a value of 160 ohms for the red one. Since I didn't have the appropriate value for all of them, I decided to group resistors in series and parallel to get a desirable and relatively close result. 
  
  The final form of the circuit is visible in the following images:
  
  ![Top-down view](/Homework1/top_down_view.jpg)
  
  ![Side view](/Homework1/side_view.jpg)

</details>
  <details>
<summary>

  ## Homework 2
  
</summary>
  
  ### Task requirements
  


  ### Components needed:
  
  1. Arduino Uno
  2. 5x LEDs (2 for pedestrians, 3 for cars)
  3. 1 buzzer
  4. 7 - segment display
  5. Wires & resistors as needed

  ### Photos of the setup (including design schematic)
  
  Initially, I created the following schematic in KiCad so I could easily assemble the final circuit:
  
  ![KiCad schematic](/Homework2/KiCad_Schematic.png)
  
  The final form of the circuit is visible in the following images:
  
  ![Top-down view](/Homework2/top_down_view.jpeg)
  
  ![Side view](/Homework2/side_view.jpeg)

  ### Youtube video showcasing functionality
  
  [Video Link]()

</details>
