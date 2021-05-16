/* NODISKEMU - SD/MMC to IEEE-488 interface/controller
   Copyright (C) 2007-2018  Ingo Korb <ingo@akana.de>
   Copyright (C) 2015-2018  Nils Eilers <nils.eilers@gmx.de>

   NODISKEMU is a fork of sd2iec by Ingo Korb (et al.), http://sd2iec.de

   Inspired by MMC2IEC by Lars Pontoppidan et al.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   arch-config.h: The main architecture-specific config header

*/

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

/* No enum used here to make these definitions visible for assembler code */
#define HW_EXAMPLE       1
#define HW_SHADOWOLF1    2
#define HW_LARSP         3
#define HW_UIEC          4
#define HW_SHADOWOLF2    5
#define HW_UIECV3        7
#define HW_PETSD         8
#define HW_PETSDPLUS     9
#define HW_PETSDLITE     10

#include <avr/io.h>
#include <avr/interrupt.h>
/* Include avrcompat.h to get the PA0..PD7 macros on 1284P */
#include "avrcompat.h"

/* ----- Common definitions for all AVR hardware variants ------ */

/* Return value of buttons_read() */
typedef uint8_t rawbutton_t;

/* EEPROMFS: offset and size must be multiples of 4 */
/* to actually enable it, CONFIG_HAVE_EEPROMFS must be set in config */
#  define EEPROMFS_OFFSET     512
#  define EEPROMFS_SIZE       3584
#  define EEPROMFS_ENTRIES    8
#  define EEPROMFS_SECTORSIZE 64


#if CONFIG_HARDWARE_VARIANT==HW_EXAMPLE
/* ---------- Hardware configuration: Example ---------- */
/* This is a commented example for most of the available options    */
/* in case someone wants to build Yet Another[tm] hardware variant. */
/* Some of the values are chosen randomly, so this variant is not   */
/* expected to compile successfully.                                */

/*** SD card support ***/
/* If your device supports SD cards by default, define this symbol. */
#  define HAVE_SD

/* Declaration of the interrupt handler for SD card change */
#  define SD_CHANGE_HANDLER ISR(INT0_vect)

/* Declaration of the interrupt handler for SD card 2 change */
#  define SD2_CHANGE_HANDLER ISR(INT9_vect)

/* Initialize all pins and interrupts related to SD - except SPI */
static inline void sdcard_interface_init(void) {
  /* card detect (SD1) */
  DDRD  &= ~_BV(PD2);
  PORTD |=  _BV(PD2);
  /* write protect (SD1) */
  DDRD &= ~_BV(PD6);
  PORTD |= _BV(PD6);
  /* card change interrupt (SD1) */
  EICRA |= _BV(ISC00);
  EIMSK |= _BV(INT0);
  // Note: Wrapping SD2 in CONFIG_TWINSD may be a good idea
  /* chip select (SD2) */
  PORTD |= _BV(PD4);
  DDRD |= _BV(PD4);
  /* card detect (SD2) */
  DDRD &= ~_BV(PD3);
  PORTD |= _BV(PD3);
  /* write protect (SD2) */
  DDRD &= ~_BV(PD7);
  PORTD |= _BV(PD7);
  /* card change interrupt (SD2) */
  EICRA |=  _BV(ISC90); // Change interrupt
  EIMSK |=  _BV(INT9);  // Change interrupt
}

/* sdcard_detect() must return non-zero while a card is inserted */
/* This must be a pin capable of generating interrupts.          */
static inline uint8_t sdcard_detect(void) {
  return !(PIND & _BV(PD2));
}

/* Returns non-zero when the currently inserted card is write-protected */
static inline uint8_t sdcard_wp(void) {
  return PIND & _BV(PD6);
}

/* Support for a second SD card - use CONFIG_TWINSD=y in your config file to enable! */
/* Same as the two functions above, but for card 2 */
static inline uint8_t sdcard2_detect(void) {
  return !(PIND & _BV(PD3));
}
static inline uint8_t sdcard2_wp(void) {
  return PIND & _BV(PD7);
}

/* SD card 1 is assumed to use the standard SS pin   */
/* If that's not true, #define SDCARD_SS_SPECIAL and */
/* implement this function:                          */
//static inline __attribute__((always_inline)) void sdcard_set_ss(uint8_t state) {
//  if (state)
//    PORTZ |= _BV(PZ9);
//  else
//    PORTZ &= ~_BV(PZ9);
//}

/* SD card 2 CS pin */
static inline __attribute__((always_inline)) void sdcard2_set_ss(uint8_t state) {
  if (state)
    PORTD |= _BV(PD4);
  else
    PORTD &= ~_BV(PD4);
}

/* SD Card supply voltage - choose the one appropiate to your board */
/* #  define SD_SUPPLY_VOLTAGE (1L<<15)  / * 2.7V - 2.8V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<16)  / * 2.8V - 2.9V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<17)  / * 2.9V - 3.0V */
#  define SD_SUPPLY_VOLTAGE (1L<<18)  /* 3.0V - 3.1V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<19)  / * 3.1V - 3.2V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<20)  / * 3.2V - 3.3V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<21)  / * 3.3V - 3.4V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<22)  / * 3.4V - 3.5V */
/* #  define SD_SUPPLY_VOLTAGE (1L<<23)  / * 3.5V - 3.6V */

/* SPI clock divisors - the slow one must be 400KHz or slower,        */
/* the fast one can be as high as you thing your hardware will handle */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4


/*** Device address selection ***/
/* device_hw_address() returns the hardware-selected device address */
static inline uint8_t device_hw_address(void) {
  return 8 + !(PIND & _BV(PD7)) + 2*!(PIND & _BV(PD5));
}

/* Configure hardware device address pins */
static inline void device_hw_address_init(void) {
  DDRD  &= ~(_BV(PD7) | _BV(PD5));
  PORTD |=   _BV(PD7) | _BV(PD5);
}


/*** LEDs ***/
/* Please don't build single-LED hardware anymore... */

/* Initialize ports for all LEDs */
static inline void leds_init(void) {
  /* Note: Depending on the chip and register these lines can compile */
  /*       to one instruction each on AVR. For two bits this is one   */
  /*       instruction shorter than "DDRC |= _BV(PC0) | _BV(PC1);"    */
  DDRC |= _BV(PC0);
  DDRC |= _BV(PC1);
}

/* --- "BUSY" led, recommended color: green (usage similiar to 1541 LED) --- */
static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
  if (state)
    PORTC |= _BV(PC0);
  else
    PORTC &= ~_BV(PC0);
}

/* --- "DIRTY" led, recommended color: red (errors, unwritten data in memory) --- */
#  define LED_DIRTY_PORT        PORTC
#  define LED_DIRTY_INPUT       PINC
#  define LED_DIRTY_PIN         PC1


/*** IEC signals ***/
#  define IEC_INPUT PINA
#  define IEC_DDR   DDRA
#  define IEC_PORT  PORTA

/* Pins assigned for the IEC lines */
#  define IEC_PIN_ATN   PA0
#  define IEC_PIN_DATA  PA1
#  define IEC_PIN_CLOCK PA2
#  define IEC_PIN_SRQ   PA3

/* Use separate input/output lines?                                    */
/* The code assumes that the input is NOT inverted, but the output is. */
//#  define IEC_SEPARATE_OUT
//#  define IEC_OPIN_ATN   PA4
//#  define IEC_OPIN_DATA  PA5
//#  define IEC_OPIN_CLOCK PA6
//#  define IEC_OPIN_SRQ   PA7

/* You can use different ports for input and output bits. The code tries */
/* to not stomp on the unused bits. IEC output is on IEC_PORT.           */
/* Not well-tested yet.                                                  */
//#  define IEC_DDRIN      DDRX
//#  define IEC_DDROUT     DDRY
//#  define IEC_PORTIN     PORTX

/* ATN interrupt (required) */
#  define IEC_ATN_INT_VECT    PCINT0_vect
static inline void iec_interrupts_init(void) {
  PCMSK0 = _BV(PCINT0);
  PCIFR |= _BV(PCIF0);
}

