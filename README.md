# MoonMining

	 This project simulates a lunar Helium-3 space mining operation. This
	 simulation will manage and track the efficiency of mining trucks and unload stations over a
	 continuous 72-hour operation.
 
 KeyComponents:
 
	 ● MiningTrucks: These vehicles perform the actual mining tasks.
  
	 ● MiningSites: Locations on the moon where the trucks extract Helium-3. Assume an infinite
	 number of sites, ensuring trucks always have access to mine without waiting.
  
	 ● MiningUnloadStations: Designated stations where trucks unload the mined Helium-3. Each
	 station can handle one truck at a time
  
Operation Details:

	● There are (n) mining trucks and (m) mining unload stations.
 
	● Mining trucks can spend a random duration between 1 to 5 hours mining at the sites.
 
	● It takes a mining truck 30 minutes to travel between a mining site and an unload station.
 
	○ Assume all trucks are empty at a mining site when the simulation starts.
 
	● Unloading the mined Helium-3 at a station takes 5 minutes.
 
	● Trucks are assigned to the first available unload station. If all stations are occupied, trucks
	queue at the station with the shortest wait time and remain in their chosen queue.
 
Simulation Requirements:
 
	● The simulation must be configurable to accommodate various numbers of mining trucks (n)
	and unload stations (m).
 
	● Calculate and report statistics for the performance and efficiency of each mining truck and
	unload station.
 
	● The simulation represents 72 hours of non-stop mining and must execute faster than
	real-time to provide timely analysis.
 
 Producer/Consumer Semaphores Design Diagram:
 
 This diagram represents the counting semaphores workflow implementated in the current software:

 ![MoonMineDiagram](https://github.com/ntvu5451/MoonMining/assets/44453995/7ef6e08a-6567-461b-993d-0a8397b2d58d)

 
 Run Instructions:
 
	● to configure: cmake -S . -B build
	
	● to build: cmake --build build
 
	● cd to build/Debug
	
	● run: MoonProjectTarget.exe //will use default arguments or 
 
	● run: MoonProjectTarget.ext [n] [m] [run_time in hrs] [output file name] //n=number of trucks, m=number of unload sites
   

Gtests/GMockTests: 

Instructions to run in console below: 

![MoonProjectBuildGmockOutput](https://github.com/ntvu5451/MoonMining/assets/44453995/0269370a-7609-4fd7-89ff-3b13b816272c)

  	test/Mock_MoonMining utilitizes Gtests for testing MoonMining objects/functionaties as well as GMockTests for stubbed functions 
  
  	such as semaphore creation which would require substantial run time in a gtest suite
 
Output: 

	Statistics for truck mining duration for each truck id as well as statistics for number of trucks unloaded per unloading 
	site are printed to the screen as well as to an output file. Due to requirements to run faster than real time,
 	total run time entered via command line argument is scaled to a factor of .015. Simulation mining hrs are measured in
	hrs unscaled, however, the actual mining processing per semaphore is scaled using the scale factor to speed up simulation 
 	run time. 

![MoonProject_Output4](https://github.com/ntvu5451/MoonMining/assets/44453995/417d3a54-e53d-44f9-ac91-e35b8cc0c10a)
