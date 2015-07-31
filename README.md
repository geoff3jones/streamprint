# streamprint
This is a  little header only templated helper class for the times when you need to print variables to a stream output (cout,stringstream,etc). There is little or no error checking so if your stream does not support the iomanip functions used, this will break... but at compile time: yay!
two version exist:
-     print(streamout, args...)
-     fprint(streamout,fmt,args)

For the formating version fprint(...) the emphasis was to keep things minimal so the varibles get substituted into '{...}' sections or if the sections run out they get tacked on the end. Likewise if variables run out the remainder of the format string is flushed braces and all.
