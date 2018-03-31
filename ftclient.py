#Nathan Greenlaw
#CS 372
#ONID: greenlan
#chatserve.py 

from socket import *
import sys
import os

def sendMessage(cSocket, msg, msgLength):
	#Found from the python documentation https://docs.python.org/2/howto/sockets.html
	totalSent = 0;
	while totalSent < msgLength:
		sent = cSocket.send(msg[totalSent:]);
		if sent == 0:
			raise RuntimeError("socket connection broken");
		totalSent = totalSent + sent;
		
def recvMessage(cSocket, recvLength):
	#Found from the python documentation https://docs.python.org/2/howto/sockets.html
	chunks = [];
	bytes_recd = 0;
	while bytes_recd < recvLength:
		chunk = cSocket.recv(min(recvLength - bytes_recd, 2048))
		if chunk == '':
			raise RuntimeError("Socket connection Broken")
		chunks.append(chunk);
		#print(chunks);
		bytes_recd = bytes_recd + len(chunk);
	return ''.join(chunks);

def recvMessageAt(cSocket):
	chunks = [];
	fullMess = '';
	at = 0;
	while at < 2:
		chunk = cSocket.recv(1);
		if chunk == '':
			raise RuntimeError("Socket connection Broken");
		if '@' in chunk:
			at+=1;
		chunks.append(chunk);
		#print(chunks);
	fullMess = ''.join(chunks);
	#print(fullMess);
	mess = fullMess.replace('@','');
	return mess;

def sendCommand(cName, lName, sPort, com, fName, cPort):
	clientSocket = socket(AF_INET, SOCK_STREAM);
	serverAddress = (lName, sPort);
	clientSocket.connect(serverAddress);

	#print(cName, lName, sPort, com, fName, cPort);
	
	if fName == None:
		comLength = len(com);
	else:
		cF = com+" "+fName;
		comLength = len(cF);

	cLen = str(comLength)+'@'+'@'; 
	#print(cLen, len(cLen));
	#First send the command length
	sendMessage(clientSocket, cLen, (len(cLen)));
	
	#receive the ready from the server
	ready = "ready";
	msgR = recvMessage(clientSocket, len(ready));
	#print(msgR);
	
	#send the command
	if fName == None:
		sendMessage(clientSocket, com, comLength);
	else:
		sendMessage(clientSocket, cF, comLength); 
	
	#receive the length of the response
	rLength = int(recvMessageAt(clientSocket));
	#print(rLength);
	
	#send a ready
	sendMessage(clientSocket, ready, len(ready));
	
	#receive response and display based on code
	responseMessage = recvMessage(clientSocket, rLength);
	
	if com == '-l': #directory
		print("Receiving directory structure from " + lName + ":" + str(sPort));
		print(responseMessage);
	elif com == '-g' and responseMessage != "File does Not Exist Error": #get file
		cwd = os.getcwd();
		#handle duplicate names
		#found here: https://stackoverflow.com/questions/12375612/avoid-duplicate-file-names-in-a-folder-in-python
		path = cwd+'/'+fName;
		uniq = 1;
		finalFileName = fName;
		print("Receiving file " +fName);
		while os.path.isfile(path):
			path = cwd+'/'+str(uniq)+"_"+fName;
			finalFileName = str(uniq)+"_"+fName;
			uniq+=1;
		#Create the text file
		text_file = open(finalFileName, "w");
		text_file.write(responseMessage);
		text_file.close();
		print("Wrote to file: " +finalFileName);
	else: #error Message
		print(responseMessage);

	#close the socket
	clientSocket.close();

#Main function from Lecture 15 slide 9 Python TCP server

def main():
	numArgs = len(sys.argv);
	# the -l command
	if(numArgs == 6):
		clientName = str(sys.argv[1]); #ftclient
		locationName = str(sys.argv[2]); #flip
		serverPort = int(sys.argv[3]); #port number for ft server
		command = str(sys.argv[4]); #argument given
		clientPort = int(sys.argv[5]); #port number for the client
		#print(clientName, locationName, serverPort, command, clientPort);
		
		if len(command) > 2048:
			print("Too long of command");
			return;
		else:
			sendCommand(clientName, locationName, serverPort, command, None, clientPort);
			return;
		
	#the -g filename command
	elif(numArgs == 7):
		clientName = str(sys.argv[1]); #ftclient
		locationName = str(sys.argv[2]); #flip
		serverPort = int(sys.argv[3]); #port number for ft server
		command = str(sys.argv[4]); #argument given
		fileName = str(sys.argv[5]); #filename given
		clientPort = int(sys.argv[6]); #port number for the client
		#print(clientName, locationName, serverPort, command, fileName, clientPort);

		if len(command) > 2048:
			print("Too long of command");
			return;
		else:
			sendCommand(clientName, locationName, serverPort, command, fileName, clientPort);
			return;
		
	#wrong number of arguments
	else:
		print("Incorrect Arguments entered");
		return;
	
if __name__ == "__main__":
	main();
