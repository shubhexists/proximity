# Proximity
Currently it's just a websocket connection written in C. That's it!

# Future Plans
I actually plan to make a GUI app over this backend which can be used across many platforms.

# Usage
The executable binaries are included in the repository. If you haven't made any changes to the `main.c` file, you can directly navigate to the `/client` and `/server` directory to run the executables.

However, if you make changes to the source code, you have to recompile the binaries by - 

`gcc main.c -lpthread -o client` or `gcc main.c -lpthread -o server` , depending on which file you are compiling..

# Why C?
Haha! Main reason was that is was cool, isn't it? Building a websocket in C 🤙🤓. 

# Current Shortages
1) Even though I completed most part of the Socket connection, the current code unfortunately isn't a complete web socket. It has capabilities to have client server communication but server can't relay back the information that it recieves to other 
clients in the network. Basically think of it as a Client Server network architecture, just that clients can't communicate :/
2) I have currently programmed just for the text data, would have to add features on how to transfer the binary data.

# Why another chat app you might ask?
Also, I am jobless, so nothing's stopping me xD
