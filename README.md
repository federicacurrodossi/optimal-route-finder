
Final exam for Algorithms and Data structures 2022-2023.

Consider a highway described as a sequence of service stations. 
Each service station is located at a distance from the start of the highway, expressed in kilometers by a non-negative integer. 
There are no two service stations with the same distance: each service station is therefore uniquely identified by its distance from the start of the highway.

Each service station has a fleet of electric rental vehicles. Each vehicle is distinguished by its range, given by a full battery charge, expressed in kilometers as a positive integer. 
The fleet of vehicles at a single station includes at most 512 vehicles. Renting a car from station 's' allows you to reach all stations whose distance from 's' is less than or equal to the range of the vehicle.

A journey is identified by a sequence of service stations where the driver stops. It starts at one service station and ends at another, passing through zero or more intermediate stations. 
It is assumed that the driver cannot backtrack during the journey and rents a new car whenever they stop at a service station. Therefore, given two consecutive stops,'s' and 't', 't' must always be farther from the start than 's', and 't' must be reachable using one of the vehicles available at 's'.

The project’s objective is as follows: given a pair of stations, plan the route with the fewest stops between them. 
If there are multiple routes with the same minimum number of stops (i.e., ties), the route that favors stops with shorter distances from the start of the highway must be chosen. 
In other words, consider the set of n tie-breaking routes P = {p1, p2, . . . pn}, where each route is a tuple of m elements pi = ⟨pi,1, pi,2, . . . pi,m⟩, corresponding to the distance from the start of the highway for each stop in the order of travel. 
The unique route pi must be chosen such that there does not exist another pj with the same k final stops preceded by a stop with a shorter distance, i.e., ∄j, k : ⟨pi,m−k+1, . . . pi,m⟩ = ⟨pj,m−k+1, . . . pj,m⟩ ∧ pj,m−k < pi,m−k.

Below is an example of a highway. In this example, the correct route between the station at distance 20 and the one at distance 50 is 20 → 30 → 50 (and not 20 → 45 → 50). 
Conversely, 50 → 30 → 20 is the correct route between the station at distance 50 and the one at distance 20 (thus in the direction from right to left).

![image](https://github.com/user-attachments/assets/31bf0eb9-4cbf-44aa-8c88-efe58a3732e7)

