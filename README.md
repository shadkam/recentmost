recentmost
==========

gvien a basedir, find most recently modified files


Why This
--------
many a times we want to find out most recently modified files on system within a directory - it may be codebase, or configuration files, where we might have been changing things in order to make it work - but in the end when it finally would begin to work, we may have lost track of which files had we touched.

Existing ways to achieve it
---------------------------
There are already ways to query most recently modified files,
One is (we'll call it existingCmds1):

<pre>
$ find ~/ -type f -printf '%TY-%Tm-%Td %TT %p\n' | sort  | tail -20
</pre>
(where 20 is the number of most recently modified files one would like to get). 

existingCmds1 works, if one's "find" supports -printf option (find found on many of the systems do not). Plus existingCmds1 takes lot more time than needed.

Another way to achieve it is (let us call it existingCmds2):

<pre>
$ find ~/xtg-main/ -type f|grep -v " "|xargs ls -lrt | tail -20
</pre>

but xargs starts splitting the arguments if they are too long - so existingCmds2 does not achieve the objective.


To Build / Run
--------------

Unix
----

<pre>
$ gcc -Wall -Wextra -o recentmost recentmost.c
$ find ~/ -type f|./recentmost 20
</pre>

Or just type `make` followed by `make test`.

Windows
-------

To build, go to Visual Studio Cmd Prompt, 
<pre>
$ cl.exe recentmost.c
</pre>
To run, Go to the dir where recently modified files are to be looked for, 
<pre>
$ dir /b/s/A-D | /path/to/recentmost.exe 20
</pre>


Performance
-----------
On my system (linux), existingCmds1 is taking ~9 secs on a dataset, and **recentmost for the same dataset, takes just ~1 sec**.
