/** @copyright 2024 Sean Kasun

This is a simple arduino clock.  The display is a VFD hooked up
to serial out.  There is a DS3231 hooked up to the SCL/SDA inputs.

It doesn't auto adjust for DST, this is just a clock for me, and I'm in AZ,
we don't use DST.

*/
#include <DS3231.h>
#include <Wire.h>

const int SETBTN = 2;
const int PREVBTN = 3;
const int NEXTBTN = 4;
bool setUp = true, set = false;
bool prevUp = true, prev = false;
bool nextUp = true, next = false;

void printNum(byte num, bool pad) {
  if (num < 10) {
    Serial.write(pad ? '0' : ' ');
  } else {
    Serial.write('0' + (num / 10));
  }
  Serial.write('0' + (num % 10));
}

DS3231 rtc;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(SETBTN, INPUT_PULLUP);
  pinMode(PREVBTN, INPUT_PULLUP);
  pinMode(NEXTBTN, INPUT_PULLUP);
  rtc.setClockMode(false);  // 24-hour clock
  while(!Serial);
  Serial.write("\x1f");
  Serial.write("\x11");  // normal display
}

int maxDom[] = {
  0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char *dayNames[] = {
  "",
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
};

const char *monNames[] = {
  "",
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

byte brightLevels[] = {
  0xff, 0x60, 0x40, 0x20
};

byte brightness = 0;

int changing = 0;
int flash = 0;
byte prevMin = 0xff;

void loop() {
  if (!digitalRead(SETBTN)) {
    if (setUp) {
      setUp = false;
      set = true;
    }
  } else {
    set = false;
    setUp = true;
  }
  if (!digitalRead(PREVBTN)) {
    if (prevUp) {
      prevUp = false;
      prev = true;
    }
  } else {
    prev = false;
    prevUp = true;
  }
  if (!digitalRead(NEXTBTN)) {
    if (nextUp) {
      nextUp = false;
      next = true;
    }
  } else {
    next = false;
    nextUp = true;
  }

  bool h12, pm;
  byte hours = rtc.getHour(h12, pm);
  byte minutes = rtc.getMinute();
  if (minutes == prevMin && !set && !changing && !prev && !next) {
    delay(1000);  // sleep for a second
    return;
  }
  prevMin = minutes;
  if (!set && next) {
    brightness++;
    brightness &= 3;
  }
  
  Serial.write("\x14");  // cursor off
  Serial.write("\x10\x05");  // move to position 5
  Serial.write("\x04");  // brightness
  Serial.write(brightLevels[brightness]);
  byte dotw = rtc.getDoW();
  byte day = rtc.getDate();
  bool century;
  byte month = rtc.getMonth(century);
  byte year = rtc.getYear();

  if (set) {
    set = false;
    changing++;
    if (changing == 7) {
      changing = 0;
    }
  }
  if (changing) {
    flash = !flash;
  }

  if (changing == 1 && flash) {
    Serial.write("  ");
    if (prev) {
      hours--;
    } else if (next) {
      hours++;
    }
    hours %= 24;
    rtc.setHour(hours);
  } else {
    printNum(hours, false);
  }
  Serial.write(':');
  if (changing == 2 && flash) {
    Serial.write("  ");
    if (prev) {
      minutes--;
    } else if (next) {
      minutes++;
    }
    minutes %= 60;
    rtc.setMinute(minutes);
  } else {
    printNum(minutes, true);
  }
  Serial.write("\r\n");
  if (changing == 3 && flash) {
    Serial.write("   ");
    if (prev) {
      dotw--;
      if (dotw == 0) {
        dotw = 7;
      }
    } else if (next) {
      dotw++;
      if (dotw == 8) {
        dotw = 1;
      }
    }
    rtc.setDoW(dotw);
  } else {
    Serial.write(dayNames[dotw]);
  }
  Serial.write(' ');
  if (changing == 4 && flash) {
    Serial.write("   ");
    if (prev) {
      month--;
      if (month == 0) {
        month = 12;
      }
    } else if (next) {
      month++;
      if (month == 13) {
        month = 1;
      }
    }
    rtc.setMonth(month);
  } else {
    Serial.write(monNames[month]);
  }
  Serial.write(" ");
  if (changing == 5 && flash) {
    Serial.write("  ");
    if (prev) {
      day--;
      if (day == 0) {
        day = maxDom[month];
      }
    } else if (next) {
      day++;
      if (day > maxDom[month]) {
        day = 1;
      }
    }
    rtc.setDate(day);
  } else {
    printNum(day, false);
  }
  if (changing == 6 && flash) {
    Serial.write("     ");
    if (prev) {
      year--;
    } else if (next) {
      year++;
    }
    year %= 100;
    rtc.setYear(year);
  } else {
    Serial.write(" 20");
    printNum(year, true);
  }

  if (changing) {
    delay(200);  // flash at 2.5 times a second
  } else {
    delay(1000);  // sleep for a second
  }
}

/*
 * TD3000 VFD command set:
 * \x12 ^R - vertical scroll mode
 *   data is written into second row and transfered to first row
 * \x11 ^Q - normal display mode
 *   data written into either row.
 * \x04 ^D - brightness control (followed by FF, 60, 40, or 20)
 * \x08 ^H - Backsspace
 * \x09 ^I - tab
 * \x0a ^J - line feed (switch rows, same position)
 * \x0d ^M - carriage return (move to leftmost)
 * \x10 ^P - digit select (move to any position \x00-\x27)
 * \x13 ^S - Turn on Cursor
 * \x14 ^T - Turn off cursor
 * \x1f ^_ - reset
 */
