#import menteeRobTest
import yaml
import sys
import random
import time
import threading

def isInteger(s):
    return s.isdigit()

def isFloat(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def isNumber(s):
    return(isInteger(s) or isFloat(s))

def isSensorOrMotor(s):
    return ( (s[0] == 'M') or (s[0] == 'S'))


def isOperand(s):
    return (isNumber(s)  or isSensorOrMotor(s))

def infix_to_rpn(expression):
    precedence = {'+': 1, '-': 1, '*': 2, '/': 2, '^': 3}
    output_queue = []
    operator_stack = []

    def apply_operator(operator):
        b, a = output_queue.pop(), output_queue.pop()
        output_queue.append(a)
        output_queue.append(b)
        output_queue.append(operator)

    for token in expression.split():
        if isOperand(token):
            output_queue.append(token)
        elif token in precedence:
            while (operator_stack and
                   operator_stack[-1] in precedence and
                   precedence[operator_stack[-1]] >= precedence[token]):
                apply_operator(operator_stack.pop())
            operator_stack.append(token)
        elif token == '(':
            operator_stack.append(token)
        elif token == ')':
            while operator_stack[-1] != '(':
                apply_operator(operator_stack.pop())
            operator_stack.pop()  # Discard the '('
        else:
            # Assume token is a function or variable name
            output_queue.append(token)

    while operator_stack:
        apply_operator(operator_stack.pop())

    return output_queue[0]

def read_image_as_bytes(image_path):
    try:
        with open(image_path, "rb") as file:
            image_bytes = file.read()
        return image_bytes
    except FileNotFoundError:
        print(f"Error: Image file '{image_path}' not found.")
        return None
    except Exception as e:
        print(f"Error: Failed to read image file '{image_path}': {e}")
        return None
    

def logResults():
     #vectorOfResults = menteeRobTest.getStatus()  #this will print the results as vector
    for result in vectorOfResults:
        print ("[",result,"]")
    threading.Timer(1, logResults).start() #logs every second

#this is testing function for the callback
def testRandomDataSend(buffer, maxDelay, maxNumberOfBytes):
    while buffer:
        num_bytes = random.randint(1, maxNumberOfBytes)  # Generate random number of bytes to send
        numberOfBytes =min(num_bytes, size(buffer))
        data_to_send = buffer[:numberOfBytes]
        buffer = buffer[numberOfBytes:]

        #menteeRobTest.callback(dataToSend,numberOfBytes )

        time.sleep(random.uniform(0, max_delay))  # Generate random delay between sends



if __name__ == "__main__":
    if len(sys.argv) > 1:
        YAMLfileName = sys.argv[1]
    else:
        print("please specify a file name of yaml to start")
        YAMLfileName = "example.yml"
        #exit(0)
    with open(YAMLfileName, "r") as file:
        yaml_data = yaml.safe_load(file)
    for motor in yaml_data["Motors"]:
        #menteeRobTest.addMotors(yaml_data["Motors"][motor])
        print(motor, yaml_data["Motors"][motor])
    for sensor in yaml_data["Sensors"]:
        print(sensor, yaml_data["Sensors"][sensor])
        #menteeRobTest.addSensors(yaml_data["Sensors"][mosensortor])
    for function in yaml_data["Functions"]:
        equation = yaml_data["Functions"][function]
        parts = equation.split('=')
        motorNumber = int(parts[0][1:])
        RPNRepresent = infix_to_rpn(parts[1])
        print(function,yaml_data["Functions"][function], RPNRepresent) 
        #menteeRobTest.addNetFunction(function,motorNumber, RPNRepresent)
    print("end of setup")

    vectorOfResults = [[],[],[]] #representing motor positions, sensor levels and netfunctions results
    counter =0
    logResults()

    testingPicture = False #this is strictly for showcasing the callback function

    if(testingPicture):
        image_path = "path.jpg"  # this is an example for the callback - it could have been another argument
        image_bytes = read_image_as_bytes(image_path)
        maxDelay = 4
        maxNumberOfBytes = 119
    while(True):
        if(testingPicture):
            testRandomDataSend(image_bytes, maxDelay, maxNumberOfBytes)
        else:
            time.sleep(1)

    