/* CLK interrupt (not required) */
/* Dreamload requires interrupts for both the ATN and CLK lines. If both are served by */
/* the same PCINT vector, define that as ATN interrupt above and define IEC_PCMSK.     */
//#  define IEC_PCMSK             PCMSK0
/* If the CLK line has its own dedicated interrupt, use the following definitions: */
//#  define IEC_CLK_INT           INT5
//#  define IEC_CLK_INT_VECT      INT5_vect
//static inline void iec_clock_int_setup(void) {
//  EICRB |= _BV(ISC50);
//}


/*** IEEE signals ***/
/* not documented yet, look at petSD/petSD+ for guidance */

/*** User interface ***/
/* Button NEXT changes to the next disk image and enables sleep mode (held) */
#  define BUTTON_NEXT _BV(PC4)

/* Button PREV changes to the previous disk image */
#  define BUTTON_PREV _BV(PC3)

/* Read the raw button state - a depressed button should read as 0 */
static inline rawbutton_t buttons_read(void) {
  return PINC & (BUTTON_NEXT | BUTTON_PREV);
}

static inline void buttons_init(void) {
  DDRC  &= (uint8_t)~(BUTTON_NEXT | BUTTON_PREV);
  PORTC |= BUTTON_NEXT | BUTTON_PREV;
}

/* Software I2C lines for the RTC and display */
#  define SOFTI2C_PORT    PORTC
#  define SOFTI2C_PIN     PINC
#  define SOFTI2C_DDR     DDRC
#  define SOFTI2C_BIT_SCL PC4
#  define SOFTI2C_BIT_SDA PC5
#  define SOFTI2C_DELAY   6


/*** board-specific initialisation ***/
/* Currently used on uIEC/CF and uIEC/SD only */
//#define HAVE_EARLY_BOARD_INIT
//static inline void early_board_init(void) {
//  // turn on power LED
//  DDRG  |= _BV(PG1);
//  PORTG |= _BV(PG1);
//}


/* Pre-configurated hardware variants */

#elif CONFIG_HARDWARE_VARIANT==HW_SHADOWOLF1
/* ---------- Hardware configuration: Shadowolf 1 ---------- */
#  define HAVE_SD
#  define SD_CHANGE_HANDLER     ISR(INT0_vect)
#  define SD_SUPPLY_VOLTAGE     (1L<<18)

/* 250kHz slow, 2MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

static inline void sdcard_interface_init(void) {
  DDRD &= ~_BV(PD2);
  PORTD |= _BV(PD2);
  DDRD &= ~_BV(PD6);
  PORTD |= _BV(PD6);
  EICRA |= _BV(ISC00);
  EIMSK |= _BV(INT0);
}

static inline uint8_t sdcard_detect(void) {
  return !(PIND & _BV(PD2));
}

static inline uint8_t sdcard_wp(void) {
  return PIND & _BV(PD6);
}

static inline uint8_t device_hw_address(void) {
  return 8 + !(PIND & _BV(PD7)) + 2*!(PIND & _BV(PD5));
}

static inline void device_hw_address_init(void) {
  DDRD  &= ~(_BV(PD7)|_BV(PD5));
  PORTD |=   _BV(PD7)|_BV(PD5);
}

static inline void leds_init(void) {
  DDRC |= _BV(PC0);
  DDRC |= _BV(PC1);
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
  if (state)
    PORTC |= _BV(PC0);
  else
    PORTC &= ~_BV(PC0);
}

#  define LED_DIRTY_PORT        PORTC
#  define LED_DIRTY_INPUT       PINC
#  define LED_DIRTY_PIN         PC1


#  define IEC_INPUT             PINA
#  define IEC_DDR               DDRA
#  define IEC_PORT              PORTA
#  define IEC_PIN_ATN           PA0
#  define IEC_PIN_DATA          PA1
#  define IEC_PIN_CLOCK         PA2
#  define IEC_PIN_SRQ           PA3
#  define IEC_ATN_INT_VECT      PCINT0_vect
#  define IEC_PCMSK             PCMSK0

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE0);
  PCIFR |= _BV(PCIF0);
}

#  define BUTTON_NEXT           _BV(PC4)
#  define BUTTON_PREV           _BV(PC3)

static inline rawbutton_t buttons_read(void) {
  return PINC & (BUTTON_NEXT | BUTTON_PREV);
}

static inline void buttons_init(void) {
  DDRC  &= (uint8_t)~(BUTTON_NEXT | BUTTON_PREV);
  PORTC |= BUTTON_NEXT | BUTTON_PREV;
}


#elif CONFIG_HARDWARE_VARIANT == HW_LARSP
/* ---------- Hardware configuration: LarsP ---------- */
#  define HAVE_SD
#  define SD_CHANGE_HANDLER     ISR(INT0_vect)
#  define SD_SUPPLY_VOLTAGE     (1L<<21)

#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

static inline void sdcard_interface_init(void) {
  DDRD  &= ~_BV(PD2);
  PORTD |=  _BV(PD2);
  DDRD  &= ~_BV(PD6);
  PORTD |=  _BV(PD6);
  EICRA |=  _BV(ISC00);
  EIMSK |=  _BV(INT0);
}

static inline uint8_t sdcard_detect(void) {
  return !(PIND & _BV(PD2));
}

static inline uint8_t sdcard_wp(void) {
  return PIND & _BV(PD6);
}

static inline uint8_t device_hw_address(void) {
  return 8 + !(PINA & _BV(PA2)) + 2*!(PINA & _BV(PA3));
}

static inline void device_hw_address_init(void) {
  DDRA  &= ~(_BV(PA2)|_BV(PA3));
  PORTA |=   _BV(PA2)|_BV(PA3);
}

static inline void leds_init(void) {
  DDRA |= _BV(PA0);
  DDRA |= _BV(PA1);
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
  if (state)
    PORTA &= ~_BV(PA0);
  else
    PORTA |= _BV(PA0);
}

#  define LED_DIRTY_PORT        PORTA
#  define LED_DIRTY_INPUT       PINA
#  define LED_DIRTY_PIN         PA1


#  define IEC_INPUT             PINC
#  define IEC_DDR               DDRC
#  define IEC_PORT              PORTC
#  define IEC_PIN_ATN           PC0
#  define IEC_PIN_DATA          PC1
#  define IEC_PIN_CLOCK         PC2
#  define IEC_PIN_SRQ           PC3
#  define IEC_ATN_INT_VECT      PCINT2_vect
#  define IEC_PCMSK             PCMSK2

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE2);
  PCIFR |= _BV(PCIF2);
}

#  define BUTTON_NEXT           _BV(PA5)
#  define BUTTON_SELECT         _BV(PA4)

static inline void buttons_init(void) {
  DDRA  &= (uint8_t)~(BUTTON_NEXT | BUTTON_SELECT);
  PORTA |= BUTTON_NEXT | BUTTON_SELECT;
}

#  define SOFTI2C_PORT          PORTC
#  define SOFTI2C_PIN           PINC
#  define SOFTI2C_DDR           DDRC
#  define SOFTI2C_BIT_SCL       PC6
#  define SOFTI2C_BIT_SDA       PC5
#  define SOFTI2C_BIT_INTRQ     PC7
#  define SOFTI2C_DELAY         6


#elif CONFIG_HARDWARE_VARIANT == HW_UIEC
/* ---------- Hardware configuration: uIEC ---------- */
/* Note: This CONFIG_HARDWARE_VARIANT number is tested in system.c */
#  define HAVE_ATA
#  ifndef CONFIG_NO_SD
#    define HAVE_SD
#  endif
#  define SPI_LATE_INIT
#  define CF_CHANGE_HANDLER     ISR(INT7_vect)
#  define SD_CHANGE_HANDLER     ISR(PCINT0_vect)
#  define SD_SUPPLY_VOLTAGE     (1L<<21)

