# Wavefront bearing generator

## Usage

Requires only gcc.

Compile and process examples with command:
```
make all
```

The program reads data from standard input and prints models to standard output.

See the `examples` for the input format.

Below is the order of the input data:<br>
![](doc/screen0.png)
<br>`Calculated roll count` and `Calculated roll ray` are generated automatically.


## Dimensions

Counts - verticles per round
Roll (vertex per round) counts - verticles per round (in rolls)
Epsilon in x-axis and y-axis - additional space between faces in x-axis and y-axis <br>
Epsilon in z-axis - additional space between faces in z-axis


<br><br>View in z-axis direction<br>
![](doc/screen1.png)

<br><br>The intersection<br>
![](doc/screen2.png)

<br><br>View in y-axis direction<br>
![](doc/screen3.png)

## Images

<br><br>Example bearing<br>
![](doc/screen5.png)

<br><br>Bearing skeleton<br>
![](doc/screen4.png)

<br><br>Large bearing<br>
![](doc/screen6.png)

<br><br>Bearing with roll liner = 1<br>
![](doc/screen7.png)

<br><br>Bearing with roll cover = 0<br>
![](doc/screen10.png)

<br><br>View on rail of roll<br>
![](doc/screen8.png)

<br><br>Skeletal side view<br>
![](doc/screen9.png)
