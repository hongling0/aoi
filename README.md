make
./test

01[07,08] add--------------------------------

02[01,08] add--------------------------------

03[01,10] add--------------------------------

in      :02[01,08],

04[08,08] add--------------------------------

in      :01[07,08],

05[05,07] add--------------------------------

in      :01[07,08],
06[03,03] add--------------------------------

07[05,08] add--------------------------------

in      :01[07,08],05[05,07],

08[10,02] add--------------------------------

09[00,07] add--------------------------------

in      :02[01,08],

10[08,00] add--------------------------------

in      :08[10,02],

x:00,01,03,05,07,08,10,

y:00,02,03,07,08,10,

01[05,05] mov--------------------------------

in      :06[03,03],

out     :07[05,08],04[08,08],

move    :05[05,07],

x:00,01,03,05,08,10,

y:00,02,03,05,07,08,10,

---
RADIUS=2

|   |  0|  1|  2|  3|  4|  5|  6|  7|  8|  9| 10|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|  0|   |   |   |   |   |   |   |  9|   |   |   |
|  1|   |   |   |   |   |   |   |   |  2|   |  3|
|  2|   |   |   |   |   |   |   |   |   |   |   |
|  3|   |   |   |  6|   |   |   |   |   |   |   |
|  4|   |   |   |   |   |   |   |   |   |   |   |
|  5|   |   |   |   |   | 1'|   |  5|  7|   |   |
|  6|   |   |   |   |   |   |   |   |   |   |   |
|  7|   |   |   |   |   |   |   |   |  1|   |   |
|  8| 10|   |   |   |   |   |   |   |  4|   |   |
|  9|   |   |   |   |   |   |   |   |   |   |   |
| 10|   |  8|   |   |   |   |   |   |   |   |   |