/* 250kHz slow, 2MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

#  define SINGLE_LED

static inline void cfcard_interface_init(void) {
  DDRE  &= ~_BV(PE7);
  PORTE |=  _BV(PE7);
  EICRB |=  _BV(ISC70);
  EIMSK |=  _BV(INT7);
}

static inline uint8_t cfcard_detect(void) {
  return !(PINE & _BV(PE7));
}

static inline void sdcard_interface_init(void) {
  DDRB   &= ~_BV(PB7);
  PORTB  |=  _BV(PB7);
  DDRB   &= ~_BV(PB6);
  PORTB  |=  _BV(PB6);
  PCMSK0 |=  _BV(PCINT7);
  PCICR  |=  _BV(PCIE0);
  PCIFR  |=  _BV(PCIF0);
}

static inline uint8_t sdcard_detect(void) {
  return !(PINB & _BV(PB7));
}

static inline uint8_t sdcard_wp(void) {
  return PINB & _BV(PB6);
}

static inline uint8_t device_hw_address(void) {
  /* No device jumpers on uIEC */
  return CONFIG_DEFAULT_ADDR;
}

static inline void device_hw_address_init(void) {
  return;
}

static inline void leds_init(void) {
  DDRE |= _BV(PE3);
}

static inline __attribute__((always_inline)) void set_led(uint8_t state) {
  if (state)
    PORTE |= _BV(PE3);
  else
    PORTE &= ~_BV(PE3);
}

static inline void toggle_led(void) {
  PINE |= _BV(PE3);
}

#  define IEC_INPUT             PINE
#  define IEC_DDR               DDRE
#  define IEC_PORT              PORTE
#  define IEC_PIN_ATN           PE6
#  define IEC_PIN_DATA          PE4
#  define IEC_PIN_CLOCK         PE5
#  define IEC_PIN_SRQ           PE2
#  define IEC_ATN_INT           INT6
#  define IEC_ATN_INT_VECT      INT6_vect
#  define IEC_CLK_INT           INT5
#  define IEC_CLK_INT_VECT      INT5_vect

static inline void iec_interrupts_init(void) {
  EICRB |= _BV(ISC60);
  EICRB |= _BV(ISC50);
}

#  define BUTTON_NEXT           _BV(PG4)
#  define BUTTON_PREV           _BV(PG3)

static inline rawbutton_t buttons_read(void) {
  return PING & (BUTTON_NEXT | BUTTON_PREV);
}

static inline void buttons_init(void) {
  DDRG  &= (uint8_t)~(BUTTON_NEXT | BUTTON_PREV);
  PORTG |= BUTTON_NEXT | BUTTON_PREV;
}

#  define SOFTI2C_PORT          PORTD
#  define SOFTI2C_PIN           PIND
#  define SOFTI2C_DDR           DDRD
#  define SOFTI2C_BIT_SCL       PD0
#  define SOFTI2C_BIT_SDA       PD1
#  define SOFTI2C_BIT_INTRQ     PD2
#  define SOFTI2C_DELAY         6

/* parallel cable - conflicts with the SOFTI2C pins above! */
#  ifdef CONFIG_NO_SD
#    define HAVE_PARALLEL
#    define PARALLEL_HANDLER      ISR(PCINT0_vect)
#    define PARALLEL_PDDR         DDRD      // CONN2 pins 1,3,...,15
#    define PARALLEL_PPORT        PORTD
#    define PARALLEL_PPIN         PIND
#    define PARALLEL_HDDR         DDRB
#    define PARALLEL_HPORT        PORTB
#    define PARALLEL_HPIN         PINB
#    define PARALLEL_HSK_OUT_BIT  5         // CONN2 pin 14, to C64 FLAG2
#    define PARALLEL_HSK_IN_BIT   4         // CONN2 pin 16, to C64 PC2
#    define PARALLEL_PCMSK        PCMSK0
#    define PARALLEL_PCINT_GROUP  0
#  elif defined(CONFIG_PARALLEL_DOLPHIN)
#    error CONFIG_PARALLEL_DOLPHIN on uIEC requires CONFIG_NO_SD=y !
#  endif

/* Use diskmux code to optionally turn off second IDE drive */
#  define NEED_DISKMUX

#  define HAVE_EARLY_BOARD_INIT

static inline void early_board_init(void) {
  /* Force control lines of the external SRAM high */
  DDRG  = _BV(PG0) | _BV(PG1) | _BV(PG2);
  PORTG = _BV(PG0) | _BV(PG1) | _BV(PG2);
}


#elif CONFIG_HARDWARE_VARIANT==HW_SHADOWOLF2
/* ---------- Hardware configuration: Shadowolf 2 aka sd2iec 1.x ---------- */
#  define HAVE_SD
#  define SD_CHANGE_HANDLER     ISR(INT0_vect)
#  define SD2_CHANGE_HANDLER    ISR(INT2_vect)
#  define SD_SUPPLY_VOLTAGE     (1L<<18)

/* 250kHz slow, 2MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

static inline void sdcard_interface_init(void) {
  DDRD  &= ~_BV(PD2);
  PORTD |=  _BV(PD2);
  DDRD  &= ~_BV(PD6);
  PORTD |=  _BV(PD6);
  EICRA |=  _BV(ISC00);
  EIMSK |=  _BV(INT0);
#ifdef CONFIG_TWINSD
  PORTD |=  _BV(PD3); // CS
  DDRD  |=  _BV(PD3); // CS
  DDRC  &= ~_BV(PC7); // WP
  PORTC |=  _BV(PC7); // WP
  DDRB  &= ~_BV(PB2); // Detect
  PORTB |=  _BV(PB2); // Detect
  EICRA |=  _BV(ISC20); // Change interrupt
  EIMSK |=  _BV(INT2);  // Change interrupt
#endif
}

static inline uint8_t sdcard_detect(void) {
  return !(PIND & _BV(PD2));
}

static inline uint8_t sdcard_wp(void) {
  return PIND & _BV(PD6);
}

static inline uint8_t sdcard2_detect(void) {
  return !(PINB & _BV(PB2));
}

static inline uint8_t sdcard2_wp(void) {
  return PINC & _BV(PC7);
}

static inline __attribute__((always_inline)) void sdcard2_set_ss(uint8_t state) {
  if (state)
    PORTD |= _BV(PD3);
  else
    PORTD &= ~_BV(PD3);
}

static inline uint8_t device_hw_address(void) {
  return 8 + !(PIND & _BV(PD7)) + 2*!(PIND & _BV(PD5));
}

static inline void device_hw_address_init(void) {
  DDRD  &= ~(_BV(PD7)|_BV(PD5));
  PORTD |=   _BV(PD7)|_BV(PD5);
}

static inline void leds_init(void) {
  DDRC |= _BV(PC0);
  DDRC |= _BV(PC1);
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
  if (state)
    PORTC |= _BV(PC0);
  else
    PORTC &= ~_BV(PC0);
}

#  define LED_DIRTY_PORT        PORTC
#  define LED_DIRTY_INPUT       PINC
#  define LED_DIRTY_PIN         PC1


#  define IEC_INPUT             PINA
#  define IEC_DDR               DDRA
#  define IEC_PORT              PORTA
#  define IEC_PIN_ATN           PA0
#  define IEC_PIN_DATA          PA1
#  define IEC_PIN_CLOCK         PA2
#  define IEC_PIN_SRQ           PA3
#  define IEC_SEPARATE_OUT
#  define IEC_OPIN_ATN          PA4
#  define IEC_OPIN_DATA         PA5
#  define IEC_OPIN_CLOCK        PA6
#  define IEC_OPIN_SRQ          PA7
#  define IEC_ATN_INT_VECT      PCINT0_vect
#  define IEC_PCMSK             PCMSK0

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE0);
  PCIFR |= _BV(PCIF0);
}

#  define BUTTON_NEXT           _BV(PC3)
#  define BUTTON_PREV           _BV(PC2)

static inline rawbutton_t buttons_read(void) {
  return PINC & (BUTTON_NEXT | BUTTON_PREV);
}

static inline void buttons_init(void) {
  DDRC  &= (uint8_t)~(BUTTON_NEXT | BUTTON_PREV);
  PORTC |= BUTTON_NEXT | BUTTON_PREV;
}

#  define SOFTI2C_PORT          PORTC
#  define SOFTI2C_PIN           PINC
#  define SOFTI2C_DDR           DDRC
#  define SOFTI2C_BIT_SCL       PC4
#  define SOFTI2C_BIT_SDA       PC5
#  define SOFTI2C_BIT_INTRQ     PC6
#  define SOFTI2C_DELAY         6


/* Hardware configuration 6 was old NKC MMC2IEC */


