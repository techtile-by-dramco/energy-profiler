# 🔍 Energy Profiler 🔍

This repository contains 
- the **hardware** files (including the controller board)
- the **firmware** running on the controller board
- the **publisher script** (which receives information from the controller board over a USB connection)
- an example **subscriber script**

### Updates ToDo 📝
- ???

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
