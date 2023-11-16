# Proximity
Currently it's just a websocket connection written in C 🤓. That's it!

# Future Plans
I actually plan to make a GUI app over this backend which can be used across many platforms.

# Usage
The executable binaries are included in the repository. If you haven't made any changes to the `main.c` file, you can directly navigate to the `/client` and `/server` directory to run the executables.

However, if you make changes to the source code, you have to recompile the binaries by - 

`gcc main.c -lpthread -o client` or `gcc main.c -lpthread -o server` , depending on which file you are compiling..

# Current Shortages
1) I have currently programmed just for the text data, would have to add features on how to transfer the binary data.

# Why another chat app you might ask?
I love being inside terminal, couldn't find any good alternative so why not :)