#elif CONFIG_HARDWARE_VARIANT == HW_UIECV3
/* ---------- Hardware configuration: uIEC v3 ---------- */
#  define HAVE_SD
#  define SD_CHANGE_HANDLER     ISR(INT6_vect)
#  define SD_SUPPLY_VOLTAGE     (1L<<21)

/* 250kHz slow, 2MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

#  define SINGLE_LED

static inline void sdcard_interface_init(void) {
  DDRE  &= ~_BV(PE6);
  PORTE |=  _BV(PE6);
  DDRE  &= ~_BV(PE2);
  PORTE |=  _BV(PE2);
  EICRB |=  _BV(ISC60);
  EIMSK |=  _BV(INT6);
}

static inline uint8_t sdcard_detect(void) {
  return !(PINE & _BV(PE6));
}

static inline uint8_t sdcard_wp(void) {
  return PINE & _BV(PE2);
}

static inline uint8_t device_hw_address(void) {
  /* No device jumpers on uIEC */
  return CONFIG_DEFAULT_ADDR;
}

static inline void device_hw_address_init(void) {
  return;
}

static inline void leds_init(void) {
  DDRG |= _BV(PG0);
}

static inline __attribute__((always_inline)) void set_led(uint8_t state) {
  if (state)
    PORTG |= _BV(PG0);
  else
    PORTG &= ~_BV(PG0);
}

static inline void toggle_led(void) {
  PING |= _BV(PG0);
}

#  define IEC_INPUT             PINB
#  define IEC_DDRIN             DDRB
#  define IEC_PORTIN            PORTB
#  define IEC_PIN_ATN           PB4
#  define IEC_PIN_DATA          PB5
#  define IEC_PIN_CLOCK         PB6
#  define IEC_PIN_SRQ           PB7
#  define IEC_SEPARATE_OUT
#  define IEC_PORT              PORTD
#  define IEC_DDROUT            DDRD
#  define IEC_OPIN_ATN          PD4
#  define IEC_OPIN_DATA         PD5
#  define IEC_OPIN_CLOCK        PD6
#  define IEC_OPIN_SRQ          PD7
#  define IEC_ATN_INT_VECT      PCINT0_vect
#  define IEC_PCMSK             PCMSK0

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE0);
  PCIFR |= _BV(PCIF0);
}

#  define BUTTON_NEXT           _BV(PG4)
#  define BUTTON_PREV           _BV(PG3)

static inline rawbutton_t buttons_read(void) {
  return PING & (BUTTON_NEXT | BUTTON_PREV);
}

static inline void buttons_init(void) {
  DDRG  &= (uint8_t)~(BUTTON_NEXT | BUTTON_PREV);
  PORTG |= BUTTON_NEXT | BUTTON_PREV;
}

#  define HAVE_EARLY_BOARD_INIT

static inline void early_board_init(void) {
  // turn on power LED
  DDRG  |= _BV(PG1);
  PORTG |= _BV(PG1);
}

#elif CONFIG_HARDWARE_VARIANT == HW_PETSD
/* ---------- Hardware configuration: petSD ---------- */
#  define HAVE_SD
#  define SD_CHANGE_HANDLER     ISR(PCINT3_vect)
#  define SD_SUPPLY_VOLTAGE (1L<<21)

/* 288 kHz slow, 2.304 MHz fast */
#  define SPI_DIVISOR_SLOW 64
#  define SPI_DIVISOR_FAST 8

static inline void sdcard_interface_init(void) {
  DDRD   &= ~_BV(PD4);            /* card detect */
  PORTD  |=  _BV(PD4);
  DDRC   &= ~_BV(PC3);            /* write protect  */
  PORTC  |=  _BV(PC3);
  PCMSK3 |=  _BV(PCINT28);        /* card change interrupt */
  PCICR  |=  _BV(PCIE3);
  PCIFR  |=  _BV(PCIF3);
}

static inline uint8_t sdcard_detect(void) {
  return (!(PIND & _BV(PD4)));
}

static inline uint8_t sdcard_wp(void) {
  return (PINC & _BV(PC3));
}

static inline uint8_t device_hw_address(void) {
  /* No device jumpers on petSD */
  return CONFIG_DEFAULT_ADDR;
}
static inline void device_hw_address_init(void) {
  return;
}

static inline void leds_init(void) {
  DDRD |= _BV(PD5);
  DDRD |= _BV(PD6);
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
  if (state)
    PORTD |= _BV(PD5);
  else
    PORTD &= (uint8_t) ~_BV(PD5);
}

#  define LED_DIRTY_PORT        PORTD
#  define LED_DIRTY_INPUT       PIND
#  define LED_DIRTY_PIN         PD6


#  define IEEE_ATN_INT          INT0    /* ATN interrupt (required!) */
#  define IEEE_ATN_INT0

#  define HAVE_7516X            /* Device uses 75160/75161 bus drivers */
#  define IEEE_PORT_TE          PORTB   /* TE */
#  define IEEE_DDR_TE           DDRB
#  define IEEE_PIN_TE           PB0
#  define IEEE_PORT_DC          PORTC   /* DC */
#  define IEEE_DDR_DC           DDRC
#  define IEEE_PIN_DC           PC5
#  define IEEE_INPUT_ATN        PIND    /* ATN */
#  define IEEE_PORT_ATN         PORTD
#  define IEEE_DDR_ATN          DDRD
#  define IEEE_PIN_ATN          PD2
#  define IEEE_INPUT_NDAC       PINC    /* NDAC */
#  define IEEE_PORT_NDAC        PORTC
#  define IEEE_DDR_NDAC         DDRC
#  define IEEE_PIN_NDAC         PC6
#  define IEEE_INPUT_NRFD       PINC    /* NRFD */
#  define IEEE_PORT_NRFD        PORTC
#  define IEEE_DDR_NRFD         DDRC
#  define IEEE_PIN_NRFD         PC7
#  define IEEE_INPUT_DAV        PINB    /* DAV */
#  define IEEE_PORT_DAV         PORTB
#  define IEEE_DDR_DAV          DDRB
#  define IEEE_PIN_DAV          PB2
#  define IEEE_INPUT_EOI        PIND    /* EOI */
#  define IEEE_PORT_EOI         PORTD
#  define IEEE_DDR_EOI          DDRD
#  define IEEE_PIN_EOI          PD7
#  define IEEE_D_PIN            PINA    /* Data */
#  define IEEE_D_PORT           PORTA
#  define IEEE_D_DDR            DDRA
/* IFC is only used if ethernet chip is not detected because
   the hardware shares the port pin with ETINT ethernet interrupt */
#  define IEEE_INPUT_IFC        PIND    /* IFC */
#  define IEEE_PORT_IFC         PORTD
#  define IEEE_DDR_IFC          DDRD
#  define IEEE_PIN_IFC          PD3

static inline void ieee_interface_init(void) {
  IEEE_PORT_TE  &= ~_BV(IEEE_PIN_TE);   // Set TE low
  IEEE_PORT_DC  |= _BV(IEEE_PIN_DC);    // Set DC high
  IEEE_DDR_TE   |= _BV(IEEE_PIN_TE);    // Define TE as output
  IEEE_DDR_DC   |= _BV(IEEE_PIN_DC);    // Define DC as output
  IEEE_PORT_ATN |= _BV(IEEE_PIN_ATN);   // Enable pull-up for ATN
  IEEE_DDR_ATN  &= ~ _BV(IEEE_PIN_ATN); // Define ATN as input
}

