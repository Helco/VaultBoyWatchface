/*
VaultBoy - a pebble watchface by Helco

Version: 1.0
License: GNU GPL v3

Confg:

#================#===============#==============================================#
|Option          | Has value?    | Description                                  |
#================#===============#==============================================#
|INVERTED        |      NO       | Inverts the whole watchface no matter what   |
|                |               | other config options are selected            |
+----------------+---------------+----------------------------------------------+
|NIGHT_INDICATOR |      NO       | Inverts the whole watcchface to black (or    |
|                |               | white if INVERTED is selected) but only on   |
|                |               | night-time (which can be configured by the   |
|                |               | next two config values)                      |
+----------------+---------------+----------------------------------------------+
|DAY_START       |      YES      | The starting hour of day-time                |
+----------------+---------------+----------------------------------------------+
|DAY_END         |      YES      | The ending hour of day-time                  |
+----------------+---------------+----------------------------------------------+
*/

//#define INVERTED

#define NIGHT_INDICATOR

#define DAY_START   8
#define DAY_END     20
