### Test Ebergy Profiler script
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