static inline void buttons_init(void) {
  DDRB  &= ~(_BV(PB1) | _BV(PB3));
  PORTB |= _BV(PB1) | _BV(PB3);
}

#  define SOFTI2C_PORT          PORTC
#  define SOFTI2C_PIN           PINC
#  define SOFTI2C_DDR           DDRC
#  define SOFTI2C_BIT_SCL       PC0
#  define SOFTI2C_BIT_SDA       PC1
#  define SOFTI2C_BIT_INTRQ     PC2
#  define SOFTI2C_DELAY         6

#  define HAVE_EARLY_BOARD_INIT
#  define ENC28J60_CONTROL_PORT PORTC
#  define ENC28J60_CONTROL_CS   PC4
#  define SAME_PORT_FOR_IFC_AND_ENC28J60_ETINT

static inline void early_board_init(void) {
  DDRC  |= _BV(PC4);
  PORTC |= _BV(PC4);       /* Disable  ENC28J60 */
}


#elif CONFIG_HARDWARE_VARIANT == HW_PETSDPLUS
/* ---------- Hardware configuration: petSD+ --------- */
#  define HAVE_SD
#  define IEC_SLOW_IEEE_FAST
#ifndef CONFIG_IGNORE_CARD_DETECT
#  define SD_CHANGE_HANDLER     ISR(PCINT3_vect)
#endif // !CONFIG_IGNORE_CARD_DETECT
#  define SD_SUPPLY_VOLTAGE (1L<<21)

/* 250 kHz slow, 2 MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

static inline void sdcard_interface_init(void) {
  DDRD   &= ~_BV(PD5);            /* card detect */
  PORTD  |=  _BV(PD5);
  DDRD   &= ~_BV(PD6);            /* write protect  */
  PORTD  |=  _BV(PD6);
#ifndef CONFIG_IGNORE_CARD_DETECT
  PCMSK3 |=  _BV(PCINT29);        /* card change interrupt */
  PCICR  |=  _BV(PCIE3);
  PCIFR  |=  _BV(PCIF3);
#endif // !CONFIG_IGNORE_CARD_DETECT
}

static inline uint8_t sdcard_detect(void) {
#ifdef CONFIG_IGNORE_CARD_DETECT
  return 1;
#else
  return (!(PIND & _BV(PD5)));
#endif // CONFIG_IGNORE_CARD_DETECT
}

static inline uint8_t sdcard_wp(void) {
  return (PIND & _BV(PD6));
}


// The busy-LED and the UART's TxD share the same port pin, so light the
// LED only when debug messages are de-selected:

static inline void leds_init(void) {
  DDRD |= _BV(PD0);
#ifndef CONFIG_UART_DEBUG
  DDRD |= _BV(PD1);
#endif // !CONFIG_UART_DEBUG
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
#ifndef CONFIG_UART_DEBUG
  if (state)
    PORTD |= _BV(PD1);
  else
    PORTD &= ~_BV(PD1);
#endif // !CONFIG_UART_DEBUG
}

#  define LED_DIRTY_PORT        PORTD
#  define LED_DIRTY_INPUT       PIND
#  define LED_DIRTY_PIN         PD0


#  define IEEE_ATN_INT          INT0    /* ATN interrupt (required!) */
#  define IEEE_ATN_INT0

#  define HAVE_7516X            /* Device uses 75160/75161 bus drivers */
#  define IEEE_PORT_TE          PORTC   /* TE */
#  define IEEE_DDR_TE           DDRC
#  define IEEE_PIN_TE           PC3
#  define IEEE_INPUT_ATN        PIND    /* ATN */
#  define IEEE_PORT_ATN         PORTD
#  define IEEE_DDR_ATN          DDRD
#  define IEEE_PIN_ATN          PD2
#  define IEEE_INPUT_NDAC       PINC    /* NDAC */
#  define IEEE_PORT_NDAC        PORTC
#  define IEEE_DDR_NDAC         DDRC
#  define IEEE_PIN_NDAC         PC6
#  define IEEE_INPUT_NRFD       PINC    /* NRFD */
#  define IEEE_PORT_NRFD        PORTC
#  define IEEE_DDR_NRFD         DDRC
#  define IEEE_PIN_NRFD         PC7
#  define IEEE_INPUT_DAV        PINC    /* DAV */
#  define IEEE_PORT_DAV         PORTC
#  define IEEE_DDR_DAV          DDRC
#  define IEEE_PIN_DAV          PC5
#  define IEEE_INPUT_EOI        PINC    /* EOI */
#  define IEEE_PORT_EOI         PORTC
#  define IEEE_DDR_EOI          DDRC
#  define IEEE_PIN_EOI          PC4
#  define IEEE_INPUT_IFC        PINC    /* IFC */
#  define IEEE_PORT_IFC         PORTC
#  define IEEE_DDR_IFC          DDRC
#  define IEEE_PIN_IFC          PC2
#  define IEEE_D_PIN            PINA    /* Data */
#  define IEEE_D_PORT           PORTA
#  define IEEE_D_DDR            DDRA
#  define IEEE_INPUT_D7         PIND    /* Data bit 7 on separate port */
#  define IEEE_PORT_D7          PORTD
#  define IEEE_DDR_D7           DDRD
#  define IEEE_PIN_D7           PD7
#  define IEEE_BIT_TE           _BV(IEEE_PIN_TE)
#  define IEEE_BIT_IFC          _BV(IEEE_PIN_IFC)



static inline void buttons_init(void) {
  // AVcc as voltage reference, select ADC7
  ADMUX |= _BV(REFS0) | _BV(MUX0) | _BV(MUX1) | _BV(MUX2);

  // disable digitial input register for PA7
  DIDR0 |= _BV(ADC7D);

  // divider 128: 16 MHz / 128 = 125 kHz (50 kHz..200 kHz)
  // enable ADC
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);

  // Start first conversion
  ADCSRA |= _BV(ADSC);

  // Wait for conversion to complete
  while (ADCSRA & _BV(ADSC));

  // dummy read of result
  (void) ADCW;
}


#include "config.h"
#include "eeprom-conf.h"
#include "menu.h"
#include "timer.h"
#include <stdio.h>

static inline void device_hw_address_init(void) {
  // left intentionally blank
}

static inline uint8_t device_hw_address(void) {
  uint8_t addr;
  if (menu_system_enabled) {
    addr = CONFIG_DEFAULT_ADDR;
  } else {
    delay_ms(500); // allow debouncing
    addr = 8;
    if (get_key_state(KEY_SEL))  addr += 1;
    if (get_key_state(KEY_NEXT)) addr += 2;
    printf("dhw:%d\r\n", addr);
  }
  return addr;
}


#  define SOFTI2C_PORT          PORTC
#  define SOFTI2C_PIN           PINC
#  define SOFTI2C_DDR           DDRC
#  define SOFTI2C_BIT_SCL       PC0
#  define SOFTI2C_BIT_SDA       PC1
#  define SOFTI2C_DELAY         6

// Definitions for I2C PWM slave on petSD+ 2.x boards
#  include                      "i2c-lcd-pwm-io-config.h"

#  define LCD_PORT_E            PORTD
#  define LCD_DDR_E             DDRD
#  define LCD_PIN_E             PD4
#  define LCD_PORT_RS           PORTD
#  define LCD_DDR_RS            DDRD
#  define LCD_PIN_RS            PD3
#  define LCD_PORT_DATA         PORTB
#  define LCD_DDR_DATA          DDRB
#  define LCD_LINES             4
#  define LCD_COLS              20
#  define LCD_ADDR_LINE1        0
#  define LCD_ADDR_LINE2        64
#  define LCD_ADDR_LINE3        20
#  define LCD_ADDR_LINE4        84


#  ifdef CONFIG_ONBOARD_DISPLAY
#    define HAVE_EARLY_BOARD_INIT
#    define HAVE_LATE_BOARD_INIT
#include "lcd.h"
#include "diagnose.h"
#include "timer.h"

