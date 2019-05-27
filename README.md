# A Code Of Ice And Fire
Take part in AI Competition in Coding Game: [contest link](https://www.codingame.com/leaderboards/challenge/a-code-of-ice-and-fire/global)  
[A Code of Ice & Fire report for Roout](https://www.codingame.com/challengereport/17642769955df1431569b124755acbbe922af687)  
![alt text](https://github.com/Roout/a-code-of-ice-and-fire/blob/master/rank-screen.PNG "Final rank")
Final world standing: **rank 113/2160**;  
Final league: **Gold**
# Core algorithms:  
1. Dfs  
2. Bfs  
3. Search for Bridges in graph (offline)
4. Dijkstra
# Core logic:  
1. Move units  
2. Try to chain-spawn units to reach HQ of the enemy  
3. Try to chain-spawn units to divide territory of the enemy  
4. Defend from chain attack of the enemy (didn't have tile to implement)  
5. Defend my existing bridges on my territory  
6. Reinforce by tower or unit my tiles which are border with the enemy  
7. Attack enemy (score all possible attacks and choose best)  
8. Build mine if it's plenty of gold (almost never)  
  
**Notice: Code is dirty and wasn't refactored.**
