# jump-cutter-revamped
it's like carykh's jumpcutter but faster and in C++ as a different method of extracting the data was used

This jump cutter uses the ffmpeg binaries, so you'll need to have that installed and have the PATH variable set to it

# Help
First, you need to run it in a command line (maybe a gui later on?)
then if you want to see the help from inside the program, type `(program name)` and the help will be displayed
if you need more help on a command, then just do this `(program name) --[command name]` and it'll display the help

# Installation
No fuss, get ffmpeg downloaded and installed and added to your PATH variable, and then all you need to do is compile the program
I compiled it with GCC 8.1.0 and using the C++14 standard. Though it should work on C++03 and C++11 (don't quote me on that) 

# Other notes
The code is really messy, especially the parts that were made first (the silence detection)
I still want to improve it as there are some things on the todo list such as: better help, less logs and make an output argument
I also would want to make a GUI with it, maybe something in imgui or gtk++ or something like that
And probably the best: make it so it will do multiple files at once (or in a sequence) by making a queue or something, would be really neat for those video lectures that are in a 30 part series!