static inline void early_board_init(void) {
#ifdef CONFIG_HAVE_IEC
  IEEE_DDR_TE |= _BV(IEEE_PIN_TE);      // TE as output
  IEEE_PORT_TE &= ~_BV(IEEE_PIN_TE);    // TE low (listen mode)
#endif // CONFIG_HAVE_IEC
}

static inline void late_board_init(void) {
  lcd_init();
  lcd_bootscreen();
  buttons_init();
  uint16_t buttons = ADCW;
  if (buttons > 580 && buttons < 630) {
    board_diagnose();
  } else if (buttons >= 630 && buttons < 680) {
    lcd_set_brightness(0xFF);                       // maximum brightness
    menu_adjust_contrast();
  }
}

#include "i2c.h"

static inline void ieee_interface_init(void) {
  IEEE_PORT_TE  &= (uint8_t) ~ IEEE_BIT_TE;         // Set TE low
  IEEE_DDR_TE   |= IEEE_BIT_TE;                     // Define TE  as output
  IEEE_PORT_ATN |= _BV(IEEE_PIN_ATN);               // Enable pull-up for ATN
  IEEE_PORT_IFC |= IEEE_BIT_IFC;                    // Enable pull-up for IFC
  IEEE_DDR_ATN  &= (uint8_t) ~ _BV(IEEE_PIN_ATN);   // Define ATN as input
  IEEE_DDR_IFC  &= (uint8_t) ~ IEEE_BIT_IFC;        // Define IFC as input
  i2c_write_register(I2C_SLAVE_ADDRESS, IO_IEC, 1); // Enable IEEE-488 bus
}

#ifdef CONFIG_HAVE_IEC
#  define IEC_OUTPUTS_NONINVERTED
#  define IEC_INPUT             PINC
#  define IEC_DDR               DDRC
#  define IEC_PORT              PORTC
#  define IEC_PIN_ATN           PC2
#  define IEC_PIN_DATA          PC4
#  define IEC_PIN_CLOCK         PC5
#  define IEC_PIN_SRQ           0
#  define IEC_SEPARATE_OUT
#  define IEC_OPIN_ATN          0
#  define IEC_OPIN_DATA         PC6
#  define IEC_OPIN_CLOCK        PC7
#  define IEC_OPIN_SRQ          0
#  define IEC_ATN_INT_VECT      PCINT2_vect
#  define IEC_PCMSK             PCMSK2

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE2);
  PCIFR |= _BV(PCIF2);
}
#endif // CONFIG_HAVE_IEC



#  endif // CONFIG_ONBOARD_DISPLAY
#elif CONFIG_HARDWARE_VARIANT == HW_PETSDLITE
/* ---------- Hardware configuration: petSD- --------- */
#  define HAVE_SD
#  define IEC_SLOW_IEEE_FAST
#ifndef CONFIG_IGNORE_CARD_DETECT
#  define SD_CHANGE_HANDLER     ISR(PCINT3_vect)
#endif // !CONFIG_IGNORE_CARD_DETECT
#  define SD_SUPPLY_VOLTAGE (1L<<21)

/* 250 kHz slow, 2 MHz fast */
#  define SPI_DIVISOR_SLOW 32
#  define SPI_DIVISOR_FAST 4

static inline void sdcard_interface_init(void) {
  DDRD   &= ~_BV(PD5);            /* card detect */
  PORTD  |=  _BV(PD5);
  DDRD   &= ~_BV(PD6);            /* write protect  */
  PORTD  |=  _BV(PD6);
#ifndef CONFIG_IGNORE_CARD_DETECT
  PCMSK3 |=  _BV(PCINT29);        /* card change interrupt */
  PCICR  |=  _BV(PCIE3);
  PCIFR  |=  _BV(PCIF3);
#endif // !CONFIG_IGNORE_CARD_DETECT
}

static inline uint8_t sdcard_detect(void) {
#ifdef CONFIG_IGNORE_CARD_DETECT
  return 1;
#else
  return (!(PIND & _BV(PD5)));
#endif // CONFIG_IGNORE_CARD_DETECT
}

static inline uint8_t sdcard_wp(void) {
  return (PIND & _BV(PD6));
}


// The busy-LED and the UART's TxD share the same port pin, so light the
// LED only when debug messages are de-selected:

static inline void leds_init(void) {
  DDRD |= _BV(PD0);
#ifndef CONFIG_UART_DEBUG
  DDRD |= _BV(PD1);
#endif // !CONFIG_UART_DEBUG
}

static inline __attribute__((always_inline)) void set_busy_led(uint8_t state) {
#ifndef CONFIG_UART_DEBUG
  if (state)
    PORTD |= _BV(PD1);
  else
    PORTD &= ~_BV(PD1);
#endif // !CONFIG_UART_DEBUG
}

#  define LED_DIRTY_PORT        PORTD
#  define LED_DIRTY_INPUT       PIND
#  define LED_DIRTY_PIN         PD0


#  define IEEE_ATN_INT          INT0    /* ATN interrupt (required!) */
#  define IEEE_ATN_INT0

#  define HAVE_7516X            /* Device uses 75160/75161 bus drivers */
#  define IEEE_PORT_TE          PORTC   /* TE */
#  define IEEE_DDR_TE           DDRC
#  define IEEE_PIN_TE           PC3
#  define IEEE_INPUT_ATN        PIND    /* ATN */
#  define IEEE_PORT_ATN         PORTD
#  define IEEE_DDR_ATN          DDRD
#  define IEEE_PIN_ATN          PD2
#  define IEEE_INPUT_NDAC       PINC    /* NDAC */
#  define IEEE_PORT_NDAC        PORTC
#  define IEEE_DDR_NDAC         DDRC
#  define IEEE_PIN_NDAC         PC6
#  define IEEE_INPUT_NRFD       PINC    /* NRFD */
#  define IEEE_PORT_NRFD        PORTC
#  define IEEE_DDR_NRFD         DDRC
#  define IEEE_PIN_NRFD         PC7
#  define IEEE_INPUT_DAV        PINC    /* DAV */
#  define IEEE_PORT_DAV         PORTC
#  define IEEE_DDR_DAV          DDRC
#  define IEEE_PIN_DAV          PC5
#  define IEEE_INPUT_EOI        PINC    /* EOI */
#  define IEEE_PORT_EOI         PORTC
#  define IEEE_DDR_EOI          DDRC
#  define IEEE_PIN_EOI          PC4
#  define IEEE_INPUT_IFC        PINC    /* IFC */
#  define IEEE_PORT_IFC         PORTC
#  define IEEE_DDR_IFC          DDRC
#  define IEEE_PIN_IFC          PC2
#  define IEEE_D_PIN            PINA    /* Data */
#  define IEEE_D_PORT           PORTA
#  define IEEE_D_DDR            DDRA
#  define IEEE_INPUT_D7         PIND    /* Data bit 7 on separate port */
#  define IEEE_PORT_D7          PORTD
#  define IEEE_DDR_D7           DDRD
#  define IEEE_PIN_D7           PD7
#  define IEEE_BIT_TE           _BV(IEEE_PIN_TE)
#  define IEEE_BIT_IFC          _BV(IEEE_PIN_IFC)



static inline void buttons_init(void) {
  // AVcc as voltage reference, select ADC7
  ADMUX |= _BV(REFS0) | _BV(MUX0) | _BV(MUX1) | _BV(MUX2);

  // disable digitial input register for PA7
  DIDR0 |= _BV(ADC7D);

  // divider 128: 16 MHz / 128 = 125 kHz (50 kHz..200 kHz)
  // enable ADC
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);

  // Start first conversion
  ADCSRA |= _BV(ADSC);

  // Wait for conversion to complete
  while (ADCSRA & _BV(ADSC));

  // dummy read of result
  (void) ADCW;
}


#include "config.h"
#include "eeprom-conf.h"
#include "menu.h"
#include "timer.h"
#include <stdio.h>

static inline void device_hw_address_init(void) {
  // left intentionally blank
}

