# Raspberry Pi C RGB Led

Another little C program. This time to have a go at software pulse width modulation with an RGB Led. So my C GPIO library using the 'sysfs interface' is slowly moving along! 

The code important to this little project is gpio/pwm.c, gpio/pwm.h and main.c.  This is where all the software pulse width modulation happens to fade and change the colour of the RGB Led.

![rgbLed](https://github.com/mse240966/raspberry-pi-c-rgb-led/blob/master/docs/rgbLed.jpg "RGB Led")

## How to clone (with GPIO sub module)

```bash
git clone --recursive https://github.com/mse240966/raspberry-pi-c-rgb-led.git
```

## Example

```bash
$ sudo ./rgbLed
Fading Red with Green and Blue
Fading Green with Red and Blue
Fading Blue with Red and Green
Fading Red with Green
Fading Red with Blue
Fading Green with Red
Fading Green with Blue
Fading Blue with Red
Fading Blue with Green
Fading Red
Fading Green
Fading Blue
```

## Circuit
