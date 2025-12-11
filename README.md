# Energy Profiler

### Updates ToDo
- Why does it still return values or adjust the resistance when the voltage is 0V? This is likely caused by negative voltage readings – a type-casting issue from int16 to uint16 → needs to be checked.
- The frame is incorrect and must be reviewed.
- Change the variable name `pwr_nw` to `pwr_pw`.

### Test Energy Profiler script
1. Test `/publisher-script/main_last.py`
2. Run services
	1. Copy service files to systemd:
	```
	sudo cp ~/energy-profiler/ep.service /lib/systemd/system/
 	```
 	2. Enable and start services
  	```
   	sudo systemctl enable ep.service
   	sudo systemctl start ep.service
  	```   
### Update energy profiler service
	
```
sudo cp ~/energy-profiler/ep.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl restart ep.service
```