static inline uint8_t device_hw_address(void) {
  uint8_t addr;
  if (menu_system_enabled) {
    addr = CONFIG_DEFAULT_ADDR;
  } else {
    delay_ms(500); // allow debouncing
    addr = 8;
    if (get_key_state(KEY_SEL))  addr += 1;
    if (get_key_state(KEY_NEXT)) addr += 2;
    printf("dhw:%d\r\n", addr);
  }
  return addr;
}


#  define SOFTI2C_PORT          PORTC
#  define SOFTI2C_PIN           PINC
#  define SOFTI2C_DDR           DDRC
#  define SOFTI2C_BIT_SCL       PC0
#  define SOFTI2C_BIT_SDA       PC1
#  define SOFTI2C_DELAY         6

// Definitions for I2C PWM slave on petSD+ 2.x boards
#  include                      "i2c-lcd-pwm-io-config.h"

#  define LCD_PORT_E            PORTD
#  define LCD_DDR_E             DDRD
#  define LCD_PIN_E             PD4
#  define LCD_PORT_RS           PORTD
#  define LCD_DDR_RS            DDRD
#  define LCD_PIN_RS            PD3
#  define LCD_PORT_DATA         PORTB
#  define LCD_DDR_DATA          DDRB
#  define LCD_LINES             4
#  define LCD_COLS              20
#  define LCD_ADDR_LINE1        0
#  define LCD_ADDR_LINE2        64
#  define LCD_ADDR_LINE3        20
#  define LCD_ADDR_LINE4        84


#  ifdef CONFIG_ONBOARD_DISPLAY
#    define HAVE_EARLY_BOARD_INIT
#    define HAVE_LATE_BOARD_INIT
#include "lcd.h"
#include "diagnose.h"
#include "timer.h"

static inline void early_board_init(void) {
#ifdef CONFIG_HAVE_IEC
  IEEE_DDR_TE |= _BV(IEEE_PIN_TE);      // TE as output
  IEEE_PORT_TE &= ~_BV(IEEE_PIN_TE);    // TE low (listen mode)
#endif // CONFIG_HAVE_IEC
}

static inline void late_board_init(void) {
  lcd_init();
  lcd_bootscreen();
  buttons_init();
  uint16_t buttons = ADCW;
  if (buttons > 580 && buttons < 630) {
    board_diagnose();
  } else if (buttons >= 630 && buttons < 680) {
    lcd_set_brightness(0xFF);                       // maximum brightness
    menu_adjust_contrast();
  }
}

#include "i2c.h"

static inline void ieee_interface_init(void) {
  IEEE_PORT_TE  &= (uint8_t) ~ IEEE_BIT_TE;         // Set TE low
  IEEE_DDR_TE   |= IEEE_BIT_TE;                     // Define TE  as output
  IEEE_PORT_ATN |= _BV(IEEE_PIN_ATN);               // Enable pull-up for ATN
  IEEE_PORT_IFC |= IEEE_BIT_IFC;                    // Enable pull-up for IFC
  IEEE_DDR_ATN  &= (uint8_t) ~ _BV(IEEE_PIN_ATN);   // Define ATN as input
  IEEE_DDR_IFC  &= (uint8_t) ~ IEEE_BIT_IFC;        // Define IFC as input
  i2c_write_register(I2C_SLAVE_ADDRESS, IO_IEC, 1); // Enable IEEE-488 bus
}

#ifdef CONFIG_HAVE_IEC
#  define IEC_OUTPUTS_NONINVERTED
#  define IEC_INPUT             PINC
#  define IEC_DDR               DDRC
#  define IEC_PORT              PORTC
#  define IEC_PIN_ATN           PC2
#  define IEC_PIN_DATA          PC4
#  define IEC_PIN_CLOCK         PC5
#  define IEC_PIN_SRQ           0
#  define IEC_SEPARATE_OUT
#  define IEC_OPIN_ATN          0
#  define IEC_OPIN_DATA         PC6
#  define IEC_OPIN_CLOCK        PC7
#  define IEC_OPIN_SRQ          0
#  define IEC_ATN_INT_VECT      PCINT2_vect
#  define IEC_PCMSK             PCMSK2

static inline void iec_interrupts_init(void) {
  PCICR |= _BV(PCIE2);
  PCIFR |= _BV(PCIF2);
}
#endif // CONFIG_HAVE_IEC



#  endif // CONFIG_ONBOARD_DISPLAY
#else
#  error "CONFIG_HARDWARE_VARIANT is unset or set to an unknown value."
#endif


/* ---------------- End of user-configurable options ---------------- */

#if !defined(CONFIG_HAVE_IEC) && !defined(CONFIG_HAVE_IEEE)
#  error Need CONFIG_HAVE_IEC and/or CONFIG_HAVE_IEEE
// Please edit your config-<devicename> if this error occurs.
#endif

#if defined(CONFIG_HAVE_IEC) && defined(CONFIG_HAVE_IEEE)
#define HAVE_DUAL_INTERFACE
#endif


/* --- IEC --- */
#ifdef CONFIG_HAVE_IEC

#define IEC_BIT_ATN      _BV(IEC_PIN_ATN)
#define IEC_BIT_DATA     _BV(IEC_PIN_DATA)
#define IEC_BIT_CLOCK    _BV(IEC_PIN_CLOCK)
#define IEC_BIT_SRQ      _BV(IEC_PIN_SRQ)

/* Return type of iec_bus_read() */
typedef uint8_t iec_bus_t;

/* OPIN definitions are only used in the assembler module */
#ifdef IEC_SEPARATE_OUT
#  define IEC_OBIT_ATN   _BV(IEC_OPIN_ATN)
#  define IEC_OBIT_DATA  _BV(IEC_OPIN_DATA)
#  define IEC_OBIT_CLOCK _BV(IEC_OPIN_CLOCK)
#  define IEC_OBIT_SRQ   _BV(IEC_OPIN_SRQ)
#  define IEC_OUTPUT     IEC_PORT
#else
#  define IEC_OPIN_ATN   IEC_PIN_ATN
#  define IEC_OPIN_DATA  IEC_PIN_DATA
#  define IEC_OPIN_CLOCK IEC_PIN_CLOCK
#  define IEC_OPIN_SRQ   IEC_PIN_SRQ
#  define IEC_OBIT_ATN   IEC_BIT_ATN
#  define IEC_OBIT_DATA  IEC_BIT_DATA
#  define IEC_OBIT_CLOCK IEC_BIT_CLOCK
#  define IEC_OBIT_SRQ   IEC_BIT_SRQ
#  define IEC_OUTPUT     IEC_DDR
#endif

#ifndef IEC_PORTIN
#  define IEC_PORTIN IEC_PORT
#endif

#ifndef IEC_DDRIN
#  define IEC_DDRIN  IEC_DDR
#  define IEC_DDROUT IEC_DDR
#endif

/* The AVR based devices usually invert output lines, */
/* so this can be the default for most configurations.   */
#ifndef IEC_OUTPUTS_NONINVERTED
#define IEC_OUTPUTS_INVERTED
#endif

#ifdef IEC_PCMSK
   /* For hardware configurations using PCINT for IEC IRQs */
#  define set_iec_atn_irq(x) \
     if (x) { IEC_PCMSK |= _BV(IEC_PIN_ATN); } \
     else { IEC_PCMSK &= (uint8_t)~_BV(IEC_PIN_ATN); }
#  define set_clock_irq(x) \
     if (x) { IEC_PCMSK |= _BV(IEC_PIN_CLOCK); } \
     else { IEC_PCMSK &= (uint8_t)~_BV(IEC_PIN_CLOCK); }
#  define HAVE_CLOCK_IRQ
#else
     /* Hardware ATN interrupt */
#  define set_iec_atn_irq(x) \
     if (x) { EIMSK |= _BV(IEC_ATN_INT); } \
     else { EIMSK &= (uint8_t)~_BV(IEC_ATN_INT); }

#  ifdef IEC_CLK_INT
     /* Hardware has a CLK interrupt */
#    define set_clock_irq(x) \
       if (x) { EIMSK |= _BV(IEC_CLK_INT); } \
       else { EIMSK &= (uint8_t)~_BV(IEC_CLK_INT); }
