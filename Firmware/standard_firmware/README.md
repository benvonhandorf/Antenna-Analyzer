TODO: Update for Si5351a DDS changes

This variant of the Antenna Analyzer firmware is altered to work wih the Si5351a signal generator rather than the original AD9850.  At the moment no effort is being made to unify the firmwares, this will be a quick and dirty tear out and replace operation, because I want to analyze my antenna.

Eventually this will also support an I2C OLED display, which is what I would rather use for a case like this.  In the meantime, it will be optimized for serial interfacing.

Makes use of the [Etherkit Si5351 library](https://github.com/etherkit/Si5351Arduino_) for DDS interfacing.  Assumes normal I2C pins for your Arduino type.