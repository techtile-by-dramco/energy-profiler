## JouleScope Measuements

Votage of the power supply was 2V in following measurement.
Power supply was connected to the joulescope and the energy profiler.
The current passes through the JouleScope.

"Joulescope switches shunt resistors in approximately 1 μs on over-range to keep your target device running correctly. It maintains a maximum burden voltage of 20 mV across the shunt resistor, for any current up to 2 A. Joulescope is electrically isolated to avoid any grounding and ground loop concerns." [See here](https://www.joulescope.com/blogs/blog/how-to-measure-current).

### Processing 1: Results

Power consumed in the EP calculated via energy profiler itself.

![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_P1.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_delta_P1.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_error_P1.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_error_log_P1.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_pot_val_P1.png)

### Processing 2: Results

Power consumed in the EP NOT calculated via energy profiler itself.

Due to energy loss over the internal JouleScope shunt resistor, the voltage measured by the energy profiler drops.
Therefore the EP power is calculated via JouleScope voltage in the following graphs.

![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_P2.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_delta_P2.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_error_P2.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_ep_error_log_P2.png)
![](https://github.com/techtile-by-dramco/energy-profiler/blob/main/validation-script/ep-1v3/02-ep-joulescope-power-sweep/js_vs_pot_val_P2.png)