#    define HAVE_CLOCK_IRQ
#  endif
#endif

/* IEC output functions */
#ifdef IEC_OUTPUTS_INVERTED
#  define COND_INV(x) (!(x))
#else
#  define COND_INV(x) (x)
#endif

static inline __attribute__((always_inline)) void set_atn(uint8_t state) {
  if (COND_INV(state))
    IEC_OUTPUT |= IEC_OBIT_ATN;
  else
    IEC_OUTPUT &= ~IEC_OBIT_ATN;
}

static inline __attribute__((always_inline)) void set_data(uint8_t state) {
  if (COND_INV(state))
    IEC_OUTPUT |= IEC_OBIT_DATA;
  else
    IEC_OUTPUT &= ~IEC_OBIT_DATA;
}

static inline __attribute__((always_inline)) void set_clock(uint8_t state) {
  if (COND_INV(state))
    IEC_OUTPUT |= IEC_OBIT_CLOCK;
  else
    IEC_OUTPUT &= ~IEC_OBIT_CLOCK;
}

#ifdef IEC_SEPARATE_OUT
static inline __attribute__((always_inline)) void set_srq(uint8_t state) {
  if (COND_INV(state))
    IEC_OUTPUT |= IEC_OBIT_SRQ;
  else
    IEC_OUTPUT &= ~IEC_OBIT_SRQ;
}
#else
/* this version of the function turns on the pullups when state is 1 */
/* note: same pin for in/out implies inverted output via DDR */
static inline __attribute__((always_inline)) void set_srq(uint8_t state) {
  if (state) {
    IEC_DDR  &= ~IEC_OBIT_SRQ;
    IEC_PORT |=  IEC_OBIT_SRQ;
  } else {
    IEC_PORT &= ~IEC_OBIT_SRQ;
    IEC_DDR  |=  IEC_OBIT_SRQ;
  }
}
#endif

#undef COND_INV

// for testing purposes only, probably does not do what you want!
#define toggle_srq() IEC_INPUT |= IEC_OBIT_SRQ

/* IEC lines initialisation */
static inline void iec_interface_init(void) {
#ifdef IEC_SEPARATE_OUT
  /* Set up the input port - pullups on all lines */
  IEC_DDRIN  &= (uint8_t)~(IEC_BIT_ATN  | IEC_BIT_CLOCK  | IEC_BIT_DATA  | IEC_BIT_SRQ);
  IEC_PORTIN |= IEC_BIT_ATN | IEC_BIT_CLOCK | IEC_BIT_DATA | IEC_BIT_SRQ;
  /* Set up the output port - all lines high */
  IEC_DDROUT |=            IEC_OBIT_ATN | IEC_OBIT_CLOCK | IEC_OBIT_DATA | IEC_OBIT_SRQ;
#ifdef IEC_OUTPUTS_INVERTED
  IEC_PORT   &= (uint8_t)~(IEC_OBIT_ATN | IEC_OBIT_CLOCK | IEC_OBIT_DATA | IEC_OBIT_SRQ);
#else
  IEC_PORT   |= (IEC_OBIT_ATN | IEC_OBIT_CLOCK | IEC_OBIT_DATA | IEC_OBIT_SRQ);
#endif
#else
  /* Pullups would be nice, but AVR can't switch from */
  /* low output to hi-z input directly                */
  IEC_DDR  &= (uint8_t)~(IEC_BIT_ATN | IEC_BIT_CLOCK | IEC_BIT_DATA | IEC_BIT_SRQ);
  IEC_PORT &= (uint8_t)~(IEC_BIT_ATN | IEC_BIT_CLOCK | IEC_BIT_DATA);
  /* SRQ is special-cased because it may be unconnected */
  IEC_PORT |= IEC_BIT_SRQ;
#endif

#ifdef HAVE_PARALLEL
  /* set data lines to input with pullup */
  PARALLEL_PDDR  = 0;
  PARALLEL_PPORT = 0xff;

  /* set HSK_OUT and _IN to input with pullup */
  PARALLEL_HDDR  &= ~(_BV(PARALLEL_HSK_OUT_BIT) |
                      _BV(PARALLEL_HSK_IN_BIT));
  PARALLEL_HPORT |= _BV(PARALLEL_HSK_OUT_BIT) |
                    _BV(PARALLEL_HSK_IN_BIT);

  /* enable interrupt for parallel handshake */
#  ifdef PARALLEL_PCINT_GROUP
  /* excluse PCINT group */
  PARALLEL_PCMSK |= _BV(PARALLEL_HSK_IN_BIT);
  PCICR |= _BV(PARALLEL_PCINT_GROUP);
  PCIFR |= _BV(PARALLEL_PCINT_GROUP);
#  else
  /* exclusive INTx line */
#    error Implement me!
#  endif
#endif

#if (CONFIG_HARDWARE_VARIANT == HW_PETSDPLUS) || (CONFIG_HARDWARE_VARIANT == HW_PETSDLITE)
  // Enable IEC bus. This is done by a wire link inside the adapter cable
  // for older petSD+ boards without IEC connector
  i2c_write_register(I2C_SLAVE_ADDRESS, IO_IEC, 0);
#endif
}

#endif /* CONFIG_HAVE_IEC */


/* The assembler module needs the vector names, */
/* so the _HANDLER macros are created here.     */
#define IEC_ATN_HANDLER   ISR(IEC_ATN_INT_VECT)
#define IEC_CLOCK_HANDLER ISR(IEC_CLK_INT_VECT)

/* SD SS pin default implementation */
#ifndef SDCARD_SS_SPECIAL
static inline __attribute__((always_inline)) void sdcard_set_ss(uint8_t state) {
  if (state)
    SPI_PORT |= SPI_SS;
  else
    SPI_PORT &= ~SPI_SS;
}
#endif

/* ENC28J60 ethernet interface chip select */
#ifdef SAME_PORT_FOR_IFC_AND_ENC28J60_ETINT
static inline __attribute__((always_inline)) void enc28j60_set_ss(uint8_t state) {
  if (state)
    ENC28J60_CONTROL_PORT |=  _BV(ENC28J60_CONTROL_CS);
  else
    ENC28J60_CONTROL_PORT &= ~_BV(ENC28J60_CONTROL_CS);
}
#endif

/* Display interrupt pin */
#ifdef CONFIG_REMOTE_DISPLAY
static inline void display_intrq_init(void) {
  /* Enable pullup on the interrupt line */
  SOFTI2C_PORT |= _BV(SOFTI2C_BIT_INTRQ);
}

static inline uint8_t display_intrq_active(void) {
  return !(SOFTI2C_PIN & _BV(SOFTI2C_BIT_INTRQ));
}
#endif

/* P00 name cache is in bss by default */
#ifndef P00CACHE_ATTRIB
#  define P00CACHE_ATTRIB
#endif

/* -- ensure that the timing for Dolphin is achievable        -- */
/* the C64 will switch to an alternate, not-implemented protocol */
/* if the answer to the XQ/XZ commands is too late and the       */
/* file name/command dump takes too long if the buffer is        */
/* smaller than the output from uart_trace                       */
#if defined(CONFIG_PARALLEL_DOLPHIN) && \
    defined(CONFIG_UART_DEBUG) && \
  CONFIG_UART_BUF_SHIFT < 8  // 7 may work with short file names
#  error Enabling both DolphinDOS and UART debugging requires CONFIG_UART_BUF_SHIFT >= 8 !
#endif


/* LED functions */
static inline __attribute__((always_inline)) void set_dirty_led(uint8_t state) {
  if (state)
    LED_DIRTY_PORT |= _BV(LED_DIRTY_PIN);
  else
    LED_DIRTY_PORT &= ~_BV(LED_DIRTY_PIN);
}

/* Toggle function used for error blinking */
static inline void toggle_dirty_led(void) {
  /* Sufficiently new AVR cores have a toggle function */
  LED_DIRTY_INPUT |= _BV(LED_DIRTY_PIN);
}


#endif
