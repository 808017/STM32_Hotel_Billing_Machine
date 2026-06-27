# STM32 Hotel Billing Machine

An embedded hotel billing machine developed using the STM32F446RE microcontroller. The system provides a menu-driven interface for selecting food items through a 4×4 keypad, displays information on a 16×2 LCD, maintains real-time operation using the RTC peripheral, and prints the final bill through UART communication.

## Features

* Menu-driven billing system.
* 4×4 matrix keypad interface
* 16×2 LCD display (8-bit mode)
* Real-Time Clock (RTC)
* Automatic menu scrolling
* Multiple item selection
* Running total calculation
* Final bill generation
* UART-based bill printing
* State machine implementation
* Item count tracking
* Bill number generation

## Hardware Used

* STM32F446RE Nucleo Board
* 16×2 Character LCD
* 4×4 Matrix Keypad
* UART Terminal / Serial Monitor
* Jumper Wires
* Breadboard

## Software Used

* STM32CubeIDE
* STM32 HAL Library

## Peripherals Used

| Peripheral | Purpose                  |
| ---------- | ------------------------ |
| GPIO       | LCD and Keypad Interface |
| RTC        | Real-Time Clock          |
| USART2     | Bill Printing            |
| ADC1       | Future Expansion         |
| SysTick    | Menu Timing              |

## Menu Items

| Code | Item         | Price |
| ---- | ------------ | ----: |
| 1    | Sambhar Vada |   ₹30 |
| 2    | Idli Sambhar |   ₹30 |
| 3    | Idli Vada    |   ₹30 |
| 4    | Plain Dosa   |   ₹40 |
| 5    | Masala Dosa  |   ₹50 |
| 6    | Rava Dosa    |   ₹70 |
| 7    | Uthappam     |   ₹70 |
| 8    | Mix Uthappam |   ₹80 |
| 9    | Upma         |   ₹50 |
| 10   | Fried Rice   |   ₹60 |
| 11   | Jeera Rice   |   ₹60 |
| 12   | Tomato Rice  |   ₹60 |
| 13   | Coffee       |   ₹10 |
| 14   | Tea          |   ₹12 |
| 15   | Badam Milk   |   ₹15 |

## Working Principle

1. Startup screen displays system information.
2. Menu items automatically scroll on the LCD.
3. Home screen displays the current time using RTC.
4. User enters item codes using the keypad.
5. Selected item prices are added to the running total.
6. Multiple items can be added.
7. Pressing the bill button generates the final bill.
8. The bill is printed through USART2.

## State Machine

The application is implemented using a finite state machine:

```text
STARTUP
   ↓
MENU
   ↓
HOME
   ↓
ADD_ITEM
   ↓
FINAL_BILL
```

## Example Bill Output

```text
India Skills - 2026 National

****ISC Billing Machine****

Bill Number: 0001

--------------------------------
Sr.  Item Name        Price

1   Masala Dosa        50
2   Coffee             10
3   Tea                12

--------------------------------

Grand Total            72
```

## Keypad Functions

| Key | Function            |
| --- | ------------------- |
| 0-9 | Item code input     |
| #   | Enter               |
| *   | Back                |
| D   | Generate Final Bill |

## Project Structure

```text
STM32_Hotel_Billing_Machine
│
├── Core
│   ├── Inc
│   └── Src
├── Drivers
├── STM32_Hotel_Billing_Machine.ioc
├── README.md
├── LICENSE
└── .gitignore
```

## Applications

* Restaurant Billing Systems
* Cafeteria Automation
* Hotel Menu Management
* Point of Sale (POS) Systems
* Embedded Human Machine Interfaces
* Industrial Training Projects

## Target MCU

* STM32F446RE

## Communication Interfaces

* UART (USART2)
* GPIO

## IDE

* STM32CubeIDE

## Author

**Yash Mane**

## License

This project is licensed under the MIT License.
