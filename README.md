# "Scrap Brain"
YM2612-based synth controlled by a couple of ATmega328 chips with multiplexed analog inputs and an Adafruit touchscreen.

Contains 2 Arduino projects:
- "panel" provides a touchscreen UI and handles potentiometer readings, merging these with incoming MIDI messages
- "voice" controls the YM2612, responding to MIDI messages (either passed thru or sent by "panel")

The front panels were created in Lightburn and designed for Kitronik opaque black frosted perspex sheets.
I found it useful to burn off some of the material on the back of the panels, behind the jacks and pots.
