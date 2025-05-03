
import numpy as np
import time
import math
import requests
import csv

filename = "piTimes.csv"

def timeFuntionDec(func):
    def timeFunc():
        startTime = time.time()

        func()

        endTime = time.time()
        total = endTime - startTime
        print(f"{func.__name__} Took {total:.14f} Seconds")
        # Write func name, start, end to a csv
        # Write function name, start time, and end time to a CSV file
        with open(filename, mode="a", newline="") as file:
            writer = csv.writer(file)
            writer.writerow([func.__name__, startTime, endTime])

    return timeFunc

# monte_carlo simulation to find pi to 3 sig figs
@timeFuntionDec
def monte_carlo():
 
    # Input parameters
    nTrials = int(20E6)

    #-------------
    # Counter for the points inside the circle
    nInside = 0
    
    # startTime = time.time()
    # Generate points in a square of side 2 units, from -1 to 1.
    XrandCoords = np.random.default_rng().uniform(-1, 1, (nTrials,))
    YrandCoords = np.random.default_rng().uniform(-1, 1, (nTrials,))
    
    for i in range(nTrials):
        x = XrandCoords[i]
        y = YrandCoords[i]
        # Check if the points are inside the circle or not
        if x**2+y**2<=1:
            nInside = nInside + 1
                
        
    area = 4*nInside/nTrials

    # endTime = time.time()
    print("Monte Carlo Value of Pi: ",area)

# iteritive calc of pi that uses the approches pi via the Riemann zeta function
# zeta(2) = (pi**2)/6
@timeFuntionDec
def rzf2():
    nTerms = int(10E4)

    sum = 0.0
    for n in range(1,nTerms):
        sum = sum + (1/(n**2))

    pi = (6*sum)**0.5

    print("Rzf2 Value of Pi: ",pi)

#finds factorial for given number, needed for ram
def factorial(x):
    if x==0:
        return 1
    else:
        r=x*factorial(x-1)
        return r

# computes pi value by Ramanujan formula
# source : https://www.maa.org/sites/default/files/pdf/upload_library/22/Chauvenet/BorweinBorweinBailey.pdf
@timeFuntionDec
def ram():
    sum = 0
    n = 0
    i = (math.sqrt(8))/9801

    while True:
        tmp = i*(factorial(4*n)/pow(factorial(n),4))*((26390*n+1103)/pow(396,4*n))
        sum +=tmp

        if(abs(tmp) < 1e-15):
            break
        n += 1

    pi = (1/sum)
    print("Ram Value of Pi: ",pi)

# same as rzf2 but includes a call to sleep for a 10th of a second
@timeFuntionDec
def rzf2sleep():
    nTerms = int(10E4)

    sum = 0.0
    for n in range(1,nTerms):
        sum = sum + (1/(n**2))
        

    pi = (6*sum)**0.5
    time.sleep(0.01)

    print("Rzf2sleep Value of Pi: ",pi)

#get the vlaue of pi over the network
@timeFuntionDec
def network():
    start=0 
    end=5
    url = "https://api.math.tools/numbers/pi"
    params = {"from": start, "to": end}
    try:
        response = requests.get(url, params=params)
        response.raise_for_status()
        data = response.json()
        # print("DATA:",data)
        pi = data["cotents"]["result"]
    except requests.RequestException as e:
        print(f"An error occurred: {e}")

    print("network Value of Pi: ",pi)

# reads from file pi.txt
@timeFuntionDec
def readFromFile():
    f = open("pi.txt", 'r')

    pi = float(f.read())

    print("Read From File Value of Pi: ",pi)

# the fp vlaue of 355/113 is a good approx of pi
@timeFuntionDec
def rationalApprox():
    pi = 355/113

    print("Rational Approx Value of Pi: ",pi)

# calc pi in 5 ways to demo energy vs time on function calls, output timestamps to file
if __name__ == "__main__":

    # Write function name, start time, and end time to a CSV file
    with open(filename, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["Func_Name", "Start_Time", "End_Time"])
    
    #mote carlo
    monte_carlo()
    
    #iterative
    rzf2()


    #ram
    ram()

    #iteritive but wastes time
    rzf2sleep()

    #network
    network()

    #file
    readFromFile()

    #rational approx
    rationalApprox()