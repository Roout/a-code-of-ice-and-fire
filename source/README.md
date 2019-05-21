




***Contest for the tile**  
Tile with units on both sides  
  
Should be run using units with the lowest level.  
(Who will give up and use high level unit on tile will lose.  
It's not for sure, cause there is also a gold issue but with same amount of gold this is true).  
***Algorithm***  
1.	Calculate possible reinforcement from both side:  
force = size of connected component with units using dfs  
2.  Compare forces and make decision  
	```C++
	if our force is greater  
		contest for tile    
	else   
		swap unit on unit with greater level```  
