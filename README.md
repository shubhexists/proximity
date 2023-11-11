# Proximity
Currently it's just a websocket connection written in C. That's it!

# Future Plans
I actually plan to make a GUI app over this backend which can be used across many platforms so that one can easily save messages across multiple devices. This would not be aimed as a chat app but a synced, lightweight notes type app across 
multiple devices, which anyone with a server can use easily! Here "notes" includes files, documents, images etc etc...

# Why C?
Haha! Main reason was that is was cool, isn't it? Building a websocket in C 🤙🤓. Just kidding, a thought was that, the backend should be efficient enough that it would easily fit into the free tiers of multiple hosting providers like [Render](https://render.com)
etc even after a substantial usage (PS - Text and Video data might take a lot, no?).

# Current Shortages
1) Even though I completed most part of the Socket connection, the current code unfortunately isn't a complete web socket. It has capabilities to have client server communication but server can't relay back the information that it recieves to other 
clients in the network. Basically think of it as a Client Server network architecture, just that clients can't communicate :/
2) I have currently programmed just for the text data, would have to add features on how to transfer the binary data.

# Why another notes app you might ask?
Whatsapp web sucks :( . I can't share my files to my phone easily through it. Hangs every now and then :/ . Also, I am jobless, so nothing's stopping me xD
